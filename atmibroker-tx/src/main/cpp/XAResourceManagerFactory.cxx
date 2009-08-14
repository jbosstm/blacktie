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
#include "XAResourceManagerFactory.h"
#include "ThreadLocalStorage.h"
#include "SymbolLoader.h"

#include "ace/DLL.h"
#include "ace/ACE.h"
#include "ace/OS.h"
#ifdef ACE_HAS_POSITION_INDEPENDENT_POINTERS
#include "ace/Based_Pointer_Repository.h"
#endif /* ACE_HAS_POSITION_INDEPENDENT_POINTERS */
#include "ace/Malloc_T.h"
#include "ace/MMAP_Memory_Pool.h"
#include "ace/PI_Malloc.h"
#include "ace/Null_Mutex.h"
#include "ace/Based_Pointer_T.h"

log4cxx::LoggerPtr xarflogger(log4cxx::Logger::getLogger("TxLogXAFactory"));

#if 0
static PortableServer::POA_var xyzpoa;
static XAResourceAdaptorImpl *rm_servant;

void XAResourceManagerFactory::test(CORBA_CONNECTION *connection, CosTransactions::Control_ptr &curr, long rmid)
{
    // root poa
    CORBA::Object_var obj = connection->orbRef->resolve_initial_references("RootPOA");
    PortableServer::POA_var rootp = PortableServer::POA::_narrow(obj);

    // policy for persisent refs and user assigned ids
    PortableServer::LifespanPolicy_var lifespan = rootp->create_lifespan_policy(PortableServer::PERSISTENT);
    PortableServer::IdAssignmentPolicy_var assign = rootp->create_id_assignment_policy(PortableServer::USER_ID);
    CORBA::PolicyList pl;
    pl.length(2);
    pl[0] = PortableServer::LifespanPolicy::_duplicate(lifespan);
    pl[1] = PortableServer::IdAssignmentPolicy::_duplicate(assign);

    // child poa for RMs
    PortableServer::POA_var xyzpoa = rootp->create_POA("RM1", PortableServer::POAManager::_nil(), pl);

    // create and activate a servant
    XID xid;
    XAResourceManagerFactory::getXID(xid);
    XAResourceManager *rm = findRM(rmid);
    LOG4CXX_DEBUG(xarflogger,  (char *) "creating new servant for rmid: " << rmid);
    rm_servant = new XAResourceAdaptorImpl(rm, &xid, rm->rmid(), rm->get_xa_switch());

    PortableServer::ObjectId_var oid = PortableServer::string_to_ObjectId("RM1_xares_1");
    xyzpoa->activate_object_with_id(oid, rm_servant);
    lifespan->destroy();
    assign->destroy();
    rm_servant->_remove_ref();

    // get an obj ref and enlist it with the TM
    try {
//        CORBA::Object_var ref = xyzpoa->id_to_reference (oid.in());
        CORBA::Object_var ref = xyzpoa->servant_to_reference(rm_servant);
        CosTransactions::Resource_var v = CosTransactions::Resource::_narrow(ref);
//        CosTransactions::Control_ptr curr = (CosTransactions::Control_ptr) get_control();
        CosTransactions::Coordinator_ptr c = curr->get_coordinator();
//        CORBA::release(curr);
LOG4CXX_DEBUG(xarflogger,  (char *) "registering resource");
        CosTransactions::RecoveryCoordinator_ptr rc = c->register_resource(v);
LOG4CXX_DEBUG(xarflogger,  (char *) "rc is " << rc);
        rm_servant->setRecoveryCoordinator(rc);
        CORBA::release(c);
    } catch (const CORBA::SystemException& e) {
        LOG4CXX_WARN(xarflogger, (char*) "mem_test ex: " << e._name() << " minor: " << e.minor());
    }
LOG4CXX_DEBUG(xarflogger,  (char *) "TEST 10");
}
#endif

