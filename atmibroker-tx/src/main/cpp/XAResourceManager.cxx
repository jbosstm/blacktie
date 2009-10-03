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

SynchronizableObject* XAResourceManager::lock = new SynchronizableObject();
int XAResourceManager::counter = 0;

ostream& operator<<(ostream &os, const XID& xid)
{
	os << xid.formatID << ':' << xid.gtrid_length << ':' << xid.bqual_length << ':' << xid.data;
	return os;
}

void XAResourceManager::show_branches(const char *msg, XID * xid)
{
	FTRACE(xarmlogger, "ENTER " << *xid);

	for (XABranchMap::iterator i = branches_.begin(); i != branches_.end(); ++i) {

		LOG4CXX_TRACE(xarmlogger, (char *) "XID: " << (i->first)); 
	}
}

static int compareXids(const XID& xid1, const XID& xid2)
{
	char *x1 = (char *) &xid1;
	char *x2 = (char *) &xid2;
	char *e = (char *) (x1 + sizeof (XID));

	while (x1 < e)
		if (*x1 < *x2)
			return -1;
		else if (*x1++ > *x2++)
			return 1;

	return 0;
}

bool xid_cmp::operator()(const XID& xid1, const XID& xid2) {
	return (compareXids(xid1, xid2) < 0);
}

XAResourceManager::XAResourceManager(
	CORBA_CONNECTION* connection,
	const char * name,
	const char * openString,
	const char * closeString,
	CORBA::Long rmid,
	struct xa_switch_t * xa_switch,
	XARecoveryLog& log, PortableServer::POA_ptr poa) throw (RMException) :

	poa_(poa), connection_(connection), name_(name), openString_(openString), closeString_(closeString),
	rmid_(rmid), xa_switch_(xa_switch), rclog_(log) {

	FTRACE(xarmlogger, "ENTER " << (char *) "new RM name: " << name << (char *) " openinfo: " <<
		openString << (char *) " rmid: " << rmid_);

	if (name == NULL) {
		RMException ex("Invalid RM name", EINVAL);
		throw ex;
	}

	int rv = xa_switch_->xa_open_entry((char *) openString, rmid_, TMNOFLAGS);

	LOG4CXX_TRACE(xarmlogger,  (char *) "xa_open: " << rv);

	if (rv != XA_OK) {
		LOG4CXX_ERROR(xarmlogger,  (char *) "xa_open error: " << rv);

		RMException ex("xa_open", rv);
		throw ex;
	}

	// each RM has its own POA
//	createPOA();
}

XAResourceManager::~XAResourceManager() {
	FTRACE(xarmlogger, "ENTER");
	int rv = xa_switch_->xa_close_entry((char *) closeString_, rmid_, TMNOFLAGS);

	LOG4CXX_TRACE(xarmlogger, (char *) "xa_close: " << rv);

	if (rv != XA_OK)
		LOG4CXX_WARN(xarmlogger, (char *) " close RM " << name_ << " failed: " << rv);
}

