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
#include "XAResourceManager.h"
#include "ThreadLocalStorage.h"
#include "ace/OS_NS_time.h"

log4cxx::LoggerPtr xarmlogger(log4cxx::Logger::getLogger("TxLogXAManager"));

ostream& operator<<(ostream &os, const XID& xid)
{
    os << xid.formatID << ':' << xid.gtrid_length << ':' << xid.bqual_length << ':' << xid.data;
    return os;
}

void XAResourceManager::show_branches(const char *msg, XID * xid)
{
    FTRACE(xarmlogger, "ENTER " << *xid);

    for (XABranchMap::iterator i = branches_.begin(); i != branches_.end(); ++i) {

        LOG4CXX_TRACE(xarmlogger, (char *) "XID: " << *(i->first)); 
    }
}

static int compareXids(XID * xid1, XID * xid2)
{
    FTRACE(xarmlogger, "ENTER");
    if (xid1 == xid2)
        return 0;

    if ((xid1->formatID == xid2->formatID)
        && (xid1->gtrid_length == xid2->gtrid_length)
        && (xid1->bqual_length == xid2->bqual_length))
    {
        for (int i = 0; i < (xid1->gtrid_length + xid1->bqual_length); i++)
            if (xid1->data[i] != xid2->data[i])
                return -1;

        return 0;    // XIDs are equal
    } else {
        return 1;
    }
}

XAResourceManager::XAResourceManager(
    CORBA_CONNECTION* connection,
    const char * name,
    const char * openString,
    const char * closeString,
    CORBA::Long rmid,
    struct xa_switch_t * xa_switch) throw (RMException) :

    poa_(NULL), connection_(connection), name_(name), openString_(openString), closeString_(closeString),
    rmid_(rmid), xa_switch_(xa_switch) {

    FTRACE(xarmlogger, "ENTER " << (char *) "new RM name: " << name << (char *) " openinfo: " <<
        openString << (char *) " rmid: " << rmid_);

    if (name == NULL) {
        RMException ex("Invalid RM name", EINVAL);
        throw ex;
    }

    int rv = xa_switch_->xa_open_entry((char *) openString, rmid_, TMNOFLAGS);

    LOG4CXX_TRACE(xarmlogger,  (char *) "xa_open: " << rv);

    if (rv != XA_OK) {
        RMException ex("xa_open", rv);
        throw ex;
    }

    // each RM has its own POA
    createPOA();
}

XAResourceManager::~XAResourceManager() {
    FTRACE(xarmlogger, "ENTER");
    int rv = xa_switch_->xa_close_entry((char *) closeString_, rmid_, TMNOFLAGS);

    LOG4CXX_TRACE(xarmlogger, (char *) "xa_close: " << rv);

    if (rv != XA_OK)
        LOG4CXX_WARN(xarmlogger, (char *) " close RM " << name_ << " failed: " << rv);

    if (!CORBA::is_nil(poa_)) {
        CORBA::release(poa_);
        poa_ = NULL;
    }
}

