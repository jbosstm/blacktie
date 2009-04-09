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


bool XAResourceManagerFactory::getXID(XID& xid)
{
	CosTransactions::Control_ptr cp = (CosTransactions::Control_ptr) getSpecific(TSS_KEY);

	if (CORBA::is_nil(cp))
		return false;

	try {
		CosTransactions::Coordinator_var cv = cp->get_coordinator();
		CosTransactions::PropagationContext_var pcv = cv->get_txcontext();
		CosTransactions::otid_t otid = pcv->current.otid;
		int len = otid.tid.length();

		xid.formatID = otid.formatID;
		xid.bqual_length = otid.bqual_length;
		xid.gtrid_length = otid.tid.length() - otid.bqual_length;

		for (int i = 0; i < len; i++)
		 	xid.data[i] = otid.tid[i];

		return true;
	} catch (CosTransactions::Unavailable & e) {
		return false;
	}
}

static int _rm_start(XAResourceManager* rm, XID& xid, long flags)
{
	LOG4CXX_TRACE(xaResourceLogger,  (char *) "_rm_start xid="
		<< xid.formatID << ':'
		<< xid.gtrid_length << ':'
		<< xid.bqual_length << ':'
		<< xid.data
		<< (char *) " flags=" << flags);

	return rm->xa_start(&xid, rm->rmid(), flags);
}
static int _rm_end(XAResourceManager* rm, XID& xid, long flags)
{
	LOG4CXX_TRACE(xaResourceLogger,  (char *) "_rm_end xid="
		<< xid.formatID << ':'
		<< xid.gtrid_length << ':'
		<< xid.bqual_length << ':'
		<< xid.data
		<< (char *) " flags=" << flags);

	return rm->xa_end(&xid, rm->rmid(), flags);
}

static int _rmiter(ResourceManagerMap& rms, int (*func)(XAResourceManager *, XID&, long), int flags)
{
	XID xid;

	if (!XAResourceManagerFactory::getXID(xid))
		return XAER_NOTA;

	for (ResourceManagerMap::iterator i = rms.begin(); i != rms.end(); ++i) {
		XAResourceManager * rm = i->second;
		int rc = func(rm, xid, flags);

		if (rc != XA_OK) {
			LOG4CXX_DEBUG(xaResourceLogger,  (char *) "rm operation failed " << rc);
			return rc;
		}
	}

	return XA_OK;
}

XAResourceManagerFactory::XAResourceManagerFactory()
{
}

XAResourceManagerFactory::~XAResourceManagerFactory()
{
	destroyRMs(NULL);
}

XAResourceManager * XAResourceManagerFactory::findRM(long id)
{
	ResourceManagerMap::iterator i = rms_.find(id);

	return (i == rms_.end() ? NULL : i->second);
}

void XAResourceManagerFactory::destroyRMs(CORBA_CONNECTION * connection)
{
	for (ResourceManagerMap::iterator i = rms_.begin(); i != rms_.end(); ++i)
		delete i->second;

	rms_.clear();
}

int XAResourceManagerFactory::startRMs(CORBA_CONNECTION * connection)
{
	// there is a current transaction (otherwise the call doesn't need to start the RMs
	return _rmiter(rms_, _rm_start, TMNOFLAGS);
}
int XAResourceManagerFactory::endRMs(CORBA_CONNECTION * connection)
{
	return _rmiter(rms_, _rm_end, TMSUCCESS);
}
int XAResourceManagerFactory::suspendRMs(CORBA_CONNECTION * connection)
{
	return _rmiter(rms_, _rm_end, TMSUSPEND);
}
int XAResourceManagerFactory::resumeRMs(CORBA_CONNECTION * connection)
{
	return _rmiter(rms_, _rm_start, TMRESUME);
}

void XAResourceManagerFactory::createRMs(CORBA_CONNECTION * connection) throw (RMException)
{
	if (rms_.size() == 0) {
		xarm_config_t * rmp = (xarmp == 0 ? 0 : xarmp->head);

		while (rmp != 0) {
			LOG4CXX_TRACE(xaResourceLogger,  (char*) "createRM:"
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
	// make sure the XA_RESOURCE XML config is valid
	if (rmp->resourceMgrId == 0 || rmp->xasw == NULL || rmp->xalib == NULL) {
		LOG4CXX_DEBUG(xaResourceLogger, 
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
		LOG4CXX_INFO(xaResourceLogger, 
			(char *) "Duplicate RM with id " << rmp->resourceMgrId);

		RMException ex("RMs must have unique ids", EINVAL);
		throw ex;
	}

	void * symbol = lookup_symbol(rmp->xalib, rmp->xasw);
	LOG4CXX_TRACE(xaResourceLogger,  (char *) "got symbol");
	ptrdiff_t tmp = reinterpret_cast<ptrdiff_t> (symbol);
	LOG4CXX_TRACE(xaResourceLogger,  (char *) "cast to ptr");
	struct xa_switch_t * xa_switch = reinterpret_cast<struct xa_switch_t *>(tmp);
	LOG4CXX_TRACE(xaResourceLogger,  (char *) "cast to struct");

	if (xa_switch == NULL) {
		LOG4CXX_ERROR(xaResourceLogger, 
			(char *) " xa_switch " << rmp->xasw << (char *) " not found in library " << rmp->xalib);
		RMException ex("Could not find xa_switch in library", 0);
		throw ex;
	}

	LOG4CXX_TRACE(xaResourceLogger,  (char *) "creating xarm");
	LOG4CXX_TRACE(xaResourceLogger,  (char *) "using" << xa_switch->name);
	XAResourceManager * a = new XAResourceManager(
		connection, rmp->resourceName, rmp->openString, rmp->closeString, rmp->resourceMgrId, xa_switch);
	LOG4CXX_TRACE(xaResourceLogger,  (char *) "created xarm");

	if (a != NULL)
		rms_[rmp->resourceMgrId] = a;
	
	LOG4CXX_TRACE(xaResourceLogger,  (char *) "assigned rms_");

	return a;
}
