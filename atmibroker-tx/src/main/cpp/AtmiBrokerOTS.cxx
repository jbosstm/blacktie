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

#ifdef TAO_COMP
#include <tao/ORB.h>
#include "tao/ORB_Core.h"
#include "ConnectorImpl.h"
#include "LocalResourceManagerCache.h"
#endif

#include "ThreadLocalStorage.h"
#include "XAResourceManagerFactory.h"

#include "TxInitializer.h"
#include "OrbManagement.h"
#include "AtmiBrokerOTS.h"
#include "AtmiBrokerEnvXml.h"

#include "tx.h"

#include "Worker.h"

#include "log4cxx/logger.h"

log4cxx::LoggerPtr loggerAtmiBrokerOTS(log4cxx::Logger::getLogger("AtmiBrokerOTS"));

/*
 * use different transaction branches in processes using the same resource manager within the same
 * distributed transaction (see section B.2.6 of the OMG OTS spec)
 */
#define XA_LOOSE_COUPLING

AtmiBrokerOTS *AtmiBrokerOTS::ptrAtmiBrokerOTS = NULL;

AtmiBrokerOTS *
AtmiBrokerOTS::get_instance() {
	if (ptrAtmiBrokerOTS == NULL)
		ptrAtmiBrokerOTS = new AtmiBrokerOTS();
	return ptrAtmiBrokerOTS;
}

void AtmiBrokerOTS::discard_instance() {
	if (ptrAtmiBrokerOTS != NULL) {
		delete ptrAtmiBrokerOTS;
		ptrAtmiBrokerOTS = NULL;
	}
}

AtmiBrokerOTS::AtmiBrokerOTS() {
	LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "constructor");
	nextControlId = 1;
	currentImpl = NULL;
	tx_current = NULL;
	ots_connection = init_orb((char*) "ots");
#ifdef XA_LOOSE_COUPLING
	try {
		xaRMFac.createRMs(ots_connection);
	} catch (RMException& ex) {
		LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getWarn(), (char*) "failed to load RMs: " << ex.what());
	}
#endif // XA_LOOSE_COUPLING
	//	createTransactionPolicy();
}

AtmiBrokerOTS::~AtmiBrokerOTS() {
	LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "destructor");
	shutdownBindings(ots_connection);
	/* TODO
	 if (xaResourceMgrId)
	 //free (xaResourceMgrId);
	 if (transFactoryId)
	 //free (transFactoryId);
	 if (xaResourceName)
	 //free (xaResourceName);
	 if (xaOpenString)
	 //free (xaOpenString);
	 if (xaCloseString)
	 //free (xaCloseString);
	 */
}

CORBA_CONNECTION* AtmiBrokerOTS::init_orb(char* name) {
	register_tx_interceptors(name);
	return ::initOrb(name);
}

int AtmiBrokerOTS::tx_open(void) {

	if (CORBA::is_nil(tx_current)) {
		LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "tx_open ");
		if (tx_factory == NULL) {
			if (transFactoryId != NULL && strlen(transFactoryId) != 0) {
				// TJJ resolving by nameservice "TransactionManagerService.OTS"
				LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "to_name/resolve TransactionService: " << transFactoryId);
				CosNaming::Name * name = getNamingContextExt()->to_name(transFactoryId);
				CORBA::Object_var obj = getNamingContextExt()->resolve(*name);
				LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "resolved TransactionService: " << (void*) obj);
				tx_factory = CosTransactions::TransactionFactory::_narrow(obj);
				LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "narrowed TransactionFactory: " << (void*) tx_factory);
				LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "Obtained TransactionService: " << transFactoryId);
			} else {
				return -1;
			}
		}

		LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "getTransactionCurrent");
		currentImpl = new CurrentImpl(tx_factory);
		tx_current = currentImpl;
	}
//	createXAConnectorAndResourceManager();

	return TX_OK;
}

void AtmiBrokerOTS::rm_resume(void) {
#ifdef XA_LOOSE_COUPLING
	xaRMFac.resumeRMs(ots_connection);
#endif // XA_LOOSE_COUPLING
}
void AtmiBrokerOTS::rm_suspend(void) {
#ifdef XA_LOOSE_COUPLING
	xaRMFac.suspendRMs(ots_connection);
#endif // XA_LOOSE_COUPLING
}

int AtmiBrokerOTS::tx_begin(void) {

	if (CORBA::is_nil(tx_current) || getSpecific(TSS_KEY)) {
		// either tx_open hasn't been called or already in a transaction
		LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "tx_begin: protocol violation");
		return TX_PROTOCOL_ERROR;
	}

	try {
		LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "tx_begin");
		tx_current->begin();
		setSpecific(TSS_KEY, tx_current->get_control());
		rm_resume();
		LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "called begin ");
		return TX_OK;
	} catch (...) {
		// TODO placeholder return the correct error code
		return TX_ERROR;
	}

}

