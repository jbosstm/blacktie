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

#ifdef TAO_COMP
#include <orbsvcs/CosNamingS.h>
#elif ORBIX_COMP
#include <omg/CosNaming.hh>
#endif
#ifdef VBC_COMP
#include <CosNaming_c.hh>
#endif

#include "AtmiBrokerOTS.h"
#include "LocalResourceManager.h"

#include "log4cxx/logger.h"


log4cxx::LoggerPtr loggerLocalResourceManager(log4cxx::Logger::getLogger("LocalResourceManager"));

LocalResourceManager::LocalResourceManager(ResourceManagerDataStruct& aResourceManagerDataStruct) {
	LOG4CXX_LOGLS(loggerLocalResourceManager, log4cxx::Level::getInfo(), (char*) "constructor");
	m_resourceManagerDataStruct = aResourceManagerDataStruct;

	// GET REFERENCE TO REMOTE RESOURCE MANAGER FACTORY FROM NAMING SERVICE
	// TODO THIS IS ASSUMING THAT IT IS THE CLIENT?
	CosNaming::Name * name = AtmiBrokerOTS::get_instance()->getNamingContextExt()->to_name("AtmiBrokerXA_ResourceManagerFactory");
	CORBA::Object_var tmp_ref = AtmiBrokerOTS::get_instance()->getNamingContext()->resolve(*name);
	if (CORBA::is_nil(tmp_ref)) {
		LOG4CXX_LOGLS(loggerLocalResourceManager, log4cxx::Level::getError(), (char*) "Could not find AtmiBrokerXA_ResourceManagerFactory");
	} else {
		LOG4CXX_LOGLS(loggerLocalResourceManager, log4cxx::Level::getInfo(), (char*) "tmp_ref is " << (void *) tmp_ref);
		m_resourceManagerFactory = AtmiBrokerXA::ResourceManagerFactory::_narrow(tmp_ref);
		if (CORBA::is_nil(m_resourceManagerFactory)) {
			LOG4CXX_LOGLS(loggerLocalResourceManager, log4cxx::Level::getError(), (char*) "Could not narrow reference to interface AtmiBrokerXA::ResourceManagerFactory");
		} else {
			LOG4CXX_LOGLS(loggerLocalResourceManager, log4cxx::Level::getInfo(), (char*) "m_resourceManagerFactory is " << (void *) m_resourceManagerFactory);
			// GET REFERENCE TO REMOTE RESOURCE MANAGER FROM FACTORY
			char * aResourceManagerIOR = m_resourceManagerFactory->create_resource_manager_ior(m_resourceManagerDataStruct.resource_manager_name.c_str(), m_resourceManagerDataStruct.open_string.c_str(), m_resourceManagerDataStruct.close_string.c_str(), m_resourceManagerDataStruct.thread_model, m_resourceManagerDataStruct.automatic_association,
					m_resourceManagerDataStruct.dynamic_registration_optimization);

			LOG4CXX_LOGLS(loggerLocalResourceManager, log4cxx::Level::getInfo(), (char*) "Remote ResourceManager is " << aResourceManagerIOR);
			tmp_ref = AtmiBrokerOTS::get_instance()->getOrb()->string_to_object(aResourceManagerIOR);
			LOG4CXX_LOGLS(loggerLocalResourceManager, log4cxx::Level::getInfo(), (char*) "Remote ResourceManager is " << (void *) tmp_ref);
			m_resourceManagerDataStruct.resourceManager = XA::ResourceManager::_narrow(tmp_ref);
			LOG4CXX_LOGLS(loggerLocalResourceManager, log4cxx::Level::getInfo(), (char*) "Narrowed Remote ResourceManager is " << (void *) m_resourceManagerDataStruct.resourceManager);
		}
	}
}

LocalResourceManager::~LocalResourceManager() {
	LOG4CXX_LOGLS(loggerLocalResourceManager, log4cxx::Level::getInfo(), (char*) "destructor");
	CORBA::release(m_resourceManagerDataStruct.resourceManager);
}

CORBA::ULong LocalResourceManager::register_before_completion_callback(XA::BeforeCompletionCallback_ptr bcc) throw(CORBA::SystemException) {
	LOG4CXX_LOGLS(loggerLocalResourceManager, log4cxx::Level::getInfo(), (char*) "register_before_completion_callback");

	return m_resourceManagerDataStruct.resourceManager->register_before_completion_callback(bcc);
}

void LocalResourceManager::unregister_before_completion_callback(unsigned int key) throw(CORBA::SystemException) {
	LOG4CXX_LOGLS(loggerLocalResourceManager, log4cxx::Level::getInfo(), (char*) "unregister_before_completion_callback");

	m_resourceManagerDataStruct.resourceManager->unregister_before_completion_callback(key);
}

ResourceManagerDataStruct&
LocalResourceManager::getResourceManagerDataStruct() {
	LOG4CXX_LOGLS(loggerLocalResourceManager, log4cxx::Level::getInfo(), (char*) "getResourceManagerDataStruct");

	return m_resourceManagerDataStruct;
}
