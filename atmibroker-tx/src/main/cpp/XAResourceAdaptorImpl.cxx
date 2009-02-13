/*
 * JBoss, Home of Professional Open Source
 * Copyright 2009, Red Hat Middleware LLC, and others contributors as indicated
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

log4cxx::LoggerPtr xaResourceLogger(log4cxx::Logger::getLogger("XAResourceLogger"));

XAResourceAdaptorImpl::XAResourceAdaptorImpl(
	XAResourceManager * rm, XID * xid, CORBA::Long rmid, struct xa_switch_t * xa_switch) throw (RMException) :
	rm_(rm), xid_(*xid), complete_(false), rmid_(rmid), xa_switch_(xa_switch) {
}

XAResourceAdaptorImpl::~XAResourceAdaptorImpl() {
}

void XAResourceAdaptorImpl::setComplete()
{
	complete_ = true;

	if (rm_)
		rm_->setComplete(&xid_);
}

// TODO cross check this implementation with JBossTS XAResourceRecord
// CosTransactions::Resource implementation
CosTransactions::Vote XAResourceAdaptorImpl::prepare() {
	int rv = xa_end (&xid_, rmid_, TMSUCCESS);
	LOG4CXX_LOGLS(xaResourceLogger, log4cxx::Level::getTrace(), (char*) "resource OTS prepare rv=" << rv);

	switch (rv) {
	case XA_OK:
		return CosTransactions::VoteCommit;
	case XA_RDONLY:
		return CosTransactions::VoteReadOnly;
	case XAER_RMERR:
	case XAER_RMFAIL:
	case XA_RBROLLBACK:
	case XA_RBEND:
	case XA_RBCOMMFAIL:
	case XA_RBDEADLOCK:
	case XA_RBINTEGRITY:
	case XA_RBOTHER:
	case XA_RBPROTO:
	case XA_RBTIMEOUT:
	case XAER_INVAL:
	case XAER_PROTO:
	case XAER_NOTA: // resource may have arbitrarily rolled back (shouldn't, but ...)
		return CosTransactions::VoteRollback;
	default:
		return CosTransactions::VoteRollback;	// don't know what it did - should be a heuristic
	}
}
void XAResourceAdaptorImpl::rollback() {
	int rv = xa_rollback (&xid_, rmid_, TMFAIL);
	LOG4CXX_LOGLS(xaResourceLogger, log4cxx::Level::getTrace(), (char*) "OTS resource rollback rv=" << rv);
	// TODO figure out whether to forget this branch
	setComplete();
}
void XAResourceAdaptorImpl::commit() {
	int rv = xa_commit (&xid_, rmid_, TMNOFLAGS);
	LOG4CXX_LOGLS(xaResourceLogger, log4cxx::Level::getTrace(), (char*) "OTS resource commit rv=" << rv);
	// TODO figure out whether to forget this branch
	setComplete();
}
void XAResourceAdaptorImpl::forget() {
	int rv = xa_forget (&xid_, rmid_, TMNOFLAGS);
	LOG4CXX_LOGLS(xaResourceLogger, log4cxx::Level::getTrace(), (char*) "OTS resource forget rv=" << rv);
	setComplete();
}
void XAResourceAdaptorImpl::commit_one_phase() {
	int rv = xa_commit (&xid_, rmid_, TMONEPHASE);
	LOG4CXX_LOGLS(xaResourceLogger, log4cxx::Level::getTrace(), (char*) "OTS resource commit_one_phase rv=" << rv);
	setComplete();
}

// accessors
bool XAResourceAdaptorImpl::is_complete() {
	return complete_;
}

// XA methods
char* XAResourceAdaptorImpl::get_name() {
	return (char *) xa_switch_->name;
}
long XAResourceAdaptorImpl::get_flags() {
	return xa_switch_->flags;
}
long XAResourceAdaptorImpl::get_version() {
	return xa_switch_->version;
}
int XAResourceAdaptorImpl::xa_start (XID * txid, int rmid, long flags) {
	LOG4CXX_LOGLS(xaResourceLogger, log4cxx::Level::getTrace(), (char*) "xa_start " << rmid);
	return xa_switch_->xa_start_entry(txid, rmid, flags);
}
int XAResourceAdaptorImpl::xa_end (XID * txid, int rmid, long flags) {
	LOG4CXX_LOGLS(xaResourceLogger, log4cxx::Level::getTrace(), (char*) "xa_end " << rmid);
	return xa_switch_->xa_end_entry(txid, rmid, flags);
}
int XAResourceAdaptorImpl::xa_rollback (XID * txid, int rmid, long flags) {
	LOG4CXX_LOGLS(xaResourceLogger, log4cxx::Level::getTrace(), (char*) "xa_rollback " << rmid);
	return xa_switch_->xa_rollback_entry(txid, rmid, flags);
}
int XAResourceAdaptorImpl::xa_prepare (XID * txid, int rmid, long flags) {
	LOG4CXX_LOGLS(xaResourceLogger, log4cxx::Level::getTrace(), (char*) "xa_prepare " << rmid);
	return xa_switch_->xa_prepare_entry(txid, rmid, flags);
}
int XAResourceAdaptorImpl::xa_commit (XID * txid, int rmid, long flags) {
	LOG4CXX_LOGLS(xaResourceLogger, log4cxx::Level::getTrace(), (char*) "xa_commit " << rmid);
	return xa_switch_->xa_commit_entry(txid, rmid, flags);
}
int XAResourceAdaptorImpl::xa_recover (XID * txid, long xxx, int rmid, long flags) {
	LOG4CXX_LOGLS(xaResourceLogger, log4cxx::Level::getTrace(), (char*) "xa_recover " << rmid);
	return xa_switch_->xa_recover_entry(txid, xxx, rmid, flags);
}
int XAResourceAdaptorImpl::xa_forget (XID * txid, int rmid, long flags) {
	LOG4CXX_LOGLS(xaResourceLogger, log4cxx::Level::getTrace(), (char*) "xa_forget " << rmid);
	return xa_switch_->xa_forget_entry(txid, rmid, flags);
}
int XAResourceAdaptorImpl::xa_complete (int * handle, int * retvalue, int rmid, long flags) {
	LOG4CXX_LOGLS(xaResourceLogger, log4cxx::Level::getTrace(), (char*) "xa_complete " << rmid);
	return xa_switch_->xa_complete_entry(handle, retvalue, rmid, flags);
}
