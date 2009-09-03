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

using namespace atmibroker::xa;

XAResourceAdaptorImpl::XAResourceAdaptorImpl(
    XAResourceManager * rm, XID& xid, XID& bid, CORBA::Long rmid, struct xa_switch_t * xa_switch) throw (RMException) :
    rm_(rm), xid_(xid), bid_(bid), complete_(false), rmid_(rmid), xa_switch_(xa_switch), rc_(0),
    tightly_coupled_(0)
{
    FTRACE(xaralogger, "ENTER" << (char*) " new OTS resource rmid:" << rmid_);
}

XAResourceAdaptorImpl::~XAResourceAdaptorImpl()
{
    FTRACE(xaralogger, "ENTER");
    if (!CORBA::is_nil(rc_)) {
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
    FTRACE(xaralogger, "ENTER astate=" << sm_.astate() << " bstate=" << sm_.bstate());
    int rv1, rv2;

    // This resource is joining an existing branch. In this case the thread that
    // originally started the branch is responsible for all updates the RM.
    // Disable since we have introduced bid_ for unique branches for work
    // performed on the RM from different processes
    if (tightly_coupled_)
        return CosTransactions::VoteReadOnly;

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

    switch (rv2) {
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
    default:     // shouldn't happen
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
    case XA_RBROLLBACK:    // these codes may be returned only if the TMONEPHASE flag was set
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
}

void XAResourceAdaptorImpl::commit()
    throw(
        CosTransactions::NotPrepared,
        CosTransactions::HeuristicRollback,
        CosTransactions::HeuristicMixed,
        CosTransactions::HeuristicHazard)
{
    FTRACE(xaralogger, "ENTER");
    if (tightly_coupled_) {
        setComplete();
        return;
    }
    int rv = xa_commit (TMNOFLAGS);    // no need for xa_end since prepare must have been called

    LOG4CXX_TRACE(xaralogger, (char*) "OTS resource commit rv=" << rv);

    terminate(rv);

    setComplete();
}

void XAResourceAdaptorImpl::rollback()
    throw(CosTransactions::HeuristicCommit,CosTransactions::HeuristicMixed,CosTransactions::HeuristicHazard)
{
    FTRACE(xaralogger, "ENTER");
    if (tightly_coupled_) {
        setComplete();
        return;
    }

    int rv = xa_end (TMSUCCESS);

    if (rv != XA_OK) {
        LOG4CXX_WARN(xaralogger, (char *) xa_switch_->name <<
            (char*) ": rollback OTS resource end failed: error=" << rv << " rid=" << rmid_);
    } else {
        LOG4CXX_DEBUG(xaralogger, (char*) "OTS resource end rv=" << rv << " rid=" << rmid_);
    }

    rv = xa_rollback (TMNOFLAGS);
    LOG4CXX_DEBUG(xaralogger, (char*) "OTS resource rollback rv=" << rv);
    terminate(rv);

    setComplete();
}

void XAResourceAdaptorImpl::commit_one_phase() throw(CosTransactions::HeuristicHazard)
{
    FTRACE(xaralogger, "ENTER");
    if (tightly_coupled_) {
        setComplete();
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
    setComplete();
}

void XAResourceAdaptorImpl::forget()
{
    FTRACE(xaralogger, "ENTER");
    int rv = xa_forget (TMNOFLAGS);

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
int XAResourceAdaptorImpl::xa_start (long flags)
{
    FTRACE(xaralogger, (char*) "ENTER astate=" << sm_.astate() << " bstate=" << sm_.bstate());
 
    int rv = xa_switch_->xa_start_entry(&bid_, rmid_, flags);
    return sm_.transition(bid_, XACALL_START, flags, rv);
}
int XAResourceAdaptorImpl::xa_end (long flags)
{
    FTRACE(xaralogger, (char*) "ENTER bstate=" << std::hex << sm_.bstate() << " flags=" << flags);
  
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
