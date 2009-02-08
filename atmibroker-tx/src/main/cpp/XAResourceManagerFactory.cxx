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
#include "XAResourceManagerFactory.h"
#include "AtmiBrokerEnvXml.h"
#include "ThreadLocalStorage.h"

#include "ace/OS.h"
//#include "ace/os_include/os_dlfcn.h"

static int fn1(char *a, int i, long l) { return 0; }
static int fn2(XID *x, int i, long l) { return 0; }
static int fn3(XID *, long l1, int i, long l2) { return 0; }
static int fn4(int *ip1, int *ip2, int i, long l) { return 0; }
static struct xa_switch_t defSwitch = { "DummyRM", 0L, 0, fn1, fn1, /* open and close */
	fn2, fn2, fn2, fn2, fn2, /*start, end, rollback, prepare, commit */
	fn3, /* recover */
	fn2, /* forget */
	fn4 /* complete */
};


// put this in a common utility
static void * lookup_symbol(const char *lib, const char *symbol)
	throw (RMException)
{
	LOG4CXX_LOGLS(xaResourceLogger, log4cxx::Level::getTrace(),
		(char *) "lookup_symbol " << symbol << (char *) " in library " << lib);

	if (symbol == NULL || lib == NULL)
		return 0;

	// copied from see ResourceManagerCache
	ACE_SHLIB_HANDLE handle = ACE_OS::dlopen(lib, ACE_DEFAULT_SHLIB_MODE);

	if (!handle) {
		RMException ex(ACE_OS::dlerror(), 0);
		LOG4CXX_LOGLS(xaResourceLogger, log4cxx::Level::getTrace(),
			(char*) "lookup_symbol: " << symbol << (char *) " dlopen error: " << ACE_OS::dlerror());
		throw ex;
	}

	try {
		void * sym = ACE_OS::dlsym(handle, symbol);

		if (ACE_OS::dlerror()) {
			LOG4CXX_LOGLS(xaResourceLogger, log4cxx::Level::getTrace(),
				(char*) "lookup_symbol: " << symbol << (char *) " dlsym error: " << ACE_OS::dlerror());
			RMException ex(ACE_OS::dlerror(), 0);
			ACE_OS::dlclose(handle);
			throw ex;
		}

		LOG4CXX_LOGLS(xaResourceLogger, log4cxx::Level::getTrace(), (char *) "symbol addr=" << sym);

		return sym;
	} catch (std::exception& e) {
		RMException ex(e.what(), 0);
		throw ex;
	}

	return NULL;
}

static bool getXID(XID& xid)
{
	CosTransactions::Control_ptr cp = (CosTransactions::Control_ptr) getSpecific(TSS_KEY);

	if (CORBA::is_nil(cp))
		return false;

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
}

static int _rm_start(XAResourceManager* rm, XID& xid, long flags)
{
	LOG4CXX_LOGLS(xaResourceLogger, log4cxx::Level::getTrace(), (char *) "_rm_start xid="
		<< xid.formatID << ':'
		<< xid.gtrid_length << ':'
		<< xid.bqual_length << ':'
		<< xid.data
		<< (char *) " flags=" << flags);

	return rm->xa_start(&xid, rm->rmid(), flags);
}
static int _rm_end(XAResourceManager* rm, XID& xid, long flags)
{
	LOG4CXX_LOGLS(xaResourceLogger, log4cxx::Level::getTrace(), (char *) "_rm_end xid="
		<< xid.formatID << ':'
		<< xid.gtrid_length << ':'
		<< xid.bqual_length << ':'
		<< xid.data
		<< (char *) " flags=" << flags);

	return rm->xa_end(&xid, rm->rmid(), flags);
}

static void _rmiter(ResourceManagerMap& rms, int (*func)(XAResourceManager *, XID&, long), int flags)
{
	XID xid;
	if (!getXID(xid))
		return;

	ResourceManagerMap::iterator iter;

	for (iter = rms.begin(); iter != rms.end(); ++iter) {
		XAResourceManager * rm = (*iter).second;
		func(rm, xid, flags);
	}
}

XAResourceManagerFactory::XAResourceManagerFactory()
{
}

XAResourceManagerFactory::~XAResourceManagerFactory()
{
	destroyRMs(NULL);
}

XAResourceManager * XAResourceManagerFactory::findRM(const char *name)
{
	/*assert(name != NULL);*/
	return rms_[name];
}

void XAResourceManagerFactory::destroyRMs(CONNECTION * connection)
{
	ResourceManagerMap::iterator iter;

	for (iter = rms_.begin(); iter != rms_.end(); ++iter)
		delete (*iter).second;

	rms_.clear();
}

void XAResourceManagerFactory::startRMs(CONNECTION * connection)
{
	// there is a current transaction (otherwise the call doesn't need to start the RMs
	_rmiter(rms_, _rm_start, TMNOFLAGS);
}
void XAResourceManagerFactory::endRMs(CONNECTION * connection)
{
	_rmiter(rms_, _rm_end, TMSUCCESS);
}
void XAResourceManagerFactory::suspendRMs(CONNECTION * connection)
{
	_rmiter(rms_, _rm_end, TMSUSPEND);
}
void XAResourceManagerFactory::resumeRMs(CONNECTION * connection)
{
	_rmiter(rms_, _rm_start, TMRESUME);
}

void XAResourceManagerFactory::createRMs(CONNECTION * connection) throw (RMException)
{
	// TODO for each RM ... see AtmiBrokerEnvXml
	LOG4CXX_LOGLS(xaResourceLogger, log4cxx::Level::getTrace(), (char*) "createRM:"
		<< (char *) " xaResourceMgrId: " << xaResourceMgrId
		<< (char *) " xaResourceName: " << xaResourceName
		<< (char *) " xaOpenString: " << xaOpenString
		<< (char *) " xaCloseString: " << xaCloseString
		<< (char *) " xaSwitch: " << xaSwitchEnv
		<< (char *) " xaLibName: " << xaLibNameEnv
	);
	long rmid = atol(xaResourceMgrId);

	if (rmid == 0) {
		RMException ex = RMException(" XA_RESOURCE_MGR_ID env variable is not numeric", EINVAL);
		throw ex;
	}

	(void) createRM(connection, rmid, xaResourceName, xaOpenString, xaCloseString, xaSwitchEnv, xaLibNameEnv);
}

XAResourceManager * XAResourceManagerFactory::createRM(
	CONNECTION * connection,
	long rmid,
	const char * name,
	const char * openString,
	const char * closeString,
	const char * xaSwitchSym,
	const char * xaLibName)
	throw (RMException)
{
	XAResourceManager * a = findRM(name);

	if (a == NULL) {
		struct xa_switch_t * xa_switch = NULL;

		if (xaSwitchSym != NULL && strcmp(xaSwitchSym, "DefaultSwitch") == 0)
			xa_switch = &defSwitch;
		else
			xa_switch = (struct xa_switch_t *) lookup_symbol(xaLibName, xaSwitchSym);


		if (xa_switch == NULL) {
			LOG4CXX_LOGLS(xaResourceLogger, log4cxx::Level::getWarn(),
				(char *) " xa_switch " << xaSwitchSym << (char *) " not found in library " << xaLibName);
			RMException ex("Could not find xa_switch in library", 0);
			throw ex;
		}

		a = new XAResourceManager(connection, name, openString, closeString, rmid, xa_switch);
		rms_[name] = a;
	}

	return a;
}
