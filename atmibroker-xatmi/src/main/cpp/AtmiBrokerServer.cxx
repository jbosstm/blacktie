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
#include <string>
#include <sstream>
#include <queue>

#ifdef TAO_COMP
#include <orbsvcs/CosNamingS.h>
#endif

#include "log4cxx/logger.h"
#include "AtmiBrokerInit.h"
#include "AtmiBrokerServer.h"
#include "AtmiBrokerPoaFac.h"
#include "AtmiBrokerEnv.h"
#include "AtmiBrokerServerControl.h"
#include "AtmiBrokerMem.h"
#include "txx.h"
#include "OrbManagement.h"
#include "SymbolLoader.h"
#include "ace/Get_Opt.h"
#include "ace/OS_NS_stdio.h"
#include "ace/OS_NS_stdlib.h"
#include "ace/OS_NS_string.h"
#include "ace/Default_Constants.h"
#include "ace/Signal.h"
#include "ThreadLocalStorage.h"
#include "xatmi.h"

// WORK AROUND FOR NO tx.h
#define TX_OK              0
#ifdef __cplusplus
extern "C" {
#endif
extern BLACKTIE_TX_DLL int tx_open(void);
#ifdef __cplusplus
}
#endif

extern void ADMIN(TPSVCINFO* svcinfo);
extern const char* version;

log4cxx::LoggerPtr loggerAtmiBrokerServer(log4cxx::Logger::getLogger(
		"AtmiBrokerServer"));
AtmiBrokerServer * ptrServer = NULL;
bool serverInitialized = false;
PortableServer::POA_var server_poa;
bool configFromCmdline = false;
int errorBootAdminService = 0;
char configDir[256];
char server[30];
int serverid = -1;

int server_sigint_handler_callback(int sig_type) {
	LOG4CXX_INFO(
			loggerAtmiBrokerServer,
			(char*) "SIGINT Detected: Shutting down server this may take several minutes");
	if (ptrServer != NULL)
		ptrServer->shutdown();
	LOG4CXX_INFO(loggerAtmiBrokerServer,
			(char*) "SIGINT Detected: Shutdown complete");
	return -1;

}

int serverrun() {
	setSpecific(TPE_KEY, TSS_TPERESET);
	return ptrServer->block();
}

void parsecmdline(int argc, char** argv) {
	ACE_Get_Opt getopt(argc, argv, ACE_TEXT("c:i:s:"));
	int c;

	configFromCmdline = false;
	while ((c = getopt()) != -1) {
		switch ((char) c) {
		case 'c':
			configFromCmdline = true;
			ACE_OS::strncpy(configDir, getopt.opt_arg(), 256);
			break;
		case 'i':
			serverid = atoi(getopt.opt_arg());
			if (serverid <= 0 || serverid > 9) {
				serverid = -1;
			}
			break;
		case 's':
			ACE_OS::strncpy(server, getopt.opt_arg(), 30);
			break;
		}
	}
}

const char* getConfiguration() {
	const char* dir = NULL;

	if (configFromCmdline) {
		dir = configDir;
	}

	return dir;
}

int serverinit(int argc, char** argv) {
	AtmiBrokerInitSingleton::instance();
	setSpecific(TPE_KEY, TSS_TPERESET);
	int toReturn = 0;

	if (ptrServer == NULL) {
		LOG4CXX_DEBUG(loggerAtmiBrokerServer,
				(char*) "serverinit getting config");
		// Check the configuration
		const char* serverName = ACE_OS::getenv("BLACKTIE_SERVER");
		if (serverName != NULL) {
			ACE_OS::strncpy(server, serverName, 30);
		} else {
			ACE_OS::strncpy(server, "default", 30);
		}
		const char* serverId = ACE_OS::getenv("BLACKTIE_SERVER_ID");
		if (serverId != NULL) {
			serverid = atoi(serverId);
		} else {
			serverid = -1;
		}
		if (argc > 0) {
			parsecmdline(argc, argv);
		}
		if (serverid == -1) {
			fprintf(stderr,
					"you must specify a server id with -i greater than 0 and less than 10\n");
			fprintf(stderr,
					"example usage: ./server [-c config] -i id [-s server]\n");
			toReturn = -1;
			setSpecific(TPE_KEY, TSS_TPESYSTEM);
		}

		if (toReturn != -1) {
			LOG4CXX_DEBUG(loggerAtmiBrokerServer,
					(char*) "serverinit beginning configuration");
			const char* configuration = getConfiguration();
			if (configuration != NULL) {
				AtmiBrokerEnv::set_configuration(configuration);
				LOG4CXX_DEBUG(loggerAtmiBrokerServer,
						(char*) "set AtmiBrokerEnv configuration type "
								<< configuration);
			}

			try {
				AtmiBrokerEnv* env = AtmiBrokerEnv::get_instance();
				std::stringstream sname;
				std::stringstream sid;
				sname << "BLACKTIE_SERVER_NAME=" << domain << server
						<< serverid;
				sid << "BLACKTIE_SERVER_ID=" << serverid;
				env->putenv((char *) (sname.str().c_str()));
				env->putenv((char *) (sid.str().c_str()));

				LOG4CXX_DEBUG(loggerAtmiBrokerServer,
						(char*) "serverinit called");
				ptrServer = new AtmiBrokerServer();
				LOG4CXX_DEBUG(loggerAtmiBrokerServer,
						(char*) "serverInitialized=" << serverInitialized);

				if (!serverInitialized) {
					::serverdone();
					toReturn = -1;
					setSpecific(TPE_KEY, TSS_TPESYSTEM);
				} else {
					ptrServer->advertiseAtBootime();

					// install a handler for the default set of signals (namely, SIGINT and SIGTERM)
					(env->getSignalHandler()).addSignalHandler(
							server_sigint_handler_callback, true);

					LOG4CXX_INFO(loggerAtmiBrokerServer, (char*) "Server "
							<< serverid << " Running");
				}
			} catch (...) {
				LOG4CXX_ERROR(loggerAtmiBrokerServer,
						(char*) "Server startup failed");
				toReturn = -1;
				setSpecific(TPE_KEY, TSS_TPESYSTEM);
			}
		}
	}
	if (toReturn != 0) {
		LOG4CXX_FATAL(loggerAtmiBrokerServer, (char*) "serverinit failed");
	}
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "serverinit returning: "
			<< toReturn);
	return toReturn;
}

