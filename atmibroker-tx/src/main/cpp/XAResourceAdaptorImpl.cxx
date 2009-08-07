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
#include "XAResourceAdaptorImpl.h"

log4cxx::LoggerPtr xaralogger(log4cxx::Logger::getLogger("TxLogXAAdaptor"));

XAResourceAdaptorImpl::XAResourceAdaptorImpl(
	XAResourceManager * rm, XID * xid, CORBA::Long rmid, struct xa_switch_t * xa_switch) throw (RMException) :
	rm_(rm), xid_(*xid), complete_(false), rmid_(rmid), xa_switch_(xa_switch)
{
	FTRACE(xaralogger, "ENTER" << (char*) " new OTS resource rmid:" << rmid_);
}

XAResourceAdaptorImpl::~XAResourceAdaptorImpl()
{
	FTRACE(xaralogger, "ENTER");
	if (rc_) {
		CORBA::release(rc_);
	}
}

void XAResourceAdaptorImpl::notifyError(int reason, bool forget)
{
	FTRACE(xaralogger, "ENTER");
	if (rm_)
		rm_->notifyError(&xid_, reason, forget);
}

void XAResourceAdaptorImpl::setComplete()
{
	FTRACE(xaralogger, "ENTER");
	complete_ = true;

	if (rm_)
		rm_->setComplete(&xid_);
}

CosTransactions::Vote XAResourceAdaptorImpl::prepare()
	throw (CosTransactions::HeuristicMixed,CosTransactions::HeuristicHazard)
{
	FTRACE(xaralogger, "ENTER");
	int rv = xa_end (&xid_, rmid_, TMSUCCESS);

	if (rv != XA_OK) {
		LOG4CXX_WARN(xaralogger, (char *) xa_switch_->name <<
			(char*) ": prepare OTS resource end failed: error=" << rv << " rid=" << rmid_);
	} else {
		LOG4CXX_DEBUG(xaralogger, (char*) "OTS resource end rv=" << rv << " rid=" << rmid_);
	}

	rv = xa_switch_->xa_prepare_entry(&xid_, rmid_, TMNOFLAGS);
	LOG4CXX_DEBUG(xaralogger, (char*) "OTS resource prepare rv=" << rv << " rid=" << rmid_);

	switch (rv) {
	case XA_OK:
		return CosTransactions::VoteCommit;
	case XA_RDONLY:
		return CosTransactions::VoteReadOnly;
	case XA_RBROLLBACK:
	case XA_RBCOMMFAIL:
	case XA_RBDEADLOCK:
	case XA_RBINTEGRITY:
	case XA_RBOTHER:
	case XA_RBPROTO:
	case XA_RBTIMEOUT:
	case XA_RBTRANSIENT:
	case XAER_ASYNC:
	case XAER_RMERR:
	case XAER_RMFAIL:
	case XAER_NOTA:
	case XAER_INVAL:
	case XAER_PROTO:
		return CosTransactions::VoteRollback;
	default: 	// shouldn't happen
		return CosTransactions::VoteRollback;
	}
}

void XAResourceAdaptorImpl::terminate(int rv)
	throw(
		CosTransactions::HeuristicRollback,
		CosTransactions::HeuristicMixed,
		CosTransactions::HeuristicHazard)
{
	FTRACE(xaralogger, "ENTER");
	switch (rv) {
	default:
		break;
	case XA_HEURHAZ: {
		CosTransactions::HeuristicHazard e;
		notifyError(rv, true);
		throw e;
		break;
	}
	case XA_HEURCOM:
		// a heuristic descision to commit was made (we were lucky) 
		break;
	case XA_HEURRB:
	case XA_RBROLLBACK:	// these codes may be returned only if the TMONEPHASE flag was set
	case XA_RBCOMMFAIL:
	case XA_RBDEADLOCK:
	case XA_RBINTEGRITY:
	case XA_RBOTHER:
	case XA_RBPROTO:
	case XA_RBTIMEOUT:
	case XA_RBTRANSIENT: {
		CosTransactions::HeuristicRollback e;
		notifyError(rv, true);
		throw e;
		break;
	}
	case XA_HEURMIX: {
		CosTransactions::HeuristicMixed e;
		notifyError(rv, true);
		throw e;
		break;
	}
	}

	// TODO figure out whether to forget this branch
}

void XAResourceAdaptorImpl::commit()
	throw(
		CosTransactions::NotPrepared,
		CosTransactions::HeuristicRollback,
		CosTransactions::HeuristicMixed,
		CosTransactions::HeuristicHazard)
{
	FTRACE(xaralogger, "ENTER");
	int rv = xa_commit (&xid_, rmid_, TMNOFLAGS);	// no need for xa_end since prepare must have been called

	LOG4CXX_TRACE(xaralogger, (char*) "OTS resource commit rv=" << rv);

	terminate(rv);

	// TODO figure out whether to forget this branch
	setComplete();
}