bool XAResourceManagerFactory::getXID(XID& xid)
{
    FTRACE(xarflogger, "ENTER");
    CosTransactions::Control_ptr cp = (CosTransactions::Control_ptr) txx_get_control();
    bool ok = false;

    if (CORBA::is_nil(cp)) {
        LOG4CXX_WARN(xarflogger,  (char *) "getXID: no tx associated with the callers thread");
        return false;
    }

    try {
        CosTransactions::Coordinator_var cv = cp->get_coordinator();
        CosTransactions::PropagationContext_var pcv = cv->get_txcontext();
        CosTransactions::otid_t otid = pcv->current.otid;
#if 0
        int len = otid.tid.length();

        xid.formatID = otid.formatID;
        xid.bqual_length = otid.bqual_length;
        xid.gtrid_length = otid.tid.length() - otid.bqual_length;

        for (int i = 0; i < len; i++)
             xid.data[i] = otid.tid[i];

#else
// TODO com.arjuna.ats.jts.utils.Utility.uidToOtid is not OTS complient
// duplicate what JBossTS does - will be fixed in JBossTS 4.8.0 (see JBTM-577)
        char JBOSSTS_NODE_SEPARATOR = '-';

        memset(&xid, 0, sizeof (XID));
        xid.formatID = otid.formatID;
        xid.gtrid_length = 0; 
        xid.bqual_length = otid.bqual_length; 

        LOG4CXX_TRACE(xarflogger,  (char *) "converting OTS tid: ");
        for (int i = 0; i < otid.bqual_length; i++) {
            if (otid.tid[i] == JBOSSTS_NODE_SEPARATOR) {
                xid.gtrid_length = i; 
                i += 1;    // skip pass separator
                xid.bqual_length = otid.bqual_length - i;

                // copy bqual into the xid
                for (int j = i; j < otid.bqual_length; j++)
                     xid.data[j] = otid.tid[j];
                break;
            }

            xid.data[i] = otid.tid[i];
        }
        LOG4CXX_TRACE(xarflogger,  (char *) "converted OTS tid len:" <<
            otid.tid.length() << (char *) " otid bqual len: " << otid.bqual_length <<
            (char *) " gtrid: " << xid.gtrid_length << (char *) " bqual: " << xid.bqual_length);

#endif
        ok = true;
    } catch (CosTransactions::Unavailable & e) {
        LOG4CXX_ERROR(xarflogger,  (char *) "XA-compatible Transaction Service raised unavailable");
    } catch (const CORBA::OBJECT_NOT_EXIST &e) {
        LOG4CXX_ERROR(xarflogger,  (char *) "Unexpected exception converting xid: " << e._name());
    } catch  (CORBA::Exception& e) {
        LOG4CXX_ERROR(xarflogger,  (char *) "Unexpected exception converting xid: " << e._name());
    } catch  (...) {
        LOG4CXX_ERROR(xarflogger,  (char *) "Unexpected generic exception converting xid");
    }

    txx_release_control(cp);

    return ok;
}

static int _rm_start(XAResourceManager* rm, XID& xid, long flags)
{
    FTRACE(xarflogger, "ENTER");
    LOG4CXX_TRACE(xarflogger,  (char *) "_rm_start xid="
        << xid.formatID << ':'
        << xid.gtrid_length << ':'
        << xid.bqual_length << ':'
        << xid.data
        << (char *) " flags=" << std::hex << flags);

    return rm->xa_start(&xid, rm->rmid(), flags);
}
static int _rm_end(XAResourceManager* rm, XID& xid, long flags)
{
    FTRACE(xarflogger, "ENTER");
    LOG4CXX_TRACE(xarflogger,  (char *) "_rm_end xid="
        << xid.formatID << ':'
        << xid.gtrid_length << ':'
        << xid.bqual_length << ':'
        << xid.data
        << (char *) " flags=" << std::hex << flags);

    return rm->xa_end(&xid, rm->rmid(), flags);
}

static int _rmiter(ResourceManagerMap& rms, int (*func)(XAResourceManager *, XID&, long), int flags)
{
    FTRACE(xarflogger, "ENTER");
    XID xid;

    if (!XAResourceManagerFactory::getXID(xid))
        return XAER_NOTA;

    for (ResourceManagerMap::iterator i = rms.begin(); i != rms.end(); ++i) {
        XAResourceManager * rm = i->second;
        int rc = func(rm, xid, flags);

        if (rc != XA_OK) {
            LOG4CXX_DEBUG(xarflogger,  (char *) "rm operation on " << rm->name() << " failed");
            return rc;
        }
        LOG4CXX_TRACE(xarflogger,  (char *) "rm operation on " << rm->name() << " ok");
    }

    return XA_OK;
}

XAResourceManagerFactory::XAResourceManagerFactory()
{
    FTRACE(xarflogger, "ENTER");
}

XAResourceManagerFactory::~XAResourceManagerFactory()
{
    FTRACE(xarflogger, "ENTER");
    destroyRMs();

//    CORBA::release(xyzpoa);
}

XAResourceManager * XAResourceManagerFactory::findRM(long id)
{
    FTRACE(xarflogger, "ENTER");
    ResourceManagerMap::iterator i = rms_.find(id);

    return (i == rms_.end() ? NULL : i->second);
}

