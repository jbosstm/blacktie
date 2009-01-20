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
#include "OrbManagement.h"
#include "AtmiBrokerEnv.h"
#include "AtmiBrokerEnvXml.h"

#include <string.h>
#include "log4cxx/logger.h"
using namespace log4cxx;
using namespace log4cxx::helpers;
LoggerPtr loggerOrbManagement(Logger::getLogger("OrbManagment"));

void initOrb(char* name, Worker*& worker, CORBA::ORB_ptr& orbRef) {
	LOG4CXX_DEBUG(loggerOrbManagement, (char*) "initOrb");

	if (CORBA::is_nil(orbRef)) {
		LOG4CXX_DEBUG(loggerOrbManagement, (char*) "initOrb initing ORB ");

		std::string values = AtmiBrokerEnv::get_instance()->getenv((char*) "ORBOPT");
		char * cstr, *p;
		cstr = new char[values.size() + 1];
		strcpy(cstr, values.c_str());
		char** vals = new char*[values.size()];
		p = strtok(cstr, " ");
		int i = 1;
		while (p != NULL) {
			vals[i] = p;
			p = strtok(NULL, " ");
			i++;
		}

		orbRef = CORBA::ORB_init(i, vals, name);

		LOG4CXX_DEBUG(loggerOrbManagement, (char*) "initOrb inited ORB %p ");
		worker = new Worker(orbRef);
		if (worker->activate(THR_NEW_LWP| THR_JOINABLE, 1, 0, ACE_DEFAULT_THREAD_PRIORITY, -1, 0, 0, 0, 0, 0, 0) != 0) {
			delete worker;
			worker = NULL;
			LOG4CXX_ERROR(loggerOrbManagement, (char*) "Could not start thread pool");
		}

	} else {
		LOG4CXX_ERROR(loggerOrbManagement, (char*) "initOrb already inited ORB");
	}
}

void shutdownBindings(CORBA::ORB_ptr& orbRef, PortableServer::POA_var& poa, PortableServer::POAManager_var& poa_manager, CosNaming::NamingContextExt_var& ctx, CosNaming::NamingContext_var& nameCtx, PortableServer::POA_var& innerPoa, Worker* worker) {
	LOG4CXX_INFO(loggerOrbManagement, "Closing Bindings");

	if (!CORBA::is_nil(orbRef)) {
		LOG4CXX_DEBUG(loggerOrbManagement, "shutdownBindings shutting down ORB ");
		try {
			orbRef->shutdown(1);
			LOG4CXX_DEBUG(loggerOrbManagement, "shutdownBindings shut down ORB ");
		} catch (CORBA::Exception &ex) {
			LOG4CXX_ERROR(loggerOrbManagement, (char*) "shutdownBindings Unexpected CORBA exception shutting down orb: " << ex._name());
		} catch (...) {
			LOG4CXX_FATAL(loggerOrbManagement, (char*) "shutdownBindings Unexpected fatal exception");
		}

		try {
			// TODO DOES NOT WORK WHEN NO ORB WORK DONE
			LOG4CXX_DEBUG(loggerOrbManagement, (char*) "shutdownBindings destroying ORB ");
			orbRef->destroy();
		} catch (CORBA::Exception &ex) {
			LOG4CXX_ERROR(loggerOrbManagement, (char*) "shutdownBindings Unexpected CORBA exception destroying orb: " << ex._name());
		}
	}

	if (worker != NULL) {
		worker->wait();
		delete worker;
		worker = NULL;
	}

	if (innerPoa) {
//		delete innerPoa;
		innerPoa = NULL;
	}
	if (ctx) {
//		delete ctx;
		ctx = NULL;
	}
	if (nameCtx) {
//		delete nameCtx;
		nameCtx = NULL;
	}
	if (poa_manager) {
//		delete poa_manager;
		poa_manager = NULL;
	}
	if (poa) {
//		delete poa;
		poa = NULL;
	}
	if (orbRef) {
//		delete orbRef;
		orbRef = NULL;
	}
	LOG4CXX_INFO(loggerOrbManagement, (char*) "Closed Bindings");
}