void XAResourceAdaptorImpl::rollback()
	throw(CosTransactions::HeuristicCommit,CosTransactions::HeuristicMixed,CosTransactions::HeuristicHazard)
{
	FTRACE(xaralogger, "ENTER");
	int rv = xa_end (&xid_, rmid_, TMSUCCESS);

	if (rv != XA_OK) {
		LOG4CXX_WARN(xaralogger, (char *) xa_switch_->name <<
			(char*) ": rollback OTS resource end failed: error=" << rv << " rid=" << rmid_);
	} else {
		LOG4CXX_DEBUG(xaralogger, (char*) "OTS resource end rv=" << rv << " rid=" << rmid_);
	}

	rv = xa_rollback (&xid_, rmid_, TMNOFLAGS);
	LOG4CXX_DEBUG(xaralogger, (char*) "OTS resource rollback rv=" << rv);
	terminate(rv);
	// TODO figure out whether to forget this branch
	setComplete();
}

void XAResourceAdaptorImpl::commit_one_phase() throw(CosTransactions::HeuristicHazard)
{
	FTRACE(xaralogger, "ENTER");
	int rv = xa_end (&xid_, rmid_, TMSUCCESS);

	if (rv != XA_OK) {
		LOG4CXX_WARN(xaralogger, (char *) xa_switch_->name <<
			(char*) ": commit 1PC OTS resource end failed: error=" << rv << " rid=" << rmid_);
	} else {
		LOG4CXX_DEBUG(xaralogger, (char*) "1PC OTS resource end ok, rid=" << rmid_);
	}

	rv = xa_commit (&xid_, rmid_, TMONEPHASE);
	LOG4CXX_DEBUG(xaralogger, (char*) "1PC OTS resource commit rv=" << rv);

	terminate(rv);
	setComplete();
}

void XAResourceAdaptorImpl::forget()
{
	FTRACE(xaralogger, "ENTER");
	int rv = xa_forget (&xid_, rmid_, TMNOFLAGS);

	LOG4CXX_TRACE(xaralogger, (char*) "OTS resource forget rv=" << rv);
	setComplete();
}
// accessors
bool XAResourceAdaptorImpl::is_complete()
{
	FTRACE(xaralogger, "ENTER");
	return complete_;
}

// XA methods
char* XAResourceAdaptorImpl::get_name()
{
	FTRACE(xaralogger, "ENTER");
	return (char *) xa_switch_->name;
}
long XAResourceAdaptorImpl::get_flags()
{
	FTRACE(xaralogger, "ENTER");
	return xa_switch_->flags;
}
long XAResourceAdaptorImpl::get_version()
{
	FTRACE(xaralogger, "ENTER");
	return xa_switch_->version;
}
int XAResourceAdaptorImpl::xa_start (XID * txid, int rmid, long flags)
{
	FTRACE(xaralogger, (char*) "ENTER rmid= " << rmid << (char*) ", flags 0x" << std::hex << flags);
	return xa_switch_->xa_start_entry(txid, rmid, flags);
}
int XAResourceAdaptorImpl::xa_end (XID * txid, int rmid, long flags)
{
	FTRACE(xaralogger, (char*) "ENTER rmid= " << rmid << (char*) ", flags 0x" << std::hex << flags);
	return xa_switch_->xa_end_entry(txid, rmid, flags);
}
int XAResourceAdaptorImpl::xa_rollback (XID * txid, int rmid, long flags)
{
	FTRACE(xaralogger, (char*) "ENTER xa_rollback: rmid= " << rmid << (char*) ", flags=0x" << std::hex << flags);
	return xa_switch_->xa_rollback_entry(txid, rmid, flags);
}
int XAResourceAdaptorImpl::xa_prepare (XID * txid, int rmid, long flags)
{
	FTRACE(xaralogger, (char*) "ENTER: rmid= " << rmid << (char*) ", flags=0x" << std::hex << flags);
	return xa_switch_->xa_prepare_entry(txid, rmid, flags);
}
int XAResourceAdaptorImpl::xa_commit (XID * txid, int rmid, long flags)
{
	FTRACE(xaralogger, (char*) "ENTER rmid=" << rmid << (char*) ", flags=0x" << std::hex << flags);
	return xa_switch_->xa_commit_entry(txid, rmid, flags);
}
int XAResourceAdaptorImpl::xa_recover (XID * txid, long xxx, int rmid, long flags)
{
	FTRACE(xaralogger, (char*) "ENTER: rmid= " << rmid << (char*) ", flags=0x" << std::hex << flags);
	return xa_switch_->xa_recover_entry(txid, xxx, rmid, flags);
}
int XAResourceAdaptorImpl::xa_forget (XID * txid, int rmid, long flags)
{
	FTRACE(xaralogger, "ENTER: rmid= " << rmid << (char*) ", flags=0x" << std::hex << flags);
	return xa_switch_->xa_forget_entry(txid, rmid, flags);
}
int XAResourceAdaptorImpl::xa_complete (int * handle, int * retvalue, int rmid, long flags)
{
	FTRACE(xaralogger, (char*) "ENTER " << rmid
		<< (char*) ", flags=0x" << std::hex << flags);
	return xa_switch_->xa_complete_entry(handle, retvalue, rmid, flags);
}
