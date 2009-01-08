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

#include "LocalResourceManagerCache.h"

#include "log4cxx/logger.h"
using namespace log4cxx;
using namespace log4cxx::helpers;
LoggerPtr loggerLocalResourceManagerCache(Logger::getLogger("LocalResourceManagerCache"));

#ifdef TAO_COMP
#include "ace/OS.h"
#endif

static LocalResourceManagerCache * localResourceManagerCache = NULL;

LocalResourceManagerCache::LocalResourceManagerCache() {
	LOG4CXX_LOGLS(loggerLocalResourceManagerCache, Level::getDebug(), (char*) "constructor");
}

LocalResourceManagerCache::~LocalResourceManagerCache() {
	LOG4CXX_LOGLS(loggerLocalResourceManagerCache, Level::getDebug(), (char*) "destructor");
	for (std::deque<LocalResourceManager*>::iterator itLocalResourceManager = localResourceManagerQueue.begin(); itLocalResourceManager != localResourceManagerQueue.end(); itLocalResourceManager++) {
		LOG4CXX_LOGLS(loggerLocalResourceManagerCache, Level::getDebug(), (char*) "deleting local resource manager " << (*itLocalResourceManager)->getResourceManagerDataStruct().resource_manager_name.c_str());
		delete (*itLocalResourceManager);
	}
}

LocalResourceManagerCache *
LocalResourceManagerCache::getLocalResourceManagerCache() {
	if (localResourceManagerCache == (LocalResourceManagerCache *) NULL) {
		localResourceManagerCache = new LocalResourceManagerCache();
	}
	return localResourceManagerCache;
}

void LocalResourceManagerCache::discardLocalResourceManagerCache() {
	if (localResourceManagerCache != NULL) {
		delete localResourceManagerCache;
		localResourceManagerCache = NULL;
	}
}

LocalResourceManager *
LocalResourceManagerCache::create_local_resource_manager(const std::string& resource_manager_name, const std::string& open_string, const std::string& close_string, XA::ThreadModel thread_model, CORBA::Boolean automatic_association, CORBA::Boolean dynamic_registration_optimization, const std::string& library_name, const std::string& xa_symbol_name) {
	LOG4CXX_LOGLS(loggerLocalResourceManagerCache, Level::getInfo(), (char*) "create_local_resource_manager ENTERED");

	LocalResourceManager* aLocalResourceManager = find_local_resource_manager(resource_manager_name, open_string, close_string, thread_model, automatic_association, dynamic_registration_optimization);

	if (aLocalResourceManager == (LocalResourceManager*) NULL) {
		ResourceManagerDataStruct* aResourceManagerDataStruct = new ResourceManagerDataStruct();
		aResourceManagerDataStruct->resource_manager_name = resource_manager_name;
		aResourceManagerDataStruct->open_string = open_string;
		aResourceManagerDataStruct->close_string = close_string;
		aResourceManagerDataStruct->thread_model = thread_model;
		aResourceManagerDataStruct->automatic_association = automatic_association;
		aResourceManagerDataStruct->dynamic_registration_optimization = dynamic_registration_optimization;

		// CREATE XA SWITCH TEMP
		/* TEMP
		 xa_switch_t* xaswitch;
		 ACE_SHLIB_HANDLE handle = ACE_OS::dlopen (library_name.c_str(), ACE_DEFAULT_SHLIB_MODE);
		 if (!handle)
		 LOG4CXX_LOGLS(loggerLocalResourceManagerCache, Level::getError(), (char*)"Error Opening DB Libary %s",  library_name.c_str());

		 xaswitch = (xa_switch_t*) ACE_OS::dlsym(handle, xa_symbol_name.c_str());

		 char *error  = ACE_OS::dlerror();
		 if (error != NULL)
		 {
		 ACE_OS::dlclose(handle);
		 LOG4CXX_LOGLS(loggerLocalResourceManagerCache, Level::getError(), (char*)"Error Finding XA Symbol %s",  xa_symbol_name.c_str());
		 }
		 aResourceManagerDataStruct->xaSwitch							= *xaswitch;
		 */
		aResourceManagerDataStruct->rmid = atol(resource_manager_name.c_str());
		aLocalResourceManager = new LocalResourceManager(*aResourceManagerDataStruct);

		localResourceManagerQueue.push_back(aLocalResourceManager);
	}
	return aLocalResourceManager;
}

LocalResourceManager *
LocalResourceManagerCache::find_local_resource_manager(const std::string& resource_manager_name, const std::string& open_string, const std::string& close_string, XA::ThreadModel thread_model, CORBA::Boolean automatic_association, CORBA::Boolean dynamic_registration_optimization) {
	LOG4CXX_LOGLS(loggerLocalResourceManagerCache, Level::getInfo(), (char*) "find_local_resource_manager ENTERED");

	for (std::deque<LocalResourceManager*>::iterator itLocalResourceManager = localResourceManagerQueue.begin(); itLocalResourceManager != localResourceManagerQueue.end(); itLocalResourceManager++) {
		LOG4CXX_LOGLS(loggerLocalResourceManagerCache, Level::getInfo(), (char*) "next " << (*itLocalResourceManager)->getResourceManagerDataStruct().resource_manager_name.c_str());
		if ((*itLocalResourceManager)->getResourceManagerDataStruct().resource_manager_name.compare(resource_manager_name)) {
			LOG4CXX_LOGLS(loggerLocalResourceManagerCache, Level::getInfo(), (char*) " found matching local resource manager " << resource_manager_name.c_str());
			return (*itLocalResourceManager);
		}
	}
	// TODO IS THIS COPIED FROM CONNECTOR
	LOG4CXX_LOGLS(loggerLocalResourceManagerCache, Level::getInfo(), (char*) "find_local_resource_manager FINISHED ");

	return NULL;
}

