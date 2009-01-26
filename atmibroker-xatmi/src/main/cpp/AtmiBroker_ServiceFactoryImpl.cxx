/*
 * JBoss, Home of Professional Open Source
 * Copyright 2008, Red Hat Middleware LLC, and others contributors as indicated
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
/*
 * BREAKTHRUIT PROPRIETARY - NOT TO BE DISCLOSED OUTSIDE BREAKTHRUIT, LLC.
 */
// copyright 2006, 2008 BreakThruIT

//
// Servant which implements the AtmiBroker::ServiceFactory interface.
//
#ifdef TAO_COMP
#include <tao/ORB.h>
#include "tao/ORB_Core.h"
#include "AtmiBrokerC.h"
#elif ORBIX_COMP
#include <omg/orb.hh>
#endif
#ifdef VBC_COMP
#include <orb.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "AtmiBroker_ServiceFactoryImpl.h"

#include "AtmiBroker.h"
#include "AtmiBrokerServer.h"
#include "AtmiBroker_ServiceImpl.h"
#include "AtmiBrokerServiceXml.h"

#include "xatmi.h"
#include "userlog.h"

#include "log4cxx/logger.h"
using namespace log4cxx;
using namespace log4cxx::helpers;
LoggerPtr loggerAtmiBroker_ServiceFactoryImpl(Logger::getLogger("AtmiBroker_ServiceFactoryImpl"));

// _create() -- create a new servant.
// Hides the difference between direct inheritance and tie servants
// For direct inheritance, simple create and return an instance of the servant.
// For tie, creates an instance of the tied class and the tie, return the tie.
//
/*
 POA_AtmiBroker::ServiceFactory*
 AtmiBroker_ServiceFactoryImpl::_create(PortableServer::POA_ptr the_poa)
 {
 return new AtmiBroker_ServiceFactoryImpl(the_poa);
 }
 */

// AtmiBroker_ServiceFactoryImpl constructor
//
// Note: since we use virtual inheritance, we must include an
// initialiser for all the virtual base class constructors that
// require arguments, even those that we inherit indirectly.
//
AtmiBroker_ServiceFactoryImpl::AtmiBroker_ServiceFactoryImpl(PortableServer::POA_ptr the_poa, char *svcName, char *svcPoaName, char *svcDescriptror

) :
	IT_ServantBaseOverrides(the_poa), serviceName(svcName), servicePoaName(svcPoaName), descriptorFileName(svcDescriptror), factoryPoaPtr(the_poa) {
	// Intentionally empty.
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceFactoryImpl, (char*) "constructor() ");

	serviceInfo.maxSize = MAX_SERVICE_CACHE_SIZE;
	serviceInfo.securityType = CORBA::string_dup("");
	getDescriptorData();
}

void AtmiBroker_ServiceFactoryImpl::createServantCache(void(*func)(TPSVCINFO *)) {
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceFactoryImpl, (char*) "createServantCache() ");

	for (int i = 0; i < serviceInfo.minSize; i++) {
		createCacheInstance(i, func);
	}
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceFactoryImpl, (char*) "createServantCache done ");
}

void AtmiBroker_ServiceFactoryImpl::createCacheInstance(int i, void(*func)(TPSVCINFO *)) {
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceFactoryImpl, (char*) "createCacheInstance(int i): %d", i);

	AtmiBroker_ServiceImpl * tmp_servant;
	AtmiBroker::Service_var* tmp_service_ref = new AtmiBroker::Service_var();
	PortableServer::ObjectId * objectId;

	userlog(Level::getDebug(), loggerAtmiBroker_ServiceFactoryImpl, (char*) "createServant ");

	createPoa();

	userlog(Level::getDebug(), loggerAtmiBroker_ServiceFactoryImpl, (char*) "constucting servant");
	tmp_servant = createServant(i, func);

	servicePoaPtr->activate_object(tmp_servant);
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceFactoryImpl, (char*) "activated tmp_servant %p", (void*) tmp_servant);

	objectId = servicePoaPtr->servant_to_id(tmp_servant);
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceFactoryImpl, (char*) "id for servant %p is %p", (void*) tmp_servant, (void*) objectId);

	userlog(Level::getDebug(), loggerAtmiBroker_ServiceFactoryImpl, (char*) "constucting reference");
	createReference(*objectId, tmp_service_ref);

	corbaObjectVector.push_back(*tmp_service_ref);
	servantVector.push_back(tmp_servant);
}

// ~AtmiBroker_ServiceFactoryImpl destructor.
//
AtmiBroker_ServiceFactoryImpl::~AtmiBroker_ServiceFactoryImpl() {
	//	std::vector<AtmiBroker::Service_var>::iterator i = corbaObjectVector.begin();
	//	while (i != corbaObjectVector.end()) {
	//		CORBA::release((*i));
	//		i = corbaObjectVector.erase(i);
	//		//		delete servantVector[i];
	//	}
	//	factoryPoaPtr->destroy(false, true);
}