int AtmiBrokerOTS::tx_commit(void) {
	if (CORBA::is_nil(tx_current) || getSpecific(TSS_KEY) == NULL) {
		// either tx_open hasn't been called or not in a transaction
		LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "tx_commit: protocol violation");
		return TX_PROTOCOL_ERROR;
	}

	try {
		LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "calling commit");
		tx_current->commit(false);
		// if we get an exception leave the tx associated - // TODO is the correct semantics
		destroySpecific(TSS_KEY);
		LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "called commit");
		return TX_OK;
	} catch (CORBA::TRANSACTION_ROLLEDBACK & aRef) {
		LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getError(), (char*) "transaction has been rolled back " << (void*) &aRef);
		return -1; // should be TX_ROLLBACK ... and all the other outcomes
	} catch (...) {
		// TODO placeholder return the correct error code
		return TX_ERROR;
	}
}

int AtmiBrokerOTS::tx_rollback(void) {
	if (CORBA::is_nil(tx_current) || getSpecific(TSS_KEY) == NULL) {
		// either tx_open hasn't been called or not in a transaction
		LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "tx_rollback: protocol violation");
		return TX_PROTOCOL_ERROR;
	}

	try {
		LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "calling rollback ");
		tx_current->rollback();
		destroySpecific(TSS_KEY);
		LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "called rollback ");

		return TX_OK;
	} catch (...) {
		// TODO placeholder return the correct error code
		return TX_ERROR;
	}
}

int AtmiBrokerOTS::suspend(long& tranid) {
	LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "suspend ");

	if (CORBA::is_nil(tx_current) || getSpecific(TSS_KEY) == NULL) {
		// either tx_open hasn't been called or not in a transaction
		LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "suspend: not in a transaction");
		return -1;
	} else {
		LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "calling suspend ");
		CosTransactions::Control_var aControl = tx_current->suspend();
		rm_suspend();
		destroySpecific(TSS_KEY);
		LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "called suspend and got Control " << (void*) aControl);
		ControlInfo* aControlInfo = (ControlInfo*) malloc(sizeof(ControlInfo) * 1);
		LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "created aControlInfo " << (void*) aControlInfo);
		aControlInfo->id = nextControlId;
		LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "populated aControlInfo with id " << aControlInfo->id);
		nextControlId++;
		aControlInfo->control = CosTransactions::Control::_duplicate(aControl);
		LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "populated aControlInfo with ctl " << (void*) aControlInfo->control);
		LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "adding aControlInfo" << (void*) aControlInfo << " with id " << aControlInfo->id << " to vector");
		controlInfoVector.push_back(aControlInfo);
		LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "added aControlInfo " << (void*) aControlInfo << " with id " << aControlInfo->id << " to vector");
		tranid = aControlInfo->id;
		return TX_OK;
	}
}

int AtmiBrokerOTS::resume(long tranid) {
	LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "resume ");

	if (!CORBA::is_nil(tx_current)) {

		for (std::vector<ControlInfo*>::iterator it = controlInfoVector.begin(); it != controlInfoVector.end(); it++) {
			//LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "next control id is: " << (char*) (*it)->id);
			if ((*it)->id == tranid) {
				LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "found matching id " << (*it)->id);

				LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "calling resume with Control " << (void*) (*it)->control);
				tx_current->resume((*it)->control);
				if (getSpecific(TSS_KEY)) {
					LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getWarn(),
							(char*) "resume: current transaction has not been suspended");
				}

				setSpecific(TSS_KEY, tx_current->get_control());
				rm_resume();
				LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "called resume with Control " << (void*) (*it)->control);

				LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "removing %p from vector" << (*it));
				controlInfoVector.erase(it);
				LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "removed from vector ");
				return TX_OK;
			}
		}
	} else {
		LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getError(), (char*) "NOT calling suspend, tx_current is NULL  ");
		return -1;
	}
	return -1;
}

CosTransactions::Control_ptr AtmiBrokerOTS::getSuspended(long tranid) {
	LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "tx_get ");

	for (std::vector<ControlInfo*>::iterator it = controlInfoVector.begin(); it != controlInfoVector.end(); it++)
		if ((*it)->id == tranid)
			return (*it)->control;

	return NULL;
}