int serverdone() {
	setSpecific(TPE_KEY, TSS_TPERESET);
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "serverdone called");
	if (ptrServer) {
		LOG4CXX_DEBUG(loggerAtmiBrokerServer,
				(char*) "serverdone deleting Corba server");
		delete ptrServer;
		ptrServer = NULL;
		LOG4CXX_DEBUG(loggerAtmiBrokerServer,
				(char*) "serverdone deleted Corba server");

	}
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "serverdone returning 0");
	//	if (configFromCmdline) {
	//		char* toFree = ACE_OS::getenv("BLACKTIE_CONFIGURATION");
	//		free(toFree);
	//	}
	return 0;
}

int isadvertised(char* name) {
	if (ptrServer) {
		if (ptrServer->isAdvertised(name)) {
			return 0;
		}
	}
	return -1;
}

int getServiceStatus(char** str, char* svc) {
	if (ptrServer) {
		return ptrServer->getServiceStatus(str, svc);
	}

	return -1;
}

long getServiceMessageCounter(char* serviceName) {
	if (ptrServer) {
		return ptrServer->getServiceMessageCounter(serviceName);
	}

	return 0;
}

long getServiceErrorCounter(char* serviceName) {
	if (ptrServer) {
		return ptrServer->getServiceErrorCounter(serviceName);
	}

	return 0;
}

void getResponseTime(char* serviceName, unsigned long* min, unsigned long* avg,
		unsigned long* max) {
	if (ptrServer) {
		ptrServer->getResponseTime(serviceName, min, avg, max);
	}
}

int advertiseByAdmin(char* name) {
	if (isadvertised(name) == 0) {
		return 0;
	}

	if (ptrServer) {
		if (ptrServer->advertiseService(name)) {
			return 0;
		}
	}
	return -1;
}

int pauseServerByAdmin() {
	if (ptrServer) {
		return ptrServer->pause();
	}

	return -1;
}

int resumeServerByAdmin() {
	if (ptrServer) {
		return ptrServer->resume();
	}

	return -1;
}

