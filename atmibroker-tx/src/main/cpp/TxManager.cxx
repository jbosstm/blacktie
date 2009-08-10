/*
 * JBoss, Home of Professional Open Source
 * Copyright 2008, Red Hat, Inc., and others contributors as indicated
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

#include <tao/ORB.h>
#include "tao/ORB_Core.h"

#include "ThreadLocalStorage.h"
#include "XAResourceManagerFactory.h"

#include "TxInitializer.h"
#include "OrbManagement.h"
#include "TxManager.h"
#include "AtmiBrokerEnv.h"
#include "ace/Thread.h"

namespace atmibroker {
    namespace tx {

log4cxx::LoggerPtr txmlogger(log4cxx::Logger::getLogger("TxLogManager"));

TxManager *TxManager::_instance = NULL;

TxManager *TxManager::get_instance()
{
	if (_instance == NULL)
		_instance = new TxManager();

	return _instance;
}

void TxManager::discard_instance()
{
	FTRACE(txmlogger, "ENTER");
	if (_instance != NULL) {
		delete _instance;
		_instance = NULL;
	}
}

TxManager::TxManager() :
	_whenReturn(TX_COMMIT_DECISION_LOGGED), _controlMode(TX_UNCHAINED), _timeout (0L), _isOpen(false)
{
	FTRACE(txmlogger, "ENTER");
	_connection = init_orb((char*) "ots");
}

TxManager::~TxManager()
{
	FTRACE(txmlogger, "ENTER");
	if (_connection) {
		shutdownBindings(_connection);
		delete _connection;
	}
}

CORBA_CONNECTION* TxManager::init_orb(char* name)
{
	FTRACE(txmlogger, "ENTER");
	register_tx_interceptors(name);
	return ::initOrb(name);
}

atmibroker::tx::TxControl *TxManager::currentTx(const char *msg)
{
	FTRACE(txmlogger, "ENTER");
	atmibroker::tx::TxControl *tx = NULL;

	if ((!_isOpen || (tx = (atmibroker::tx::TxControl *) getSpecific(TSS_KEY)) == NULL || !tx->isActive(NULL, true)) && msg) {
		LOG4CXX_INFO(txmlogger, (char*) "protocol violation (" << msg << ") open="
			<< _isOpen << (char*) " TSS_KEY=" << getSpecific(TSS_KEY));
	}

	return tx;
}

int TxManager::begin(void)
{
	FTRACE(txmlogger, "ENTER");
	if (!_isOpen || getSpecific(TSS_KEY)) {
		// either open hasn't been called or already in a transaction
		LOG4CXX_WARN(txmlogger, (char*) "begin: protocol violation: open="
			<< _isOpen << (char*) " TSS_KEY=" << getSpecific(TSS_KEY));
		return TX_PROTOCOL_ERROR;
	} else {
		// start a new transaction
		CosTransactions::Control_ptr ctrl = _txfac->create(_timeout);

		if (CORBA::is_nil(ctrl)) {
			LOG4CXX_WARN(txmlogger, (char*) "begin: create returned nil control");
			return TX_ERROR;
		}

		TxControl *tx = new TxControl(ctrl, ACE_OS::thr_self());
		// associate the tx with the callers thread and enlist all open RMs with the tx
		int rc = TxManager::tx_resume(tx, TMNOFLAGS);

		if (rc != XA_OK) {
			// one or more RMs failed to start - roll back the transaction
			LOG4CXX_WARN(txmlogger, (char*) "begin: XA resume error: " << rc);
			try {
				CosTransactions::Terminator_var term = ctrl->get_terminator();

				if (!CORBA::is_nil(term))
					term->rollback();
			} catch (...) {
			}

			return TX_ERROR;
		}
	}

	LOG4CXX_DEBUG(txmlogger, (char*) "begin: ok");

	return TX_OK;
}

int TxManager::commit(void)
{
	FTRACE(txmlogger, "ENTER");
	return complete(true);
}

int TxManager::rollback(void)
{
	FTRACE(txmlogger, "ENTER");
	return complete(false);
}

int TxManager::rollback_only(void)
{
	FTRACE(txmlogger, "ENTER");
	atmibroker::tx::TxControl *tx = currentTx("rollback_only");

	if (tx == NULL)
		return TX_PROTOCOL_ERROR;

	// inform the local resource managers
	rm_end(TMFAIL);
	return tx->rollback_only();
}

int TxManager::complete(bool commit)
{
	FTRACE(txmlogger, "ENTER");
	int outcome;
	atmibroker::tx::TxControl *tx = currentTx("complete");

	if (tx == NULL)
		return TX_PROTOCOL_ERROR;

	// each RM wrapper calls calls xa_end 
	outcome = (commit ? tx->commit(reportHeuristics()) : tx->rollback());

	return (isChained() ? chainTransaction(outcome) : outcome);
}

int TxManager::chainTransaction(int outcome)
{
	FTRACE(txmlogger, "ENTER");
	/*
	 * NOTE: outcome will only truly represent the outcome of commit if the commit_return
	 * characteristic is TX_COMMIT_COMPLETED (see method reportHeuristics()).
	 * Using get Coordinator::get_status is ambiguous since NoTransaction can mean the
	 * transaction committed or rolled back and has been forgotten.
	 * TODO in begin register a participant in the transaction so we can definitively know
	 * the transaction outcome.
	 */
	switch(begin()) {
	case TX_OK:
		return TX_OK;
	default:
		switch (outcome) {
		case TX_OK:
			return TX_NO_BEGIN;
		case TX_ROLLBACK:
			return TX_ROLLBACK_NO_BEGIN;
		case TX_MIXED:
			return TX_MIXED_NO_BEGIN;
		case TX_HAZARD:
			return TX_HAZARD_NO_BEGIN;
		default:
			return outcome;
		}
	}
}

