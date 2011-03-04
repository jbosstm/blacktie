/*
 * JBoss, Home of Professional Open Source
 * Copyright 2009, Red Hat, Inc., and others contributors as indicated
 * by the @authors tag. All rights reserved.
 * See the copyright.txt in the distribution for a
 * full listing of individual contributors.
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License, v. 2.1.
 * This program is distributed in the hope that it will be useful, but WITHOUT A
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 * You should have received a copy of the GNU Lesser General Public License,
 * v.2.1 along with this distribution; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */
#include "log4cxx/logger.h"
#include "ThreadLocalStorage.h"
#include "TxControl.h"
#include "TxManager.h"
#include "ace/OS_NS_time.h"

#define TX_GUARD(msg, expect) { \
	FTRACE(txclogger, "ENTER"); \
	if (!isActive(msg, expect)) {   \
		return TX_PROTOCOL_ERROR;   \
	}}

namespace atmibroker {
	namespace tx {

log4cxx::LoggerPtr txclogger(log4cxx::Logger::getLogger("TxLogControl"));

TxControl::TxControl(CosTransactions::Control_ptr ctrl, long timeout, int tid) :
	_rbonly(false), _ttl(timeout), _tid(tid), _ctrl(ctrl) {
	FTRACE(txclogger, "ENTER new TXCONTROL: " << this);

	_ctime = (long) (ACE_OS::gettimeofday().sec());

	if (timeout <= 0l)
		_ttl = -1l;
}

TxControl::~TxControl()
{
	FTRACE(txclogger, "ENTER delete TXCONTROL: " << this);

	if (_cds.size() != 0) {
		LOG4CXX_ERROR(txclogger, (char*) "delete called with outstanding tp calls");
	}

	suspend();
}

int TxControl::end(bool commit, bool reportHeuristics)
{
	TX_GUARD("end", true);

	if (_cds.size() != 0) {
		LOG4CXX_WARN(txclogger, (char*) "protocol error: there are outstanding tp calls");
		return TX_PROTOCOL_ERROR;
	}

	int outcome = TX_OK;
	CosTransactions::Terminator_var term;

	try {
		term = _ctrl->get_terminator();

		if (CORBA::is_nil(term)) {
			LOG4CXX_WARN(txclogger, (char*) "end: no terminator");
			outcome = TX_FAIL;
		}
	} catch (CosTransactions::Unavailable & e) {
		LOG4CXX_WARN(txclogger, (char*) "end: term unavailable: " << e._name());
		outcome = TX_FAIL;
	} catch (CORBA::OBJECT_NOT_EXIST & e) {
		LOG4CXX_DEBUG(txclogger, (char*) "end: term ex " << e._name() << " minor: " << e.minor());
		// transaction no longer exists (presumed abort)
		outcome = TX_ROLLBACK;
	} catch (CORBA::INVALID_TRANSACTION & e) {
		LOG4CXX_WARN(txclogger, (char*) "end get terminater: ex: wrong exception type (see JBTM-748)"
			<< e._name() << " minor: " << e.minor());
		// cannot assume TX_ROLLBACK since the RMs will not have been told (must return TX_FAIL to indicate
		// that the caller’s state with respect to the transaction is unknown)
		outcome = TX_FAIL;
	} catch (...) {
		LOG4CXX_WARN(txclogger, (char*) "end: unknown error looking up terminator");
		outcome = TX_FAIL; // TM failed temporarily
	}

	if (outcome == TX_OK) {
		try {
			// ask the transaction service to end the tansaction
			(commit ? term->commit(reportHeuristics) : term->rollback());

		} catch (CORBA::TRANSACTION_ROLLEDBACK &e) {
			LOG4CXX_INFO(txclogger, (char*) "end: rolled back: " << e._name());
			outcome = TX_ROLLBACK;
		} catch (CosTransactions::Unavailable & e) {
			LOG4CXX_INFO(txclogger, (char*) "end: unavailable: " << e._name());
			outcome = TX_FAIL; // TM failed temporarily
		} catch (CosTransactions::HeuristicMixed &e) {
			LOG4CXX_INFO(txclogger, (char*) "end: HeuristicMixed: " << e._name());
			// can be thrown if commit_return characteristic is TX_COMMIT_COMPLETED
			outcome = TX_MIXED;
		} catch (CosTransactions::HeuristicHazard &e) {
			LOG4CXX_ERROR(txclogger, (char*) "end: HeuristicHazard: " << e._name());
			// can be thrown if commit_return characteristic is TX_COMMIT_COMPLETED
			outcome = TX_HAZARD;
		} catch (CORBA::INVALID_TRANSACTION & e) {
			LOG4CXX_WARN(txclogger, (char*) "end: terminate ex: wrong exception type (see JBTM-748)"
			<< e._name() << " minor: " << e.minor());
			// cannot assume TX_ROLLBACK since the RMs will not have been told (must return TX_FAIL to indicate
			// that the caller’s state with respect to the transaction is unknown)
			outcome = TX_FAIL;
		} catch (CORBA::SystemException & e) {
			LOG4CXX_WARN(txclogger, (char*) "end: " << e._name() << " minor: " << e.minor());
			outcome = TX_FAIL;
		} catch (...) {
			LOG4CXX_WARN(txclogger, (char*) "end: unknown error");
			outcome = TX_FAIL; // TM failed temporarily
		}
	}

	LOG4CXX_DEBUG(txclogger, (char*) "end: outcome: " << outcome);
	suspend();

	return outcome;
}

int TxControl::commit(bool reportHeuristics)
{
	FTRACE(txclogger, "ENTER report " << reportHeuristics);
	return end(true, reportHeuristics);
}

int TxControl::rollback()
{
	FTRACE(txclogger, "ENTER");
	return end(false, false);
}

int TxControl::rollback_only()
{
	TX_GUARD("rollback_only", true);

	try {
		CosTransactions::Coordinator_var coord = _ctrl->get_coordinator();

		if (!CORBA::is_nil(coord.in())) {
			coord->rollback_only();
			return TX_OK;
		}
	} catch (CosTransactions::Unavailable & e) {
		// no coordinator
		LOG4CXX_WARN(txclogger, (char*) "rollback_only: unavailable: " << e._name());
	} catch (CORBA::OBJECT_NOT_EXIST & e) {
		// ought to be due to the txn timing out
		LOG4CXX_DEBUG(txclogger, (char*) "rollback_only: " << e._name() << " minor: " << e.minor());
		_rbonly = true;
		return TX_OK;
	} catch (CORBA::INVALID_TRANSACTION & e) {	// TODO this is wrong (its a workaround for JBTM-748
		LOG4CXX_WARN(txclogger, (char*) "rollback_only: wrong exception type (see JBTM-748)"
			<< e._name() << " minor: " << e.minor());
//		_rbonly = true;
		return TX_FAIL;	// should not happen (see comments in TxControl::end)
	} catch (CORBA::SystemException & e) {
		LOG4CXX_WARN(txclogger, (char*) "rollback_only: " << e._name() << " minor: " << e.minor());
	}

	return TX_FAIL;
}

CosTransactions::Status TxControl::get_ots_status()
{
	FTRACE(txclogger, "ENTER");
	if (!isActive(NULL, false)) {
		return CosTransactions::StatusNoTransaction;
	}

	try {
		CosTransactions::Coordinator_var coord = _ctrl->get_coordinator();

		if (!CORBA::is_nil(coord.in()))
			return (coord->get_status());
	} catch (CosTransactions::Unavailable & e) {
		// no coordinator
		LOG4CXX_TRACE(txclogger, (char*) "unavailable: " << e._name());
	} catch (CORBA::OBJECT_NOT_EXIST & e) {
		LOG4CXX_DEBUG(txclogger, (char*) "status: " << e._name() << " minor: " << e.minor());
		// transaction no longer exists (presumed abort)
		return CosTransactions::StatusNoTransaction;
	} catch (CORBA::INVALID_TRANSACTION & e) {	// TODO this is wrong (its a workaround for JBTM-748
		LOG4CXX_WARN(txclogger, (char*) "status: wrong exception type (see JBTM-748)"
			<< e._name() << " minor: " << e.minor());
		return CosTransactions::StatusNoTransaction;
	} catch (CORBA::SystemException & e) {
		LOG4CXX_WARN(txclogger, (char*) "status: " << e._name() << " minor: " << e.minor());
	}

	return CosTransactions::StatusUnknown;
}

int TxControl::get_status()
{
	FTRACE(txclogger, "ENTER");

	if (_rbonly)
		return TX_ROLLBACK_ONLY;

	CosTransactions::Status status = get_ots_status();

	switch (status) {
	case CosTransactions::StatusActive:
	case CosTransactions::StatusPreparing:
	case CosTransactions::StatusPrepared:
	case CosTransactions::StatusCommitting:
	case CosTransactions::StatusCommitted:
		return TX_ACTIVE;

	case CosTransactions::StatusRollingBack:
	case CosTransactions::StatusRolledBack:
		return TX_ACTIVE;

	case CosTransactions::StatusMarkedRollback:
		return TX_ROLLBACK_ONLY;
		// there is no way to detect TX_TIMEOUT_ROLLBACK_ONLY

	case CosTransactions::StatusUnknown:
		// only option is to assume its active
		return TX_ACTIVE;

	case CosTransactions::StatusNoTransaction:
		// Since XATMI thinks the txn exists but OTS says it doesn't then
		// it must have been due to a timeout (a rollback would have
		// deleted this TxControl object)
		return TX_TIMEOUT_ROLLBACK_ONLY;
	default:
		LOG4CXX_DEBUG(txclogger, (char*) "get_status default: " << status);
		return -1;	// there is no #define for NO TX
	}
}

/**
 * Return -1 if the txn is subject to timeouts
 * otherwise return the remaining time to live
 */
long TxControl::ttl()
{
	FTRACE(txclogger, "ENTER ttl=" << _ttl << " ctime=" << _ctime << " now=" << ACE_OS::gettimeofday().sec());

	if (_ttl == -1l)
		return -1l;

	long ttl = _ttl - (long) (ACE_OS::gettimeofday().sec()) + _ctime;

	LOG4CXX_TRACE(txclogger, (char*) "> ttl=" << ttl);

	return (ttl <= 0l ? 0l : ttl);
}

int TxControl::get_timeout(CORBA::ULong *timeout)
{
	TX_GUARD(NULL, false);

	try {
		CosTransactions::PropagationContext* context = _ctrl->get_coordinator()->get_txcontext();

		*timeout = context->timeout;
		return TX_OK;
	} catch (CORBA::SystemException & e) {
		LOG4CXX_WARN(txclogger, (char*) "get timeout: " << e._name() << " minor: " << e.minor());
		return TX_FAIL;
	}
}

/**
 * Return the OTS control for the current transaction.
 * The caller is responsible for decrementing the ref count
 * of the returned pointer.
 */
CosTransactions::Control_ptr TxControl::get_ots_control(long* ttlp)
{
	FTRACE(txclogger, "ENTER");

	if (ttlp != NULL)
		*ttlp = ttl();

	return (CORBA::is_nil(_ctrl) ? NULL : CosTransactions::Control::_duplicate(_ctrl));
}

/**
 * release the control and remove the tx to thread association
 */
void TxControl::suspend()
{
	FTRACE(txclogger, "ENTER");
	if (!CORBA::is_nil(_ctrl)) {
		try {	// c.f. TxManager::tx_suspend
			(void) CORBA::release(_ctrl);
		} catch (CORBA::SystemException & e) {
			LOG4CXX_WARN(txclogger, (char*) "end: error unref control: " << e._name() << " minor: " << e.minor());
		}
	}

	destroySpecific(TSS_KEY);
	_ctrl = NULL;
}

/**
 * Test whether the OTS control represents a valid transaction
 * If the transaction is expected to be active and its not
 * or vica-versa then log the supplied msg (if its not NULL)
 */
bool TxControl::isActive(const char *msg, bool expect)
{
	FTRACE(txclogger, "ENTER");
	bool c = (!CORBA::is_nil(_ctrl));

	if (c != expect && msg) {
		LOG4CXX_WARN(txclogger, (char*) "protocol violation: " << msg);
	}

	return c;
}

bool TxControl::isOriginator()
{
	return (_tid != 0);
}

} //	namespace tx
} //namespace atmibroker