void getNamingServiceAndContext(CORBA::ORB_ptr& orbRef, CosNaming::NamingContextExt_var& default_ctx, CosNaming::NamingContext_var& name_ctx) {
	LOG4CXX_DEBUG(loggerOrbManagement, (char*) "getNamingServiceAndContext");

	CORBA::Object_var tmp_ref;

	if (CORBA::is_nil(default_ctx)) {
		LOG4CXX_DEBUG(loggerOrbManagement, (char*) "getNamingServiceAndContext getting Naming Service Ext ");
		tmp_ref = orbRef->resolve_initial_references("NameService");
		LOG4CXX_DEBUG(loggerOrbManagement, (char*) "getNamingServiceAndContext got orbRef->resolve_initial_references, tmp_ref = %p" << (void*) tmp_ref);
		default_ctx = CosNaming::NamingContextExt::_narrow(tmp_ref);
		LOG4CXX_DEBUG(loggerOrbManagement, (char*) "getNamingServiceAndContext narrowed tmp_ref, default_context =  %p" << (void*) default_ctx);
		//assert(!CORBA::is_nil(default_ctx));
		LOG4CXX_DEBUG(loggerOrbManagement, (char*) "getNamingServiceAndContext got Naming Service Ext ");
	} else
	LOG4CXX_ERROR(loggerOrbManagement, (char*) "getNamingServiceAndContext already got Naming Service Ext  ");

	if (CORBA::is_nil(name_ctx)) {
		LOG4CXX_DEBUG(loggerOrbManagement, (char*) "getNamingServiceAndContext getting Naming Service Instance  ");

		LOG4CXX_DEBUG(loggerOrbManagement, (char*) "getNamingServiceAndContext company is  %s" << company);

		CosNaming::Name_var name;
		try {
			tmp_ref = default_ctx->resolve_str(company);
			name_ctx = CosNaming::NamingContext::_narrow(tmp_ref);
			LOG4CXX_DEBUG(loggerOrbManagement, (char*) "getNamingServiceAndContext found company naming context");
		} catch (const CosNaming::NamingContext::NotFound&) {
			try {
				name = default_ctx->to_name(company);
				name_ctx = default_ctx->bind_new_context(name);
				LOG4CXX_DEBUG(loggerOrbManagement, (char*) "getNamingServiceAndContext created company naming context");
			} catch (const CosNaming::NamingContext::AlreadyBound&) {
				name_ctx = default_ctx->new_context();
				default_ctx->rebind_context(name, name_ctx.in());
				LOG4CXX_DEBUG(loggerOrbManagement, (char*) "getNamingServiceAndContext re-created company naming context");
			}
		}
		//assert(!CORBA::is_nil(name_ctx));
		LOG4CXX_DEBUG(loggerOrbManagement, (char*) "getNamingServiceAndContext got Naming Service Instance  ");
	} else
	LOG4CXX_ERROR(loggerOrbManagement, (char*) "getNamingServiceAndContext already got Naming Service Instance  ");
}

void getRootPOAAndManager(CORBA::ORB_ptr& orbRef, PortableServer::POA_var& poa, PortableServer::POAManager_var& poa_manager) {
	LOG4CXX_DEBUG(loggerOrbManagement, (char*) "getRootPOAAndManager");

	CORBA::Object_var tmp_ref;

	if (CORBA::is_nil(poa)) {
		LOG4CXX_DEBUG(loggerOrbManagement, (char*) "getRootPOAAndManager resolving the root POA ");
		tmp_ref = orbRef->resolve_initial_references("RootPOA");
		poa = PortableServer::POA::_narrow(tmp_ref);
		LOG4CXX_DEBUG(loggerOrbManagement, (char*) "getRootPOAAndManager resolved the root POA: " << (void*) poa);
	} else {
		LOG4CXX_DEBUG(loggerOrbManagement, (char*) "getRootPOAAndManager already resolved the root POA: " << (void*) poa);
	}

	if (CORBA::is_nil(poa_manager)) {
		LOG4CXX_DEBUG(loggerOrbManagement, (char*) "getRootPOAAndManager getting the root POA manager");
		poa_manager = poa->the_POAManager();
		//assert(!CORBA::is_nil(poa_manager));
		LOG4CXX_DEBUG(loggerOrbManagement, (char*) "getRootPOAAndManager got the root POA manager: " << (void*) poa_manager);
	} else {
		LOG4CXX_DEBUG(loggerOrbManagement, (char*) "getRootPOAAndManager already resolved the root POA manager: " << (void*) poa_manager);
	}
}