// AtmiBrokerServer constructor
//
// Note: since we use virtual inheritance, we must include an
// initialiser for all the virtual base class constructors that
// require arguments, even those that we inherit indirectly.
//
AtmiBrokerServer::AtmiBrokerServer() {
	try {
		finish = new SynchronizableObject();
		serverName = server;
		isPause = false;
		unsigned int i;

		serverInfo.serverName = NULL;

		for (i = 0; i < servers.size(); i++) {
			if (strcmp(servers[i]->serverName, serverName) == 0) {
				serverInfo.serverName = strdup(servers[i]->serverName);
				// add service ADMIN
				char adm[XATMI_SERVICE_NAME_LENGTH + 1];
				ACE_OS::snprintf(adm, XATMI_SERVICE_NAME_LENGTH + 1, ".%s%d",
						server, serverid);
				ServiceInfo service;
				memset(&service, 0, sizeof(ServiceInfo));
				service.serviceName = strdup(adm);
#ifdef WIN32
				service.transportLib = strdup("atmibroker-hybrid.dll");
#else
				service.transportLib = strdup("libatmibroker-hybrid.so");
#endif
				service.poolSize = 1;
				service.advertised = false;
				serverInfo.serviceVector.push_back(service);

				for (unsigned int j = 0; j < servers[i]->serviceVector.size(); j++) {
					ServiceInfo service;
					memset(&service, 0, sizeof(ServiceInfo));
					service.serviceName = strdup(
							servers[i]->serviceVector[j].serviceName);
					service.transportLib = strdup(
							servers[i]->serviceVector[j].transportLib);

					if (servers[i]->serviceVector[j].function_name) {
						service.function_name = strdup(
								servers[i]->serviceVector[j].function_name);
					} else {
						service.function_name = NULL;
					}
					if (servers[i]->serviceVector[j].library_name) {
						service.library_name = strdup(
								servers[i]->serviceVector[j].library_name);
					} else {
						service.library_name = NULL;
					}
					service.poolSize = servers[i]->serviceVector[j].poolSize;
					service.advertised
							= servers[i]->serviceVector[j].advertised;
					service.conversational
							= servers[i]->serviceVector[j].conversational;
					service.externally_managed_destination
							= servers[i]->serviceVector[j].externally_managed_destination;
					serverInfo.serviceVector.push_back(service);
				}
				break;
			}
		}

		if (i == servers.size()) {
			LOG4CXX_WARN(loggerAtmiBrokerServer, serverName
					<< " has no configuration");
			setSpecific(TPE_KEY, TSS_TPESYSTEM);
			return;
		}

		if (tx_open() != TX_OK) {
			setSpecific(TPE_KEY, TSS_TPESYSTEM);

			LOG4CXX_ERROR(
					loggerAtmiBrokerServer,
					serverName
							<< (char *) " transaction configuration error, aborting server startup");
		} else {
			// make ADMIN service mandatory for server
			char adm[XATMI_SERVICE_NAME_LENGTH + 1];
			ACE_OS::snprintf(adm, XATMI_SERVICE_NAME_LENGTH + 1, ".%s%d",
					server, serverid);
			if (!advertiseService(adm, ADMIN)) {
				LOG4CXX_FATAL(loggerAtmiBrokerServer,
						(char*) "advertise admin service failed");
				return;
			}

			if (errorBootAdminService == 2) {
				LOG4CXX_WARN(loggerAtmiBrokerServer,
						(char*) "Maybe the same server id running");
				throw std::exception();
			}
			serverInitialized = true;

			LOG4CXX_DEBUG(loggerAtmiBrokerServer,
					(char*) "server_init(): finished.");
		}
	} catch (CORBA::Exception& e) {
		LOG4CXX_ERROR(loggerAtmiBrokerServer,
				(char*) "serverinit - Unexpected CORBA exception: "
						<< e._name());
		setSpecific(TPE_KEY, TSS_TPESYSTEM);
	}
}

// ~AtmiBrokerServer destructor.
//
AtmiBrokerServer::~AtmiBrokerServer() {
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "destructor");
	server_done();
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "Server done");

	for (std::vector<ServiceDispatcher*>::iterator dispatcher =
			serviceDispatchersToDelete.begin(); dispatcher
			!= serviceDispatchersToDelete.end(); dispatcher++) {
		LOG4CXX_TRACE(loggerAtmiBrokerServer,
			(char*) "Waiting for dispatcher to be notified: " << (*dispatcher));
		(*dispatcher)->wait();
		LOG4CXX_TRACE(loggerAtmiBrokerServer, (char*) "deleting dispatcher: " << (*dispatcher));
		delete (*dispatcher);
		LOG4CXX_TRACE(loggerAtmiBrokerServer, (char*) "deleted dispatcher: " << (*dispatcher));
	}
	serviceDispatchersToDelete.clear();

	if (finish != NULL) {
		delete finish;
		finish = NULL;
	}

	serviceData.clear();
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "deleted service array");

	for (unsigned int i = 0; i < serverInfo.serviceVector.size(); i++) {
		ServiceInfo* service = &serverInfo.serviceVector[i];
		free(service->serviceName);
		free(service->transportLib);
		if (service->function_name != NULL) {
			free(service->function_name);
		}
		if (service->library_name != NULL) {
			free(service->library_name);
		}
	}
	if (serverInfo.serverName != NULL) {
		free(serverInfo.serverName);
	}

	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "deleting services");
	AtmiBrokerMem::discard_instance();
	txx_stop();
	AtmiBrokerEnv::discard_instance();
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "deleted services");

	connections.closeConnections();
	serverInitialized = false;
}

void AtmiBrokerServer::advertiseAtBootime() {
	for (unsigned int i = 0; i < serverInfo.serviceVector.size(); i++) {
		ServiceInfo* service = &serverInfo.serviceVector[i];
		SVCFUNC func = NULL;
		bool status = false;

		if (service->library_name != NULL) {
			func = (SVCFUNC) ::lookup_symbol(service->library_name,
					service->function_name);
			if (func == NULL) {
				LOG4CXX_WARN(loggerAtmiBrokerServer, "can not find "
						<< service->function_name << " in "
						<< service->library_name);
			}
		}

		if (service->advertised && func != NULL) {
			LOG4CXX_DEBUG(loggerAtmiBrokerServer, "begin advertise "
					<< service->serviceName);
			status = advertiseService((char*) service->serviceName, func);
			LOG4CXX_DEBUG(loggerAtmiBrokerServer, "end advertise "
					<< service->serviceName);
		}

		updateServiceStatus(service, func, status);
	}
}

int AtmiBrokerServer::block() {

	int toReturn = 0;
	if (errorBootAdminService == 3) {
		LOG4CXX_INFO(loggerAtmiBrokerServer, "Domain is paused");
		pause();
	} else {
		LOG4CXX_INFO(loggerAtmiBrokerServer, "Server waiting for requests...");
	}

	try {
		finish->lock();
		finish->wait(0);
		finish->unlock();
	} catch (...) {
		LOG4CXX_ERROR(loggerAtmiBrokerServer, "Unexpected exception");
		toReturn = -1;
	}
	return toReturn;
}

