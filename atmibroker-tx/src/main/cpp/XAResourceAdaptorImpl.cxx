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

#include "ace/OS_NS_stdlib.h"	/* TODO delete when done testing TEST_BLACKTIE_209 */

log4cxx::LoggerPtr xaralogger(log4cxx::Logger::getLogger("TxLogXAAdaptor"));

extern std::ostream& operator<<(std::ostream &os, const XID& xid);

using namespace atmibroker::xa;

XAResourceAdaptorImpl::XAResourceAdaptorImpl(
	XAResourceManager * rm, XID& xid, XID& bid, CORBA::Long rmid,
	struct xa_switch_t * xa_switch, XARecoveryLog& log) throw (RMException) :
	rm_(rm), xid_(xid), bid_(bid), complete_(false), rmid_(rmid), xa_switch_(xa_switch), rc_(0),
	eflags_(0L), tightly_coupled_(0), rclog_(log), prepared_(false)
{
	FTRACE(xaralogger, "ENTER" << (char*) " new OTS resource rmid:" << rmid_ << " branch id: " << bid_);
}

XAResourceAdaptorImpl::~XAResourceAdaptorImpl()
{
	FTRACE(xaralogger, "ENTER");
	if (rc_)
		free(rc_);
}

void XAResourceAdaptorImpl::notify_error(int reason, bool forget)
{
	FTRACE(xaralogger, "ENTER");
	if (rm_)
		rm_->notify_error(&xid_, reason, forget);
}

void XAResourceAdaptorImpl::set_complete()
{
	FTRACE(xaralogger, "ENTER");
	complete_ = true;

	if (rm_)
		rm_->set_complete(&xid_);
}

Vote XAResourceAdaptorImpl::prepare()
	throw (HeuristicMixed,HeuristicHazard)
{
	FTRACE(xaralogger, "ENTER astate=" << sm_.astate() << " bstate=" << sm_.bstate());
	int rv1, rv2;

	// This resource is joining an existing branch. In this case the thread that
	// originally started the branch is responsible for all updates the RM.
	// Disable since we have introduced bid_ for unique branches for work
	// performed on the RM from different processes
	if (tightly_coupled_)
		return VoteReadOnly;

	rv1 = xa_end(TMSUCCESS);
	rv2 = xa_prepare(TMNOFLAGS);

	if (rv1 != XA_OK && rv2 == XA_OK) {
		LOG4CXX_DEBUG(xaralogger, (char*) "OTS resource: end TMSUCCESS was already set");
	}

	if (rv2 != XA_OK && rv2 != XA_RDONLY) {
		LOG4CXX_WARN(xaralogger, (char *) xa_switch_->name <<
			(char*) ": prepare OTS resource error: " << rv2 << " rid=" << rmid_ << (char*) " rv1=" << rv1);
	} else {
		LOG4CXX_DEBUG(xaralogger, (char*) "prepare OTS resource end ok: rid=" << rmid_
			<< (char*) " rv1=" << rv1 << " rv2=" << rv2 << " bstate=" << sm_.bstate());
	}

	if (rc_ == NULL) {
		rv2 = XA_RDONLY;
		LOG4CXX_ERROR(xaralogger, (char *) "prepare called but no recovery coordinator has been set - assuming RDONLY");
	}

	if (rv2 == XA_OK) {
#if TX_RC == 3
		if (ACE_OS::getenv("TEST_BLACKTIE_209")) {
			LOG4CXX_INFO(xaralogger, (char *) "Test BLACKTIE_209:- SEGV after prepare but before writing log");
			char *s = 0;
			*s = 0;
		}
#endif
		// about to vote commit - remember the descision
		rclog_.add_rec(bid_, rc_);
		prepared_ = true;
	}

	switch (rv2) {
	case XA_OK:
		return VoteCommit;
	case XA_RDONLY:
		return VoteReadOnly;
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
		return VoteRollback;
	default:	 // shouldn't happen
		return VoteRollback;
	}
}

void XAResourceAdaptorImpl::terminate(int rv)
	throw(
		HeuristicRollback,
		HeuristicMixed,
		HeuristicHazard)
{
	FTRACE(xaralogger, "ENTER");

	// remove the entry for this branch from the recovery log
	if (prepared_ && rclog_.del_rec(bid_) != 0) {
		LOG4CXX_DEBUG(xaralogger, (char *) xa_switch_->name <<
			": terminate - entry not found in recovery log rid=" << rmid_);
	}

	switch (rv) {
	default:
		break;
	case XA_HEURHAZ: {
		HeuristicHazard e;
		notify_error(rv, true);
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
		HeuristicRollback e;
		notify_error(rv, true);
		throw e;
		break;
	}
	case XA_HEURMIX: {
		HeuristicMixed e;
		notify_error(rv, true);
		throw e;
		break;
	}
	}
}

void XAResourceAdaptorImpl::commit()
	throw(
		NotPrepared,
		HeuristicRollback,
		HeuristicMixed,
		HeuristicHazard)
{
	FTRACE(xaralogger, "ENTER");
	if (tightly_coupled_) {
		set_complete();
		return;
	}
	int rv = xa_commit (TMNOFLAGS);	// no need for xa_end since prepare must have been called

	LOG4CXX_TRACE(xaralogger, (char*) "OTS resource commit rv=" << rv);

	terminate(rv);

	set_complete();
}