void XAResourceManager::createPOA() {
    FTRACE(xarmlogger, "ENTER");
    //CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
    //PortableServer::POA_var rpoa = PortableServer::POA::_narrow(obj);

    PortableServer::POAManager_ptr poa_manager = (PortableServer::POAManager_ptr) connection_->root_poa_manager;
    PortableServer::POA_ptr parent_poa = (PortableServer::POA_ptr) connection_->root_poa;
    PortableServer::LifespanPolicy_var p1 = parent_poa->create_lifespan_policy(PortableServer::PERSISTENT);
//    PortableServer::IdAssignmentPolicy_var p2 = parent_poa->create_id_assignment_policy(PortableServer::USER_ID);

    CORBA::PolicyList policies;
    policies.length(1);

    // the servant object references must survive failure of the ORB in order to support recover of 
    // transaction branches (the default orb policy for servants is transient)
    policies[0] = PortableServer::LifespanPolicy::_duplicate(p1);
//    policies[1] = PortableServer::IdAssignmentPolicy::_duplicate(p2);

    // create a new POA for this RM

    ACE_TCHAR name[32];
    memset(name, '\0', 32);
    ACE_OS::sprintf(name, ACE_TEXT("%s%d"), "ATMI_RM_", rmid_);

    try {
        this->poa_ = parent_poa->create_POA(name, poa_manager, policies);
//            LOG4CXX_TRACE(xarmlogger,  (char *) "created poa");
        p1->destroy();
    } catch (PortableServer::POA::AdapterAlreadyExists &) {
        p1->destroy();
        try {
            this->poa_ = parent_poa->find_POA(name, false);
        } catch (const PortableServer::POA::AdapterNonExistent &) {
            LOG4CXX_WARN(xarmlogger, (char *) "Duplicate RM POA - name = " << name);
            RMException ex("Duplicate RM POA", EINVAL);
            throw ex;
        }

    } catch (PortableServer::POA::InvalidPolicy &) {
        p1->destroy();
        LOG4CXX_WARN(xarmlogger, (char *) "Invalid RM POA policy");
        RMException ex("Invalid RM POA policy", EINVAL);
        throw ex;
    }

    // take the POA out of its holding state
    PortableServer::POAManager_var mgr = this->poa_->the_POAManager();
    mgr->activate();
//    LOG4CXX_TRACE(xarmlogger,  (char *) "activated mgr");
}

int XAResourceManager::createServant(XID * xid)
{
    FTRACE(xarmlogger, "ENTER");
    int res;
    XAResourceAdaptorImpl *ra = NULL;
    CosTransactions::Control_ptr curr = (CosTransactions::Control_ptr) txx_get_control();
    CosTransactions::Coordinator_ptr coord = NULL;

    if (CORBA::is_nil(curr))
        return XAER_NOTA;

    try {
        // create a servant to represent the new branch identified by xid
        XID bid = gen_xid(*xid);
        ra = new XAResourceAdaptorImpl(this, *xid, bid, rmid_, xa_switch_);
        // and activate it
        PortableServer::ObjectId_var objId = poa_->activate_object(ra);

        // get a CORBA reference to the servant so that it can be enlisted in the OTS transaction
        CORBA::Object_var ref = poa_->servant_to_reference(ra);
        ra->_remove_ref();    // now only the POA has a reference to ra

        CosTransactions::Resource_var v = CosTransactions::Resource::_narrow(ref);

        // enlist it with the transaction
        LOG4CXX_TRACE(xarmlogger, (char*) "enlisting resource");
        coord = curr->get_coordinator();
        CosTransactions::RecoveryCoordinator_ptr rc = coord->register_resource(v);
        ra->setRecoveryCoordinator(rc);
        //c->register_synchronization(new XAResourceSynchronization(xid, rmid_, xa_switch_));

        if (CORBA::is_nil(rc)) {
            LOG4CXX_TRACE(xarmlogger, (char*) "createServant: nill RecoveryCoordinator ");
            res = XAER_RMFAIL;
        } else {
            XID * cp = (XID *) malloc(sizeof(XID));

            if (cp == 0) {
                LOG4CXX_ERROR(xarmlogger, (char *) "out of memory");
                res = XAER_RMFAIL;
            } else {
                *cp = *xid;
                branches_[cp] = ra;
                ra = NULL;
                res = XA_OK;
            }
        }
    } catch (RMException& ex) {
        LOG4CXX_WARN(xarmlogger, (char*) "unable to create resource adaptor for branch: " << ex.what());
    } catch (PortableServer::POA::ServantNotActive&) {
        LOG4CXX_ERROR(xarmlogger, (char*) "createServant: poa inactive");
    } catch (CosTransactions::Inactive&) {
        LOG4CXX_TRACE(xarmlogger, (char*) "createServant: tx inactive (too late for registration)");
    } catch (const CORBA::SystemException& e) {
        LOG4CXX_WARN(xarmlogger, (char*) "Resource registration error: " << e._name() << " minor: " << e.minor());
    }

    if (ra)
        delete ra;

    if (!CORBA::is_nil(curr))
        CORBA::release(curr);

    if (!CORBA::is_nil(coord))
        CORBA::release(coord);

    return res;
}