void AtmiBrokerServer::shutdown() {
	LOG4CXX_INFO(loggerAtmiBrokerServer, "Server prepare to shutdown");
	//	server_done(); You can't do this here as the service dispatcher will be cleaned up that is handling
	// the cleanup for an admin call
	finish->lock();
	finish->notify();
	finish->unlock();
}

int AtmiBrokerServer::pause() {
	if (!isPause) {
		char adm[XATMI_SERVICE_NAME_LENGTH + 1];
		ACE_OS::snprintf(adm, XATMI_SERVICE_NAME_LENGTH + 1, ".%s%d", server,
				serverid);
		for (std::vector<ServiceData>::iterator i = serviceData.begin(); i
				!= serviceData.end(); i++) {
			if (ACE_OS::strcmp((*i).serviceInfo->serviceName, adm) != 0) {
				LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "pausing service"
						<< (*i).serviceInfo->serviceName);
				for (std::vector<ServiceDispatcher*>::iterator j =
						(*i).dispatchers.begin(); j != (*i).dispatchers.end(); j++) {
					ServiceDispatcher* dispatcher = (*j);
					if (dispatcher->pause() != 0) {
						LOG4CXX_WARN(loggerAtmiBrokerServer,
								(char*) "pause service dispatcher "
										<< dispatcher << " failed");
					}
				}
			}
			LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "pause service"
					<< (*i).serviceInfo->serviceName << " done");
		}
		isPause = true;
		LOG4CXX_INFO(loggerAtmiBrokerServer, (char*) "Server Pause");
	}
	return 0;
}

int AtmiBrokerServer::resume() {
	if (isPause) {
		for (std::vector<ServiceData>::iterator i = serviceData.begin(); i
				!= serviceData.end(); i++) {
			LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "resuming service"
					<< (*i).serviceInfo->serviceName);
			for (std::vector<ServiceDispatcher*>::iterator j =
					(*i).dispatchers.begin(); j != (*i).dispatchers.end(); j++) {
				ServiceDispatcher* dispatcher = (*j);
				if (dispatcher->resume() != 0) {
					LOG4CXX_WARN(loggerAtmiBrokerServer,
							(char*) "resume service dispatcher " << dispatcher
									<< " failed");
				}
			}
			LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "resume service"
					<< (*i).serviceInfo->serviceName << " done");
		}
		isPause = false;
		LOG4CXX_INFO(loggerAtmiBrokerServer, (char*) "Server Resume");
	}
	return 0;
}

char *
AtmiBrokerServer::getServerName() {
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "getServerName");
	return serverName;
}

int AtmiBrokerServer::getServiceStatus(char** toReturn, char* svc) {
	int len = 0;
	char* str;
	int size = sizeof(char) * (9 + 14 + strlen(serverName) + 11 + 12 + 10);
	char adm[XATMI_SERVICE_NAME_LENGTH + 1];
	ACE_OS::snprintf(adm, XATMI_SERVICE_NAME_LENGTH + 1, ".%s%d", server,
			serverid);

	str = (char*) malloc(size);
	len += ACE_OS::sprintf(str + len, "<server>");
	len += ACE_OS::sprintf(str + len, "<name>%s</name>", serverName);
	len += ACE_OS::sprintf(str + len, "<services>");
	for (std::vector<ServiceStatus>::iterator i = serviceStatus.begin(); i
			!= serviceStatus.end(); i++) {
		if (strcmp(adm, (*i).name) != 0 && (svc == NULL || ACE_OS::strcmp(svc,
				(*i).name) == 0)) {
			int svcsize = sizeof(char) * (50 + strlen((*i).name));
			size += svcsize;
			str = (char*) realloc(str, size);

			len += ACE_OS::sprintf(str + len,
					"<service><name>%.15s</name><status>%d</status></service>",
					(*i).name, isPause && (*i).status ? 2 : (*i).status);
			if (svc != NULL)
				break;
		}
	}

	len += ACE_OS::sprintf(str + len, "</services>");
	len += ACE_OS::sprintf(str + len, "</server>");
	*toReturn = str;
	return len;
}

void AtmiBrokerServer::updateServiceStatus(ServiceInfo* service, SVCFUNC func,
		bool status) {
	bool found = false;

	for (std::vector<ServiceStatus>::iterator i = serviceStatus.begin(); i
			!= serviceStatus.end(); i++) {
		if (strncmp((*i).name, service->serviceName, XATMI_SERVICE_NAME_LENGTH)
				== 0) {
			(*i).func = func;
			(*i).status = status;
			found = true;
			break;
		}
	}

	if (found == false) {
		ServiceStatus aServiceStatus;
		memset(&aServiceStatus, 0, sizeof(aServiceStatus));
		ACE_OS::strncpy(aServiceStatus.name, service->serviceName,
				XATMI_SERVICE_NAME_LENGTH);
		aServiceStatus.func = func;
		aServiceStatus.status = status;
		serviceStatus.push_back(aServiceStatus);
	}
}