void XAResourceManagerFactory::destroyRMs()
{
    FTRACE(xarflogger, "ENTER");
    for (ResourceManagerMap::iterator i = rms_.begin(); i != rms_.end(); ++i)
        delete i->second;

    rms_.clear();
}

int XAResourceManagerFactory::startRMs(int flags)
{
    FTRACE(xarflogger, "ENTER");
    LOG4CXX_DEBUG(xarflogger, (char *) " starting RMs flags=0x" << std::hex << flags);
    // there is a current transaction (otherwise the call doesn't need to start the RMs
    return _rmiter(rms_, _rm_start, flags);
}
int XAResourceManagerFactory::endRMs(int flags)
{
    FTRACE(xarflogger, "ENTER");
    LOG4CXX_DEBUG(xarflogger,  (char *) "end RMs flags=0x" << std::hex << flags);
    return _rmiter(rms_, _rm_end, flags);
}

void XAResourceManagerFactory::createRMs(CORBA_CONNECTION * connection) throw (RMException)
{
    FTRACE(xarflogger, "ENTER");
    if (rms_.size() == 0) {
        xarm_config_t * rmp = (xarmp == 0 ? 0 : xarmp->head);

        while (rmp != 0) {
            LOG4CXX_TRACE(xarflogger,  (char*) "createRM:"
                << (char *) " xaResourceMgrId: " << rmp->resourceMgrId
                << (char *) " xaResourceName: " << rmp->resourceName
                << (char *) " xaOpenString: " << rmp->openString
                << (char *) " xaCloseString: " << rmp->closeString
                << (char *) " xaSwitch: " << rmp->xasw
                << (char *) " xaLibName: " << rmp->xalib
            );

            (void) createRM(connection, rmp);

            rmp = rmp->next;
        }
    }
}

/**
 * Create a Resource Manager proxy for a XA compliant RM.
 * RMs must have a unique rmid.
 * A separate POA is created for each RM whose name is
 * derived from the unique rmid. The POA is responsible for
 * generating servants that correspond to each transaction branch
 * (a branch is created when start on the RM is called).
 */
XAResourceManager * XAResourceManagerFactory::createRM(
    CORBA_CONNECTION * connection,
    xarm_config_t *rmp)
    throw (RMException)
{
    FTRACE(xarflogger, "ENTER");
    // make sure the XA_RESOURCE XML config is valid
    if (rmp->resourceMgrId == 0 || rmp->xasw == NULL || rmp->xalib == NULL) {
        LOG4CXX_DEBUG(xarflogger, 
            (char *) "Bad XA_RESOURCE config: "
            << " rmid: " << rmp->resourceMgrId
            << " xaswitch symbol: " << rmp->xasw
            << " xa lib name: " << rmp->xalib);

        //destroyRMs(NULL);
        RMException ex = RMException("Invalid XA_RESOURCE XML config", EINVAL);
        throw ex;
    }

    // Check that rmid is unique
    XAResourceManager * id = findRM(rmp->resourceMgrId);

    if (id != 0) {
        LOG4CXX_INFO(xarflogger, 
            (char *) "Duplicate RM with id " << rmp->resourceMgrId);

        RMException ex("RMs must have unique ids", EINVAL);
        throw ex;
    }

    void * symbol = lookup_symbol(rmp->xalib, rmp->xasw);
    LOG4CXX_TRACE(xarflogger,  (char *) "got symbol");
    ptrdiff_t tmp = reinterpret_cast<ptrdiff_t> (symbol);
    LOG4CXX_TRACE(xarflogger,  (char *) "cast to ptr");
    struct xa_switch_t * xa_switch = reinterpret_cast<struct xa_switch_t *>(tmp);
    LOG4CXX_TRACE(xarflogger,  (char *) "cast to struct");

    if (xa_switch == NULL) {
        LOG4CXX_ERROR(xarflogger, 
            (char *) " xa_switch " << rmp->xasw << (char *) " not found in library " << rmp->xalib);
        RMException ex("Could not find xa_switch in library", 0);
        throw ex;
    }

    LOG4CXX_TRACE(xarflogger,  (char *) "creating xa rm: " << xa_switch->name);
    XAResourceManager * a = new XAResourceManager(
        connection, rmp->resourceName, rmp->openString, rmp->closeString, rmp->resourceMgrId, xa_switch);
    LOG4CXX_TRACE(xarflogger,  (char *) "created xarm");

    if (a != NULL)
        rms_[rmp->resourceMgrId] = a;
    
    LOG4CXX_TRACE(xarflogger,  (char *) "assigned rms_");

    return a;
}