int TxManager::set_commit_return(COMMIT_RETURN when_return)
{
	FTRACE(txmlogger, "ENTER");
	if (!_isOpen)
		return TX_PROTOCOL_ERROR;
	else if (when_return != TX_COMMIT_DECISION_LOGGED && when_return != TX_COMMIT_COMPLETED)
		return TX_EINVAL;

	_whenReturn = when_return;

	return TX_OK;
}

int TxManager::set_transaction_control(TRANSACTION_CONTROL mode)
{
	FTRACE(txmlogger, "ENTER");
	if (!_isOpen)
		return TX_PROTOCOL_ERROR;
	else if (mode != TX_UNCHAINED && mode != TX_CHAINED)
		return TX_EINVAL;

	_controlMode = mode;

	return TX_OK;
}

int TxManager::set_transaction_timeout(TRANSACTION_TIMEOUT timeout)
{
	FTRACE(txmlogger, "ENTER");
	if (!_isOpen)
		return TX_PROTOCOL_ERROR;
	else if (timeout < 0)
		return TX_EINVAL;

	_timeout = timeout;

	return TX_OK;
}

int TxManager::info(TXINFO *info)
{
	FTRACE(txmlogger, "ENTER");
	if (!_isOpen)
		return TX_PROTOCOL_ERROR;

	atmibroker::tx::TxControl *tx = currentTx(NULL);

	if (info != 0) {
		info->transaction_state = -1;
		(info->xid).formatID = -1; // means the XID is null
		info->when_return = _whenReturn;
		info->transaction_control = _controlMode;
		// the timeout that will be used when this process begins the next transaction
		// (it is not neccessarily the timeout for the current transaction since
		// this may have been set in another process or it may have been changed by
		// this process after the current transaction was started).
		info->transaction_timeout = _timeout;

		if (tx != NULL) {
			XAResourceManagerFactory::getXID(info->xid);
			info->transaction_state = tx->get_status();
			LOG4CXX_DEBUG(txmlogger, (char*) "info status=" << info->transaction_state);
		}
	}

	return (tx != NULL);
}

int TxManager::open(void)
{
	FTRACE(txmlogger, "ENTER");
	int rc;

	if (_isOpen)
		return TX_OK;

	LOG4CXX_DEBUG(txmlogger, (char*) "open ");

	if (_txfac == NULL) {
		char *transFactoryId = AtmiBrokerEnv::get_instance()->getenv((char*)"TRANS_FACTORY_ID");

		if (transFactoryId == NULL || strlen(transFactoryId) == 0) {
			LOG4CXX_ERROR(txmlogger, (char*) "Please set the TRANS_FACTORY_ID env variable");
			return TX_ERROR;
		}

		try {
			CosNaming::NamingContextExt_ptr nce = (CosNaming::NamingContextExt_ptr) _connection->default_ctx;
			CosNaming::Name *name = nce->to_name(transFactoryId);
			LOG4CXX_DEBUG(txmlogger, (char*) "resolving Tx Fac Id: " << transFactoryId);
			CORBA::Object_var obj = nce->resolve(*name);
			delete name;
			LOG4CXX_DEBUG(txmlogger, (char*) "resolved OK: " << (void*) obj);
			_txfac = CosTransactions::TransactionFactory::_narrow(obj);
			LOG4CXX_DEBUG(txmlogger, (char*) "narrowed OK: " << (void*) _txfac);

		} catch (CORBA::SystemException & e) {
			LOG4CXX_ERROR(txmlogger, 
				(char*) "Error resolving Tx Service: " << e._name() << " minor code: " << e.minor());
			return TX_ERROR;
		} catch (...) {
			LOG4CXX_ERROR(txmlogger, 
				(char*) "Unknown error resolving Tx Service: " << transFactoryId);
			return TX_ERROR;
		}
	}

	// re-initialize values
	_controlMode = TX_UNCHAINED;
	_timeout = 0L;
	_isOpen = true;

	rc = rm_open();

	LOG4CXX_DEBUG(txmlogger, (char*) "open XA status: " << rc);
	return TX_OK;
}