bool AtmiBrokerServer::advertiseService(char * svcname) {
	for (std::vector<ServiceStatus>::iterator i = serviceStatus.begin(); i
			!= serviceStatus.end(); i++) {
		if (strncmp((*i).name, svcname, XATMI_SERVICE_NAME_LENGTH) == 0) {
			return advertiseService(svcname, (*i).func);
		}
	}
	LOG4CXX_WARN(
			loggerAtmiBrokerServer,
			(char*) "Could not advertise service, was not registered in btconfig.xml: "
					<< svcname);
	return false;
}

bool AtmiBrokerServer::advertiseService(char * svcname,
		void(*func)(TPSVCINFO *)) {

	if (!svcname || strlen(svcname) == 0) {
		setSpecific(TPE_KEY, TSS_TPEINVAL);
		LOG4CXX_DEBUG(loggerAtmiBrokerServer,
				(char*) "advertiseService invalid service name");
		return false;
	}

	char* serviceName = (char*) ::malloc(XATMI_SERVICE_NAME_LENGTH + 1);
	memset(serviceName, '\0', XATMI_SERVICE_NAME_LENGTH + 1);
	strncat(serviceName, svcname, XATMI_SERVICE_NAME_LENGTH);

	bool found = false;
	unsigned int i;
	ServiceInfo* service;
	for (i = 0; i < serverInfo.serviceVector.size(); i++) {
		if (strncmp(serverInfo.serviceVector[i].serviceName, serviceName,
				XATMI_SERVICE_NAME_LENGTH) == 0) {
			found = true;
			service = &serverInfo.serviceVector[i];
			break;
		}
	}
	if (!found) {
		LOG4CXX_WARN(
				loggerAtmiBrokerServer,
				(char*) "Could not advertise service, was not registered for server in btconfig.xml: "
						<< svcname);
		setSpecific(TPE_KEY, TSS_TPELIMIT);
		free(serviceName);
		return false;
	}
	void (*serviceFunction)(TPSVCINFO*) = getServiceMethod(serviceName);
	if (serviceFunction != NULL) {
		if (serviceFunction == func) {
			free(serviceName);
			return true;
		} else {
			setSpecific(TPE_KEY, TSS_TPEMATCH);
			free(serviceName);
			LOG4CXX_DEBUG(loggerAtmiBrokerServer,
					(char*) "advertiseService same service function");
			return false;
		}
	} else if (serviceFunction == NULL && func == NULL) {
		LOG4CXX_WARN(loggerAtmiBrokerServer,
				(char*) "Could not advertise service, no function available: "
						<< svcname);
		return false;
	}

	Connection* connection = connections.getServerConnection();
	if (connection == NULL) {
		setSpecific(TPE_KEY, TSS_TPESYSTEM);
		free(serviceName);
		LOG4CXX_DEBUG(loggerAtmiBrokerServer,
				(char*) "advertiseService no server connection");
		return false;
	}

	bool toReturn = false;

	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "advertiseService(): "
			<< serviceName);

	// create reference for Service Queue and cache
	try {
		if (service->externally_managed_destination == false) {
			toReturn = createAdminDestination(serviceName);
			LOG4CXX_DEBUG(loggerAtmiBrokerServer,
					(char*) "advertiseService status=" << toReturn);
		} else {
			toReturn = true;
			LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "service "
					<< serviceName << " has extern managed destination");
		}
		if (toReturn) {
			Destination* destination;
			destination = connection->createDestination(serviceName,
					service->conversational);

			LOG4CXX_DEBUG(loggerAtmiBrokerServer,
					(char*) "created destination: " << serviceName);

			addDestination(destination, func, service);
			updateServiceStatus(service, func, true);
			LOG4CXX_INFO(loggerAtmiBrokerServer, (char*) "advertised service "
					<< serviceName);
		}
	} catch (CORBA::Exception& e) {
		LOG4CXX_ERROR(loggerAtmiBrokerServer,
				(char*) "CORBA::Exception creating the destination: "
						<< serviceName << " Exception: " << e._name());
		setSpecific(TPE_KEY, TSS_TPEMATCH);
		try {
			if (service->externally_managed_destination == false) {
				removeAdminDestination(serviceName, true);
			}
		} catch (...) {
			LOG4CXX_ERROR(loggerAtmiBrokerServer,
					(char*) "Could not remove the destination: " << serviceName);
		}
	} catch (...) {
		LOG4CXX_ERROR(loggerAtmiBrokerServer,
				(char*) "Could not create the destination: " << serviceName);
		setSpecific(TPE_KEY, TSS_TPEMATCH);
		try {
			if (service->externally_managed_destination == false) {
				removeAdminDestination(serviceName, true);
			}
		} catch (...) {
			LOG4CXX_ERROR(loggerAtmiBrokerServer,
					(char*) "Could not remove the destination: " << serviceName);
		}
	}
	free(serviceName);
	return toReturn;
}