// start_conversation() -- Implements IDL operation "AtmiBroker::ServiceFactory::start_conversation".
//
char *
AtmiBroker_ServiceFactoryImpl::start_conversation(CORBA::Long client_id, CORBA::String_out id) throw (CORBA::SystemException ) {
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceFactoryImpl, (char*) "start_conversation()");

	int index = 0;
	CORBA::Boolean found = false;
	for (index = 0; index < serviceInfo.maxSize; index++) {
		if (servantVector[index] != NULL && !servantVector[index]->isInUse()) {
			servantVector[index]->setInUse(true);
			servantVector[index]->setClientId(client_id);
			found = true;
			break;
		}
	}
	char *idStr = (char*) malloc(sizeof(char) * (XATMI_SERVICE_NAME_LENGTH*2));

	strcpy(idStr, serviceName);
	strcat(idStr, ":");
	// ltoa
	std::ostringstream oss;
	oss << index << std::dec;
	const char* indexStr = oss.str().c_str();

	strcat(idStr, indexStr);
	id = CORBA::string_dup(idStr);
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceFactoryImpl, (char*) "id %s", (char*) id);

	userlog(Level::getDebug(), loggerAtmiBroker_ServiceFactoryImpl, (char*) "returning  %s", idStr);
	//return CORBA::string_dup(idStr);
	free(idStr);
	return server_orb->object_to_string(corbaObjectVector[index]);
}

// end_conversation() -- Implements IDL operation "AtmiBroker::ServiceFactory::end_conversation".
//
void AtmiBroker_ServiceFactoryImpl::end_conversation(CORBA::Long client_id, const char* id) throw (CORBA::SystemException ) {
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceFactoryImpl, (char*) "end_conversation(): %s", id);

	int index = atol(id);

	if (servantVector[index]->isInUse()) {
		userlog(Level::getDebug(), loggerAtmiBroker_ServiceFactoryImpl, (char*) "conversation ended for %s", id);
		servantVector[index]->setInUse(false);
	} else
		userlog(Level::getDebug(), loggerAtmiBroker_ServiceFactoryImpl, (char*) "conversation ALREADY ended for %s", id);

	userlog(Level::getDebug(), loggerAtmiBroker_ServiceFactoryImpl, (char*) "end_conversation(): returning.");
}

// get_service_info() -- Implements IDL operation "AtmiBroker::ServiceFactory::get_service_info".
//
AtmiBroker::ServiceInfo*
AtmiBroker_ServiceFactoryImpl::get_service_info() throw (CORBA::SystemException ) {
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceFactoryImpl, (char*) "get_service_info()");

	AtmiBroker::ServiceInfo_var aServiceInfo = new AtmiBroker::ServiceInfo();

	aServiceInfo->serviceName = CORBA::string_dup(serviceName);
	aServiceInfo->maxSize = serviceInfo.maxSize;
	aServiceInfo->minSize = serviceInfo.minSize;
	aServiceInfo->minAvailableSize = serviceInfo.minAvailableSize;
	aServiceInfo->logLevel = serviceInfo.logLevel;
	aServiceInfo->securityType = CORBA::string_dup(serviceInfo.securityType);

	serviceInfo.inUse = 0;
	int total = servantVector.size();
	for (int index = 0; index < total; index++) {
		if (servantVector[index] != NULL && servantVector[index]->isInUse())
			serviceInfo.inUse++;
	}
	aServiceInfo->inUse = serviceInfo.inUse;

	serviceInfo.available = total - serviceInfo.inUse;
	aServiceInfo->available = serviceInfo.available;

	return aServiceInfo._retn();
}

void AtmiBroker_ServiceFactoryImpl::createPoa() {
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceFactoryImpl, (char*) "createPoa() ");

	if (CORBA::is_nil(servicePoaPtr))
		servicePoaPtr = serverPoaFactory->createServicePoa(server_orb, servicePoaName, factoryPoaPtr, server_root_poa_manager);
}

void createPoa() {
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceFactoryImpl, (char*) "createPoa() ");
}

void AtmiBroker_ServiceFactoryImpl::createReference(PortableServer::ObjectId& anId, AtmiBroker::Service_var* refPtr) {
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceFactoryImpl, (char*) "createReference() ");

	//TODO SHOULD GET FROM SERVICE_FACTORY
	CORBA::Object_ptr tmp_ref = servicePoaPtr->create_reference_with_id(anId, "IDL:AtmiBroker/ServiceFactory:1.0");
	*refPtr = AtmiBroker::Service::_narrow(tmp_ref);
	userlog(Level::getInfo(), loggerAtmiBroker_ServiceFactoryImpl, (char*) "createReference() created  AtmiBroker::Service %p", (void*) *refPtr);
}

AtmiBroker_ServiceImpl *
AtmiBroker_ServiceFactoryImpl::createServant(int aIndex, void(*func)(TPSVCINFO *)) {
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceFactoryImpl, (char*) "createServant %d", aIndex);

	userlog(Level::getDebug(), loggerAtmiBroker_ServiceFactoryImpl, (char*) "createServant constucting AtmiBroker_ServiceImpl");
	AtmiBroker_ServiceImpl * tmp_servant = new AtmiBroker_ServiceImpl(this, servicePoaPtr, aIndex, serviceName, func);
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceFactoryImpl, (char*) "createServant constucted AtmiBroker_ServiceImpl");

	return tmp_servant;
}

int AtmiBroker_ServiceFactoryImpl::getMaxObjects() {
	return serviceInfo.maxSize;
}

void AtmiBroker_ServiceFactoryImpl::getDescriptorData() {
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceFactoryImpl, (char*) "getDescriptorData() ");

	serviceInfo.maxSize = MAX_SERVICE_CACHE_SIZE;
	serviceInfo.minSize = MIN_SERVICE_CACHE_SIZE;
	serviceInfo.minAvailableSize = MIN_AVAILABLE_SERVICE_CACHE_SIZE;

	AtmiBrokerServiceXml aAtmiBrokerServiceXml;
	aAtmiBrokerServiceXml.parseXmlDescriptor(&serviceInfo, descriptorFileName);
}