int AtmiBrokerOTS::tx_close(void) {
	if (getSpecific(TSS_KEY)) {
		LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getWarn(),
				(char*) "tx_close: transaction still active");
		return TX_PROTOCOL_ERROR;
	}

	LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "tx_close ");

	//	LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "releasing tx_current");
	//	CORBA::release(tx_current);
	//	if (!CORBA::is_nil(tx_current))	// TODO find out who forgot to release it
	//		LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getWarn(), (char *) "tx_close: current not nil after release");

	tx_current = NULL;
	LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "released tx_current");

	LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "releasing xa_connector");
	CORBA::release(xa_connector);
	LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "released xa_connector");

	LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "releasing xa_resource_manager");
	CORBA::release(xa_resource_manager);
	LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "released xa_resource_manager");

#ifdef TAO_COMP
	LocalResourceManagerCache::discardLocalResourceManagerCache();
#endif
	return TX_OK;
}


void AtmiBrokerOTS::createXAConnectorAndResourceManager() {
	LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "createXAConnectorAndResourceManager");
	CORBA::Object_var tmp_ref;

	if (CORBA::is_nil(xa_connector)) {
		LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) " resolving XA Connector ");
#ifdef TAO_COMP
		xa_connector = new ConnectorImpl();
		LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) " created local XA Connector: " << (void*)xa_connector);
#else
		//#elif ORBIX_COMP
		tmp_ref = getOrb()->resolve_initial_references("XAConnector");
		LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) " resolved XA Connector: " << (void*) tmp_ref);

		xa_connector = XA::Connector::_narrow(tmp_ref);
		assert(!CORBA::is_nil(xa_connector));
		LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) " narrowed XA Connector: " << (void*) xa_connector);
#endif
	} else
	LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "already got XA Connector: " << (void*) xa_connector);

	if (CORBA::is_nil(xa_resource_manager)) {
		LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) " creating XA Resource Manager ");

		// READ XA DATA FROM ENVIRONMENT DESCRIPTOR  TEMP
#ifdef TAO_COMP
		LocalResourceManager * aLocalResourceManagerPtr = NULL;
		aLocalResourceManagerPtr= LocalResourceManagerCache::getLocalResourceManagerCache()->create_local_resource_manager(
				"xa_resource_managers:oracle",
				"",
				"",
				XA::THREAD,
				false,
				false,
				"/opt/oracle/install/lib/libclntsh.so",
				"xaosw");
		xa_resource_manager = aLocalResourceManagerPtr;
		xa_current_connection = new CurrentConnectionImpl(*aLocalResourceManagerPtr);

#else
		//#elif ORBIX_COMP
		/*
		 xa_resource_manager = ResourceManagerCache::getResourceManagerCache()->create_resource_manager(
		 "xa_resource_managers:oracle",
		 "",                     // open string specified in config
		 "",                     // close string specified in config,
		 XA::THREAD,             // thread model is THREAD or PROCESShread model
		 false,                  // no automatic association
		 false,                  // do not use dynamic registration
		 "/opt/oracle/install/lib/libclntsh.so",
		 "xaosw");
		 */
		xa_resource_manager = xa_connector->create_resource_manager("xa_resource_managers:oracle", xaosw, "", // open string specified in config
				"", // close string specified in config,
#ifndef VBC_COMP
				XA::THREAD, // thread model is THREAD or PROCESShread model
#else
				XA::THREAD_, // thread model is THREAD or PROCESShread model
#endif
				false, // no automatic association
				false, // do not use dynamic registration
				xa_current_connection);
#endif
		LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "created XA Resource Manager: " << (void*) xa_resource_manager);
		LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "created XA Current Connection: " << (void*) xa_current_connection);
	} else
	LOG4CXX_LOGLS(loggerAtmiBrokerOTS, log4cxx::Level::getDebug(), (char*) "already got XA Resource Manager : " << (void*) xa_resource_manager);
}

CurrentImpl *
AtmiBrokerOTS::getCurrentImpl() {
	return currentImpl;
}

CosTransactions::Current_var&
AtmiBrokerOTS::getCurrent() {
	return tx_current;
}

XA::CurrentConnection_var&
AtmiBrokerOTS::getXaCurrentConnection() {
	return xa_current_connection;
}

XA::ResourceManager_var&
AtmiBrokerOTS::getXaResourceManager() {
	return xa_resource_manager;
}

XA::Connector_var&
AtmiBrokerOTS::getXaConnector() {
	return xa_connector;
}

//struct xa_switch_t&
XA::XASwitch_ptr AtmiBrokerOTS::getXaosw() {
	return xaosw;
}

CORBA::ORB_ptr AtmiBrokerOTS::getOrb() {
	return (CORBA::ORB_ptr) ots_connection->orbRef;
}

CosNaming::NamingContextExt_ptr AtmiBrokerOTS::getNamingContextExt() {
	return (CosNaming::NamingContextExt_ptr) ots_connection->default_ctx;
}

CosNaming::NamingContext_ptr AtmiBrokerOTS::getNamingContext() {
	return (CosNaming::NamingContext_ptr) ots_connection->name_ctx;
}