void AtmiBrokerServer::unadvertiseService(char * svcname, bool decrement) {
	char* serviceName = (char*) ::malloc(XATMI_SERVICE_NAME_LENGTH + 1);
	memset(serviceName, '\0', XATMI_SERVICE_NAME_LENGTH + 1);
	strncat(serviceName, svcname, XATMI_SERVICE_NAME_LENGTH);

	Connection* connection = connections.getServerConnection();
	if (connection == NULL) {
		return;
	}

	//	Connection* connz = connections.getServerConnection("BAR");
	//	delete connz;

	for (std::vector<ServiceData>::iterator i = serviceData.begin(); i
			!= serviceData.end(); i++) {
		if (strncmp((*i).serviceInfo->serviceName, svcname,
				XATMI_SERVICE_NAME_LENGTH) == 0) {
			LOG4CXX_DEBUG(loggerAtmiBrokerServer,
					(char*) "remove_service_queue: " << serviceName);
			Destination * destination = removeDestination(serviceName);
			LOG4CXX_DEBUG(loggerAtmiBrokerServer,
					(char*) "preparing to destroy" << serviceName);

			connection->destroyDestination(destination);
			LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "destroyed"
					<< serviceName);

			ServiceInfo* service;
			unsigned int i;
			bool found = false;
			for (i = 0; i < serverInfo.serviceVector.size(); i++) {
				if (strncmp(serverInfo.serviceVector[i].serviceName,
						serviceName, XATMI_SERVICE_NAME_LENGTH) == 0) {
					found = true;
					service = &serverInfo.serviceVector[i];
					break;
				}
			}

			if (found) {
				if (service->externally_managed_destination == false) {
					removeAdminDestination(serviceName, decrement);
				} else {
					LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "found "
							<< serviceName
							<< " and externally managed destination is true");
				}
			} else {
				LOG4CXX_WARN(loggerAtmiBrokerServer, (char*) "can not found "
						<< serviceName << " in btconfig.xml");
			}

			LOG4CXX_INFO(loggerAtmiBrokerServer,
					(char*) "unadvertised service " << serviceName);
			break;
		}
	}
	free(serviceName);
}

bool AtmiBrokerServer::createAdminDestination(char* serviceName) {
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "Creating admin queue for: "
			<< serviceName);

	bool isadm = false;
	char adm[XATMI_SERVICE_NAME_LENGTH + 1];
	ACE_OS::snprintf(adm, XATMI_SERVICE_NAME_LENGTH + 1, ".%s%d", server,
			serverid);
	if (strcmp(adm, serviceName) == 0) {
		isadm = true;
		LOG4CXX_DEBUG(loggerAtmiBrokerServer,
				(char*) "advertising admin service");
	}

	long commandLength;
	long responseLength = 1;

	commandLength = strlen(serverName) + strlen(serviceName) + strlen(version)
			+ 15 + 1;

	char* command = (char*) ::tpalloc((char*) "X_OCTET", NULL, commandLength);
	char* response = (char*) ::tpalloc((char*) "X_OCTET", NULL, responseLength);
	memset(command, '\0', commandLength);

	sprintf(command, "tpadvertise,%s,%s,%s,", serverName, serviceName, version);

	LOG4CXX_DEBUG(loggerAtmiBrokerServer,
			(char*) "createAdminDestination with command " << command);

	if (tpcall((char*) "BTStompAdmin", command, commandLength, &response,
			&responseLength, TPNOTRAN) != 0) {
		LOG4CXX_ERROR(loggerAtmiBrokerServer,
				"Could not advertise service with command: " << command);
		tpfree(command);
		tpfree(response);
		return false;
	} else if (responseLength != 1) {
		LOG4CXX_ERROR(loggerAtmiBrokerServer,
				"Service returned with unexpected response: " << response
						<< " with length " << responseLength);
		tpfree(command);
		tpfree(response);
		return false;
	} else if (response[0] == 4) {
		LOG4CXX_WARN(
				loggerAtmiBrokerServer,
				(char*) "Version Mismatch Detected: The version of BlackTie used by this server: "
						<< version
						<< " does not match the version of BlackTie in the deployed admin service (please ensure the server, client and admin service are all using the same version of BlackTie)");
		tpfree(command);
		tpfree(response);
		return false;
	} else if (response[0] == 3) {
		// Dispatcher needs to be paused
		LOG4CXX_DEBUG(loggerAtmiBrokerServer,
				(char*) "Created paused admin queue for: " << serviceName);
		if (isadm) {
			errorBootAdminService = 3;
		}
		tpfree(command);
		tpfree(response);
		return true;
	} else if (response[0] == 1) {
		LOG4CXX_DEBUG(loggerAtmiBrokerServer,
				(char*) "Created admin queue for: " << serviceName);
		tpfree(command);
		tpfree(response);
		return true;
	} else {
		int r = (int) response[0];
		char c = response[0];
		// REMOVED BY TOM, NOT CLEAR WHAT THIS IS REQUIRED FOR,
		// IF COMMENTED BACK IN, PLEASE PROVIDE A COMMENT
		//
		// UNCOMENTED BY AMOS
		// response[0] == 2 means start server with the same id.
		// if errorBootAdminService equals 2, the server will output the same id error and quit.
		if (!isadm || (errorBootAdminService = response[0]) == 2) {
			LOG4CXX_ERROR(loggerAtmiBrokerServer,
					"Service returned with error: " << response[0]
							<< " command was " << command << " r=" << r
							<< " c=" << c);
		}
		tpfree(command);
		tpfree(response);
		return false;
	}
}