int XAResourceManager::recover(XID& bid, const char* rc)
{
	FTRACE(xarmlogger, "ENTER");

	CORBA::Object_var ref = connection_->orbRef->string_to_object(rc);
	XAResourceAdaptorImpl *ra = NULL;

	if (CORBA::is_nil(ref)) {
		LOG4CXX_INFO(xarmlogger, (char *) "Invalid recovery coordinator ref: " << rc);
		return -1;
	} else {
		CosTransactions::RecoveryCoordinator_var rcv = CosTransactions::RecoveryCoordinator::_narrow(ref);

		if (CORBA::is_nil(rcv)) {
			LOG4CXX_INFO(xarmlogger, (char *) "Could not narrow recovery coordinator ref: " << rc);
		} else {
			ra = new XAResourceAdaptorImpl(this, bid, bid, rmid_, xa_switch_, rclog_);

			if (branches_[bid] != NULL) {
				// log an error since we forgot to clean up the previous servant
				LOG4CXX_ERROR(xarmlogger, (char *) "Recovery: branch already exists: " << bid);
			}

			// activate the servant
#if 0
			PortableServer::ObjectId_var objId = poa_->activate_object(ra);

			// get a CORBA reference to the servant so that it passed the TM so that phase 2 can be replayed
			CORBA::Object_var ref = poa_->servant_to_reference(ra);
#else
			std::string s = (char *) (bid.data + bid.gtrid_length);
			PortableServer::ObjectId_var objId = PortableServer::string_to_ObjectId(s.c_str());
			poa_->activate_object_with_id(objId, ra);
			// get a CORBA reference to the servant so that it can be enlisted in the OTS transaction
			CORBA::Object_var ref = poa_->id_to_reference(objId.in());
#endif
			ra->_remove_ref();	// now only the POA has a reference to ra

			CosTransactions::Resource_var v = CosTransactions::Resource::_narrow(ref);

			LOG4CXX_DEBUG(xarmlogger, (char*) "Recovering resource with branch id: " << bid <<
				" and recovery IOR: " << connection_->orbRef->object_to_string(v));

			try {
				/*
				 * Replay phase 2. The spec says we should use the same resource.
				 * If we really do need to use the same one then we need to reconstruct it
				 * with the same reference by setting the PortableServer::USER_ID
				 * policy on the creating POA (and use the same name based on the branch id).
				 *
				 * Remember to deal with heuristics (see section 2-50 of the OMG OTS spec).
				 */
				Status txs = rcv->replay_completion(v);

				LOG4CXX_DEBUG(xarmlogger, (char *) "Recovery: TM reports transaction status: " << txs);

				switch (txs) {
				case CosTransactions::StatusNoTransaction:
					LOG4CXX_INFO(xarmlogger, (char *) "Recovery: TM reports no such transaction");
					break;
				default:
					// TODO check spec to verify that the TM will replay phase 2 for all the other statuses
					branches_[bid] = ra;
					break;
				}
			} catch (CosTransactions::NotPrepared& e) {
				LOG4CXX_INFO(xarmlogger, (char *) "Recovery: TM says the transaction as not prepared: " << e._name());
			} catch (const CORBA::SystemException& e) {
				LOG4CXX_WARN(xarmlogger, (char*) "Recovery: replay error: " << e._name() << " minor: " << e.minor());
			}
		}
	}

//	if (ra)
//		delete ra;

	return XA_OK;	// means the caller is not allowed to clear the log
}