int TxManager::close(void)
{
	FTRACE(txmlogger, "ENTER");
	if (!_isOpen)
		return TX_OK;

	if (getSpecific(TSS_KEY)) {
		LOG4CXX_WARN(txmlogger, (char*) "close: transaction still active");
		return TX_PROTOCOL_ERROR;
	}

	LOG4CXX_DEBUG(txmlogger, (char*) "close ");
	_isOpen = false;
	rm_close();

	return TX_OK;
}

int TxManager::rm_open(void)
{
	FTRACE(txmlogger, "ENTER");
	try {
		_xaRMFac.createRMs(_connection);
		return 0;
	} catch (RMException& ex) {
		LOG4CXX_WARN(txmlogger, (char*) "failed to load RMs: " << ex.what());
		return -1;
	}
}
void TxManager::rm_close(void)
{
	FTRACE(txmlogger, "ENTER");
	_xaRMFac.destroyRMs();
}
int TxManager::rm_end(int flags)
{
	FTRACE(txmlogger, "ENTER");
	return _xaRMFac.endRMs(flags);
}
int TxManager::rm_start(int flags)
{
	FTRACE(txmlogger, "ENTER");
	return _xaRMFac.startRMs(flags);
}

// static methods
CosTransactions::Control_ptr TxManager::get_ots_control()
{
	FTRACE(txmlogger, "ENTER");
    TxControl *tx = (TxControl *) getSpecific(TSS_KEY);

    return (tx ? tx->get_ots_control() : 0);
}

int TxManager::tx_resume(CosTransactions::Control_ptr control, int flags)
{
	FTRACE(txmlogger, "ENTER");
    return TxManager::tx_resume(control, ACE_OS::thr_self(), flags);
}

int TxManager::tx_resume(CosTransactions::Control_ptr control, int creator, int flags)
{
	FTRACE(txmlogger, "ENTER");
    return TxManager::tx_resume(new TxControl(control, creator), flags);
}

int TxManager::tx_resume(char* ctrlIOR, char *orbname, int flags)
{
	FTRACE(txmlogger, "ENTER");
    CORBA::Object_ptr p = atmi_string_to_object(ctrlIOR, orbname);

    LOG4CXX_DEBUG(txmlogger, (char*) "tx_resume orb=" << orbname << (char *) " IOR=" << ctrlIOR << " ptr=" << p);

    if (!CORBA::is_nil(p)) {
        CosTransactions::Control_ptr cptr = CosTransactions::Control::_narrow(p);
        CORBA::release(p); // dispose of it now that we have narrowed the object reference

        return TxManager::tx_resume(cptr, 0, flags); // why 0 tid
    } else {
		LOG4CXX_WARN(txmlogger, (char*) "tx_resume: invalid control IOR: " << ctrlIOR);
	}

    return TMER_INVAL;
}

int TxManager::tx_resume(TxControl *tx, int flags)
{
	FTRACE(txmlogger, "ENTER");
	int rc = XAER_NOTA;

    try {
		// TMJOIN TMRESUME TMNOFLAGS
		// must associate the tx with the thread before calling start on each open RM
   		setSpecific(TSS_KEY, tx);
		if ((rc = TxManager::get_instance()->rm_start(flags)) == XA_OK) {
			LOG4CXX_DEBUG(txmlogger, (char *) "Resume tx: ok");

			return XA_OK;
		} else {
			LOG4CXX_WARN(txmlogger, (char *) "Resume tx: error: " << rc);
		}
    } catch (...) {
        LOG4CXX_DEBUG(txmlogger, (char *) "Resume tx: generic exception");
    }

	destroySpecific(TSS_KEY);

	return rc;
}

CosTransactions::Control_ptr TxManager::tx_suspend(int flags)
{
	FTRACE(txmlogger, "ENTER");
    return (tx_suspend((TxControl *) getSpecific(TSS_KEY), 0, flags));
}

CosTransactions::Control_ptr TxManager::tx_suspend(int thr_id, int flags)
{
	FTRACE(txmlogger, "ENTER");
    return (tx_suspend((TxControl *) getSpecific(TSS_KEY), thr_id, flags));
}

/**
 * Suspend the transaction and return the control.
 * The caller is responsible for releasing the returned control
 */
CosTransactions::Control_ptr TxManager::tx_suspend(TxControl *tx, int thr_id, int flags)
{
	FTRACE(txmlogger, "ENTER");
    if (tx
		&& tx->isActive(NULL, true)	// tx is active
//TODO		&& (thr_id == 0 || tx->thr_id() != (int) thr_id)	// not the owning thread or don't care about owners
	    ) {
		// increment the control reference count
		CosTransactions::Control_ptr ctrl = tx->get_ots_control();
		// suspend all open Resource Managers (TMSUSPEND TMMIGRATE TMSUCCESS TMFAIL)
        (void) TxManager::get_instance()->rm_end(flags);
		// disassociate the transaction from the callers thread
		tx->suspend();
		delete tx;

		FTRACE(txmlogger, "< ctrl: " << ctrl);
		return ctrl;
    }

	FTRACE(txmlogger, "< ctrl: 0x0");
     return NULL;
}

} //    namespace tx
} //namespace atmibroker