void AtmiBrokerServer::removeAdminDestination(char* serviceName, bool decrement) {
	long commandLength;
	long responseLength = 1;
	char* command;

	if (decrement) {
		commandLength = strlen(serverName) + strlen(serviceName) + 21;
		command = (char*) ::tpalloc((char*) "X_OCTET", NULL, commandLength);
		sprintf(command, "decrementconsumer,%s,%s,", serverName, serviceName);
	} else {
		commandLength = strlen(serverName) + strlen(serviceName) + strlen(
				"tpunadvertise,,, ");
		command = (char*) ::tpalloc((char*) "X_OCTET", NULL, commandLength);
		sprintf(command, "tpunadvertise,%s,%s,", serverName, serviceName);
	}

	char* response = (char*) ::tpalloc((char*) "X_OCTET", NULL, responseLength);

	LOG4CXX_DEBUG(loggerAtmiBrokerServer, "Unadvertise with command: "
			<< command);
	if (tpcall((char*) "BTStompAdmin", command, commandLength, &response,
			&responseLength, TPNOTRAN) != 0) {
		LOG4CXX_ERROR(loggerAtmiBrokerServer,
				"Could not unadvertise service with command: " << command);
	} else if (responseLength != 1) {
		LOG4CXX_ERROR(loggerAtmiBrokerServer,
				"Service returned with unexpected response: " << response
						<< " with length " << responseLength);
	} else if (response[0] == 0) {
		LOG4CXX_ERROR(loggerAtmiBrokerServer, "Service returned with error: "
				<< command);
	} else {
		LOG4CXX_DEBUG(loggerAtmiBrokerServer, "Unadvertise ok");
	}
	tpfree(command);
	tpfree(response);
}

bool AtmiBrokerServer::isAdvertised(char * serviceName) {
	bool toReturn = false;
	for (std::vector<ServiceData>::iterator i = serviceData.begin(); i
			!= serviceData.end(); i++) {
		if (strncmp((*i).serviceInfo->serviceName, serviceName,
				XATMI_SERVICE_NAME_LENGTH) == 0) {
			toReturn = true;
		}
	}
	return toReturn;
}

void AtmiBrokerServer::addDestination(Destination* destination, void(*func)(
		TPSVCINFO *), ServiceInfo* service) {
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "addDestination: "
			<< destination->getName());

	ServiceData entry;
	entry.destination = destination;
	entry.func = func;
	entry.serviceInfo = service;

	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "constructor: "
			<< destination->getName());

	Connection* connection = connections.getServerConnection();
	if (connection == NULL) {
		return;
	}

	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "createPool");
	SynchronizableObject* reconnect = new SynchronizableObject();
	for (int i = 0; i < entry.serviceInfo->poolSize; i++) {
		ServiceDispatcher* dispatcher = new ServiceDispatcher(this,
				destination, connection, destination->getName(), func, isPause,
				reconnect, entry.serviceInfo->conversational);
		if (dispatcher->activate(THR_NEW_LWP | THR_JOINABLE, 1, 0,
				ACE_DEFAULT_THREAD_PRIORITY, -1, 0, 0, 0, 0, 0, 0) != 0) {
			delete dispatcher;
			LOG4CXX_ERROR(loggerAtmiBrokerServer,
					(char*) "Could not start thread pool");
		} else {
			entry.dispatchers.push_back(dispatcher);
			LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) " destination "
					<< destination);
		}
	}

	serviceData.push_back(entry);
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "added: "
			<< destination->getName());
}