void XAResourceAdaptorImpl::rollback()
	throw(HeuristicCommit,HeuristicMixed,HeuristicHazard)
{
	long eflags = eflags_;

	FTRACE(xaralogger, "ENTER");
	if (tightly_coupled_) {
		set_complete();
		return;
	}

	int rv = xa_end (TMSUCCESS);

	if (rv != XA_OK && eflags != TMSUCCESS) {
		LOG4CXX_WARN(xaralogger, (char *) xa_switch_->name <<
			(char*) ": rollback OTS resource end error " << rv <<
				" for rid " << rmid_ << " - flags=" << std::hex << eflags);
	} else {
		// if rv != XA_OK and the branch was already idle then log at debug only - see ch 6 of the XA spec
		LOG4CXX_DEBUG(xaralogger, (char *) xa_switch_->name <<
			(char*) ": rollback OTS resource end result " << rv <<
				" for rid " << rmid_ << " - flags=" << std::hex << eflags);
	}

	rv = xa_rollback (TMNOFLAGS);
	LOG4CXX_DEBUG(xaralogger, (char*) "OTS resource rollback rv=" << rv);
	terminate(rv);

	set_complete();
}

void XAResourceAdaptorImpl::commit_one_phase() throw(HeuristicHazard)
{
	FTRACE(xaralogger, "ENTER");
	if (tightly_coupled_) {
		set_complete();
		return;
	}

	int rv = xa_end (TMSUCCESS);

	if (rv != XA_OK) {
		LOG4CXX_WARN(xaralogger, (char *) xa_switch_->name <<
			(char*) ": commit 1PC OTS resource end failed: error=" << rv << " rid=" << rmid_);
	} else {
		LOG4CXX_DEBUG(xaralogger, (char*) "1PC OTS resource end ok, rid=" << rmid_);
	}

	rv = xa_commit (TMONEPHASE);
	LOG4CXX_DEBUG(xaralogger, (char*) "1PC OTS resource commit rv=" << rv);

	terminate(rv);
	set_complete();
}

void XAResourceAdaptorImpl::forget()
{
	FTRACE(xaralogger, "ENTER");
	int rv = xa_forget (TMNOFLAGS);

	LOG4CXX_TRACE(xaralogger, (char*) "OTS resource forget rv=" << rv);
	set_complete();
}
// accessors
bool XAResourceAdaptorImpl::is_complete()
{
	FTRACE(xaralogger, "ENTER");
	return complete_;
}

// XA methods
int XAResourceAdaptorImpl::xa_start (long flags)
{
	FTRACE(xaralogger, (char*) "ENTER astate=" << sm_.astate() << " bstate=" << sm_.bstate());
 
	int rv = xa_switch_->xa_start_entry(&bid_, rmid_, flags);
	return sm_.transition(bid_, XACALL_START, flags, rv);
}
int XAResourceAdaptorImpl::xa_end (long flags)
{
	FTRACE(xaralogger, (char*) "ENTER bstate=" << std::hex << sm_.bstate() << " flags=" << flags);
 
	LOG4CXX_DEBUG(xaralogger, (char*) "XA_END rmid: " << rmid_ << std::hex << " flags: " << eflags_ << " -> " << flags);
	eflags_ = flags;

	// if the branch is already idle just return OK - see ch 6 of the XA specification
//	if (sm_.bstate() == S2)
//		return XA_OK;

	int rv = xa_switch_->xa_end_entry(&bid_, rmid_, flags);
	return sm_.transition(bid_, XACALL_END, flags, rv);
}
int XAResourceAdaptorImpl::xa_rollback (long flags)
{
	FTRACE(xaralogger, (char*) "ENTER bstate=" << sm_.bstate());

	int rv = xa_switch_->xa_rollback_entry(&bid_, rmid_, flags);
	return sm_.transition(bid_, XACALL_ROLLBACK, flags, rv);
}
int XAResourceAdaptorImpl::xa_prepare (long flags)
{
	FTRACE(xaralogger, (char*) "ENTER bstate=" << sm_.bstate());
	int rv = xa_switch_->xa_prepare_entry(&bid_, rmid_, flags);
	return sm_.transition(bid_, XACALL_PREPARE, flags, rv);
}
int XAResourceAdaptorImpl::xa_commit (long flags)
{
	FTRACE(xaralogger, (char*) "ENTER bstate=" << sm_.bstate());
	LOG4CXX_DEBUG(xaralogger, (char*) "Commiting resource with branch id: " << bid_);
	int rv = xa_switch_->xa_commit_entry(&bid_, rmid_, flags);
	return sm_.transition(bid_, XACALL_COMMIT, flags, rv);
}
int XAResourceAdaptorImpl::xa_recover (long xxx, long flags)
{
	FTRACE(xaralogger, (char*) "ENTER bstate=" << sm_.bstate());
	int rv = xa_switch_->xa_recover_entry(&bid_, xxx, rmid_, flags);
	return sm_.transition(bid_, XACALL_RECOVER, flags, rv);
}
int XAResourceAdaptorImpl::xa_forget (long flags)
{
	FTRACE(xaralogger, (char*) "ENTER bstate=" << sm_.bstate());
	int rv = xa_switch_->xa_forget_entry(&bid_, rmid_, flags);
	return sm_.transition(bid_, XACALL_FORGET, flags, rv);
}
int XAResourceAdaptorImpl::xa_complete (int * handle, int * retvalue, long flags)
{
	FTRACE(xaralogger, (char*) "ENTER");
	int rv = xa_switch_->xa_complete_entry(handle, retvalue, rmid_, flags);
	return rv;
}
