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

#include "AtmiBrokerOTS.h"
#include "ResourceManagerCache.h"

#include "log4cxx/logger.h"
using namespace log4cxx;
using namespace log4cxx::helpers;
LoggerPtr loggerResourceManagerCache(Logger::getLogger("ResourceManagerCache"));

static ResourceManagerCache * localResourceManagerCache = NULL;

ResourceManagerCache::ResourceManagerCache() {
	LOG4CXX_LOGLS(loggerResourceManagerCache, Level::getInfo(), (char*) "constructor");
}

ResourceManagerCache::~ResourceManagerCache() {
	LOG4CXX_LOGLS(loggerResourceManagerCache, Level::getInfo(), (char*) "destructor");
}

ResourceManagerCache *
ResourceManagerCache::getResourceManagerCache() {
	if (localResourceManagerCache == (ResourceManagerCache *) NULL) {
		localResourceManagerCache = new ResourceManagerCache();
	}
	return localResourceManagerCache;
}

XA::ResourceManager_ptr ResourceManagerCache::create_resource_manager(const std::string& resource_manager_name, const std::string& open_string, const std::string& close_string, XA::ThreadModel thread_model, CORBA::Boolean automatic_association, CORBA::Boolean dynamic_registration_optimization, const std::string& library_name,
		const std::string& xa_symbol_name) {
	LOG4CXX_LOGLS(loggerResourceManagerCache, Level::getInfo(), (char*) "create_resource_manager ENTERED");

	XA::ResourceManager_ptr aResourceManager = find_resource_manager(resource_manager_name, open_string, close_string, thread_model, automatic_association, dynamic_registration_optimization);

	if (aResourceManager == (XA::ResourceManager*) NULL) {
		ResourceManagerDataStruct* aResourceManagerDataStruct = new ResourceManagerDataStruct();
		aResourceManagerDataStruct->resource_manager_name = resource_manager_name;
		aResourceManagerDataStruct->open_string = open_string;
		aResourceManagerDataStruct->close_string = close_string;
		aResourceManagerDataStruct->thread_model = thread_model;
		aResourceManagerDataStruct->automatic_association = automatic_association;
		aResourceManagerDataStruct->dynamic_registration_optimization = dynamic_registration_optimization;
		/**
		 // CREATE XA SWITCH
		 xa_switch_t* xaswitch;
		 ACE_SHLIB_HANDLE handle = ACE_OS::dlopen (library_name.c_str(), ACE_DEFAULT_SHLIB_MODE);
		 if (!handle)
		 LOG4CXX_LOGLS(loggerResourceManagerCache, Level::getError(), (char*)"Error Opening DB Libary %s",  library_name.c_str());

		 xaswitch = (xa_switch_t*) ACE_OS::dlsym(handle, xa_symbol_name.c_str());

		 char *error  = ACE_OS::dlerror();
		 if (error != NULL)
		 {
		 ACE_OS::dlclose(handle);
		 LOG4CXX_LOGLS(loggerResourceManagerCache, Level::getError(), (char*)"Error Finding XA Symbol %s",  xa_symbol_name.c_str());
		 }
		 aResourceManagerDataStruct->xaSwitch						= *xaswitch;
		 ***/
		aResourceManagerDataStruct->rmid = atol(resource_manager_name.c_str());

		aResourceManagerDataStruct->resourceManager = AtmiBrokerOTS::get_instance()->getXaConnector()->create_resource_manager(resource_manager_name.c_str(),
		//											*xaswitch,
				AtmiBrokerOTS::get_instance()->getXaosw(), open_string.c_str(), close_string.c_str(), thread_model, automatic_association, dynamic_registration_optimization, AtmiBrokerOTS::get_instance()->getXaCurrentConnection());

		resourceManagerQueue.push_back(aResourceManagerDataStruct);
	}
	return aResourceManager;
}

XA::ResourceManager_ptr ResourceManagerCache::find_resource_manager(const std::string& resource_manager_name, const std::string& open_string, const std::string& close_string, XA::ThreadModel thread_model, CORBA::Boolean automatic_association, CORBA::Boolean dynamic_registration_optimization) {
	LOG4CXX_LOGLS(loggerResourceManagerCache, Level::getInfo(), (char*) "find_resource_manager ENTERED");

	for (std::deque<ResourceManagerDataStruct*>::iterator itResourceManagerDataStruct = resourceManagerQueue.begin(); itResourceManagerDataStruct != resourceManagerQueue.end(); itResourceManagerDataStruct++) {
		LOG4CXX_LOGLS(loggerResourceManagerCache, Level::getInfo(), (char*) "next " << (*itResourceManagerDataStruct)->resource_manager_name.c_str());
		if ((*itResourceManagerDataStruct)->resource_manager_name.compare(resource_manager_name)) {
			LOG4CXX_LOGLS(loggerResourceManagerCache, Level::getInfo(), (char*) " found matching local resource manager " << resource_manager_name.c_str());
			return (*itResourceManagerDataStruct)->resourceManager;
		}
	}
	// TODO IS THIS COPIED FROM CONNECTOR
	LOG4CXX_LOGLS(loggerResourceManagerCache, Level::getInfo(), (char*) "find_resource_manager FINISHED ");

	return NULL;
}