Destination* AtmiBrokerServer::removeDestination(const char * aServiceName) {
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "removeDestination: "
			<< aServiceName);
	Destination* toReturn = NULL;
	for (std::vector<ServiceData>::iterator i = serviceData.begin(); i
			!= serviceData.end(); i++) {
		if (strncmp((*i).serviceInfo->serviceName, aServiceName,
				XATMI_SERVICE_NAME_LENGTH) == 0) {
			toReturn = (*i).destination;
			LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "removing service "
					<< aServiceName);
			for (std::vector<ServiceDispatcher*>::iterator j =
					(*i).dispatchers.begin(); j != (*i).dispatchers.end(); j++) {
				ServiceDispatcher* dispatcher = (*j);
				dispatcher->shutdown();
			}
			LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "shutdown notified "
					<< aServiceName);

			for (std::vector<ServiceDispatcher*>::iterator j =
					(*i).dispatchers.begin(); j != (*i).dispatchers.end(); j++) {
				toReturn->disconnect();
			}
			LOG4CXX_DEBUG(loggerAtmiBrokerServer,
					(char*) "disconnect notified " << aServiceName);

			SynchronizableObject* reconnect = NULL;

			for (std::vector<ServiceDispatcher*>::iterator j =
					(*i).dispatchers.begin(); j != (*i).dispatchers.end(); j++) {
				ServiceDispatcher* dispatcher = (*j);
				if (dispatcher != NULL) {
/*					LOG4CXX_TRACE(loggerAtmiBrokerServer,
							(char*) "Waiting for dispatcher notified "
									<< aServiceName);
					dispatcher->wait();
*/
					LOG4CXX_TRACE(loggerAtmiBrokerServer,
							(char*) "Deleting dispatcher " << aServiceName);
					reconnect = dispatcher->getReconnect();
					//delete dispatcher;
					serviceDispatchersToDelete.push_back(dispatcher);
					LOG4CXX_TRACE(loggerAtmiBrokerServer,
							(char*) "Dispatcher deleted " << aServiceName);
				} else {
					LOG4CXX_TRACE(loggerAtmiBrokerServer,
							(char*) "NULL Dispatcher detected for"
									<< aServiceName);
				}
				LOG4CXX_TRACE(loggerAtmiBrokerServer,
						(char*) "Erasing dispatcher " << aServiceName);
				//j = (*i).dispatchers.erase(j);
			}
			LOG4CXX_DEBUG(loggerAtmiBrokerServer,
					(char*) "waited for dispatcher: " << aServiceName);
			if (reconnect != NULL) {
				LOG4CXX_DEBUG(loggerAtmiBrokerServer,
						(char*) "Deleting reconnect");
				delete reconnect;
			}

			updateServiceStatus((*i).serviceInfo, (*i).func, false);
			serviceData.erase(i);
			LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "removed: "
					<< aServiceName);
			break;
		}
	}
	return toReturn;
}

void AtmiBrokerServer::server_done() {
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "server_done()");

	for (int i = serverInfo.serviceVector.size() - 1; i >= 0; i--) {
		char* svcname = (char*) serverInfo.serviceVector[i].serviceName;
		if (isAdvertised(svcname)) {
			unadvertiseService(svcname, true);
		}
	}

	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "server_done(): returning.");
}

long AtmiBrokerServer::getServiceMessageCounter(char* serviceName) {
	for (std::vector<ServiceData>::iterator i = serviceData.begin(); i
			!= serviceData.end(); i++) {
		if (strncmp((*i).destination->getName(), serviceName,
				XATMI_SERVICE_NAME_LENGTH) == 0) {
			long counter = 0;
			for (std::vector<ServiceDispatcher*>::iterator dispatcher =
					(*i).dispatchers.begin(); dispatcher
					!= (*i).dispatchers.end(); dispatcher++) {
				counter += (*dispatcher)->getCounter();
			}
			return counter;
		}
	}
	return 0;
}

long AtmiBrokerServer::getServiceErrorCounter(char* serviceName) {
	for (std::vector<ServiceData>::iterator i = serviceData.begin(); i
			!= serviceData.end(); i++) {
		if (strncmp((*i).destination->getName(), serviceName,
				XATMI_SERVICE_NAME_LENGTH) == 0) {
			long counter = 0;
			for (std::vector<ServiceDispatcher*>::iterator dispatcher =
					(*i).dispatchers.begin(); dispatcher
					!= (*i).dispatchers.end(); dispatcher++) {
				counter += (*dispatcher)->getErrorCounter();
			}
			return counter;
		}
	}
	return 0;
}

void AtmiBrokerServer::getResponseTime(char* serviceName, unsigned long* min,
		unsigned long* avg, unsigned long* max) {
	*min = 0;
	*avg = 0;
	*max = 0;

	for (std::vector<ServiceData>::iterator i = serviceData.begin(); i
			!= serviceData.end(); i++) {
		if (strncmp((*i).destination->getName(), serviceName,
				XATMI_SERVICE_NAME_LENGTH) == 0) {
			long counter = 0;
			long total = 0;
			unsigned long min_time;
			unsigned long max_time;
			unsigned long avg_time;

			for (std::vector<ServiceDispatcher*>::iterator dispatcher =
					(*i).dispatchers.begin(); dispatcher
					!= (*i).dispatchers.end(); dispatcher++) {
				counter = (*dispatcher)->getCounter();
				(*dispatcher)->getResponseTime(&min_time, &avg_time, &max_time);
				if (*min == 0 || min_time < *min) {
					*min = min_time;
				}

				if (total != 0 || counter != 0) {
					*avg = ((*avg) * total + avg_time * counter) / (total
							+ counter);
				}

				if (max_time > *max) {
					*max = max_time;
				}
			}
		}
	}
}

void (*AtmiBrokerServer::getServiceMethod(const char * aServiceName))(TPSVCINFO *) {
			LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "getServiceMethod: "
					<< aServiceName);

			for (std::vector<ServiceData>::iterator i = serviceData.begin(); i
					!= serviceData.end(); i++) {
				if (strncmp((*i).destination->getName(), aServiceName,
						XATMI_SERVICE_NAME_LENGTH) == 0) {
					LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "found: "
							<< aServiceName);
					return (*i).func;
				}
			}
			LOG4CXX_DEBUG(loggerAtmiBrokerServer,
					(char*) "getServiceMethod out: " << aServiceName);
			return NULL;
		}