int XAResourceManager::createServant(XID& xid)
{
	FTRACE(xarmlogger, "ENTER");
	int res = XA_OK;
	XAResourceAdaptorImpl *ra = NULL;
	CosTransactions::Control_ptr curr = (CosTransactions::Control_ptr) txx_get_control();
	CosTransactions::Coordinator_ptr coord = NULL;

	if (CORBA::is_nil(curr))
		return XAER_NOTA;

	try {
		// create a servant to represent the new branch identified by xid
		XID bid = gen_xid(rmid_, xid);
		ra = new XAResourceAdaptorImpl(this, xid, bid, rmid_, xa_switch_, rclog_);
#if 0
		// and activate it
		PortableServer::ObjectId_var objId = poa_->activate_object(ra);
		// get a CORBA reference to the servant so that it can be enlisted in the OTS transaction
		CORBA::Object_var ref = poa_->servant_to_reference(ra);
#else
		std::string s = (char *) (bid.data + bid.gtrid_length);
		PortableServer::ObjectId_var objId = PortableServer::string_to_ObjectId(s.c_str());
		poa_->activate_object_with_id(objId, ra);
		// get a CORBA reference to the servant so that it can be enlisted in the OTS transaction
		CORBA::Object_var ref = poa_->id_to_reference(objId.in());
#endif

		ra->_remove_ref();	// now only the POA has a reference to ra

		CosTransactions::Resource_var v = CosTransactions::Resource::_narrow(ref);

		// enlist it with the transaction
		LOG4CXX_TRACE(xarmlogger, (char*) "enlisting resource: "); // << connection_->orbRef->object_to_string(v));
		coord = curr->get_coordinator();
		CosTransactions::RecoveryCoordinator_ptr rc = coord->register_resource(v);
		//c->register_synchronization(new XAResourceSynchronization(xid, rmid_, xa_switch_));

		if (CORBA::is_nil(rc)) {
			LOG4CXX_TRACE(xarmlogger, (char*) "createServant: nill RecoveryCoordinator ");
			res = XAER_RMFAIL;
		} else {
			CORBA::String_var rcref = connection_->orbRef->object_to_string(rc);
			ra->set_recovery_coordinator(ACE_OS::strdup(rcref));
	   		CORBA::release(rc);

			branches_[xid] = ra;
			res = XA_OK;
			ra = NULL;
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

void XAResourceManager::notify_error(XID * xid, int xa_error, bool forget)
{
	FTRACE(xarmlogger, "ENTER: reason: " << xa_error);

	if (forget)
		set_complete(xid);
}

void XAResourceManager::set_complete(XID * xid)
{
	FTRACE(xarmlogger, "ENTER");
	XABranchMap::iterator iter;

	LOG4CXX_TRACE(xarmlogger, (char*) "removing branch: " << *xid);

	if (rclog_.del_rec(*xid) != 0) {
		// probably a readonly resource
		LOG4CXX_TRACE(xarmlogger, (char*) "branch completion notification with out a corresponding log entry: " << *xid);
	}

	for (XABranchMap::iterator i = branches_.begin(); i != branches_.end(); ++i)
	{
		if (compareXids(i->first, (const XID&) (*xid)) == 0) {
			XAResourceAdaptorImpl *r = i->second;
			PortableServer::ObjectId_var id(poa_->servant_to_id(r));
#ifdef XYZ
			r->_remove_ref();	// deactivate will delete r
#endif
			poa_->deactivate_object(id.in());
			branches_.erase(i->first);

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
		if ((rv = createServant(*xid)) != XA_OK)
			return rv;

		if ((resource = locateBranch(xid)) == NULL)	// cannot be NULL
			return XAER_RMERR;

		FTRACE(xarmlogger, "starting branch " << *xid);
		return resource->xa_start(TMNOFLAGS);
	}

	FTRACE(xarmlogger, "existing branch " << *xid);
	return resource->xa_start(TMRESUME);
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
		LOG4CXX_TRACE(xarmlogger, (char *) "compare: " << *xid << " with " << (iter->first));

		if (compareXids(iter->first, (const XID&) (*xid)) == 0) {
			return (*iter).second;
		}
	}

	LOG4CXX_DEBUG(xarmlogger, (char *) " branch not found");
	return NULL;
}

int XAResourceManager::xa_flags()
{
	return xa_switch_->flags;
}

XID XAResourceManager::gen_xid(long id, XID &gid)
{
	FTRACE(xarmlogger, "ENTER");
	XID xid = {gid.formatID, gid.gtrid_length};
	int i;

	int myCounter = -1;
	lock->lock();
	myCounter = ++counter;
	lock->unlock();

	for (i = 0; i < gid.gtrid_length; i++)
		xid.data[i] = gid.data[i];

	// TODO improve on the uniqueness (eg include IP)
	ACE_Time_Value now = ACE_OS::gettimeofday();
	// the first long in the XID data must contain the RM id
	(void) sprintf(xid.data + i, "%d:%d:%d:%d", id, myCounter, now.sec(), now.usec());
	xid.bqual_length = strlen(xid.data + i);

	FTRACE(xarmlogger, "Leaving with XID: " << xid);
	return xid;
}