void XAResourceManager::notifyError(XID * xid, int xa_error, bool forget)
{
    FTRACE(xarmlogger, "ENTER");
    LOG4CXX_TRACE(xarmlogger, (char*) "notifyError reason:" << xa_error);

    if (forget)
        setComplete(xid);
}

void XAResourceManager::setComplete(XID * xid)
{
    FTRACE(xarmlogger, "ENTER");
    XABranchMap::iterator iter;

    LOG4CXX_TRACE(xarmlogger, (char*) "removing branch: " << *xid);

    for (XABranchMap::iterator i = branches_.begin(); i != branches_.end(); ++i)
    {
        if (compareXids(i->first, xid) == 0) {
            XAResourceAdaptorImpl *r = i->second;
            XID * key = i->first;
            PortableServer::ObjectId_var id(poa_->servant_to_id(r));
#ifdef XYZ
            r->_remove_ref();    // deactivate will delete r
#endif
            poa_->deactivate_object(id.in());
            branches_.erase(i->first);

            free(key);

            return;
        }
    }

    LOG4CXX_TRACE(xarmlogger, (char*) "... unknown branch");
}

int XAResourceManager::xa_start (XID * xid, long flags)
{
    FTRACE(xarmlogger, "ENTER " << rmid_ << (char *) ": flags=" << std::hex << flags << " lookup XID: " << *xid);
    XAResourceAdaptorImpl * resource = locateBranch(xid);
    int rv;

    if (resource == NULL) {
        FTRACE(xarmlogger, "creating branch " << *xid);
        if ((rv = createServant(xid)) != XA_OK)
            return rv;

        if ((resource = locateBranch(xid)) == NULL)    // cannot be NULL
            return XAER_RMERR;

        FTRACE(xarmlogger, "starting branch " << *xid);
        return resource->xa_start(TMNOFLAGS);
    }

    FTRACE(xarmlogger, "existing branch " << *xid);
//    return resource->xa_start(flags);
    return resource->xa_start(TMRESUME);

//    if (flags | (TMRESUME & TMJOIN))
//        return  resource->xa_start(flags);
//    else
//        return  resource->xa_start(TMJOIN);
}

int XAResourceManager::xa_end (XID * xid, long flags)
{
    FTRACE(xarmlogger, "ENTER end branch " << *xid << " rmid=" << rmid_ << " flags=" << std::hex << flags);
    XAResourceAdaptorImpl * resource = locateBranch(xid);

    if (resource == NULL) {
        LOG4CXX_WARN(xarmlogger, (char *) " no such branch " << *xid);
        return XAER_NOTA;
    }

    return resource->xa_end(flags);
}

XAResourceAdaptorImpl * XAResourceManager::locateBranch(XID * xid)
{
    FTRACE(xarmlogger, "ENTER");
    XABranchMap::iterator iter;

    for (iter = branches_.begin(); iter != branches_.end(); ++iter) {
        LOG4CXX_TRACE(xarmlogger, (char *) "compare: " << *xid << " with " << *(iter->first));

        if (compareXids(iter->first, xid) == 0)
            return (*iter).second;
    }

    return NULL;
}

int XAResourceManager::xa_flags()
{
    return xa_switch_->flags;
}

XID XAResourceManager::gen_xid(XID &gid)
{
    XID xid = {gid.formatID, gid.gtrid_length};
    int i;

    for (i = 0; i < gid.gtrid_length; i++)
        xid.data[i] = gid.data[i];

    // TODO improve on the uniqueness (eg include IP)
    ACE_Time_Value now = ACE_OS::gettimeofday();
    (void) sprintf(xid.data + i, "%ld%ld", now.sec(), now.usec());
    xid.bqual_length = strlen(xid.data + i);

    return xid;
}
