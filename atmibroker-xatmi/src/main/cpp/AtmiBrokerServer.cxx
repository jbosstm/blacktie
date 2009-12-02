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
#include "AtmiBrokerServer.h"
#include "AtmiBrokerPoaFac.h"
#include "AtmiBrokerEnv.h"
#include "userlog.h"
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

log4cxx::LoggerPtr loggerAtmiBrokerServer(log4cxx::Logger::getLogger(
		"AtmiBrokerServer"));
AtmiBrokerServer * ptrServer = NULL;
bool serverInitialized = false;
PortableServer::POA_var server_poa;
bool configFromCmdline = false;
int  errorBootAdminService = 0;
char configDir[256];
char server[30];
int serverid = 0;

void server_sigint_handler_callback(int sig_type) {
	signal(SIGINT, SIG_IGN);
	userlog(
			log4cxx::Level::getInfo(),
			loggerAtmiBrokerServer,
			(char*) "SIGINT Detected: Shutting down server this may take several minutes");
	ptrServer->shutdown();
	userlog(log4cxx::Level::getInfo(), loggerAtmiBrokerServer,
			(char*) "SIGINT Detected: Shutdown complete");
}

int serverrun() {
	setSpecific(TPE_KEY, TSS_TPERESET);
	return ptrServer->block();
}

int parsecmdline(int argc, char** argv) {
	ACE_Get_Opt getopt(argc, argv, ACE_TEXT("c:i:"));
	int c;
	int r = 0;
	bool isSetServerId = false;

	configFromCmdline = false;
	while ((c = getopt()) != -1) {
		switch ((char) c) {
		case 'c':
			configFromCmdline = true;
			ACE_OS::strncpy(configDir, getopt.opt_arg(), 256);
			break;
		case 'i':
			serverid = atoi(getopt.opt_arg());
			if (serverid <= 0) {
				r = -1;
			} else {
				isSetServerId = true;
			}
			break;
		default:
			r = -1;
		}
	}

	int last = getopt.opt_ind();
	if (r == 0 && last < argc) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServer,
				(char*) "opt_ind is %d, server is %s", last, argv[last]);
		ACE_OS::strncpy(server, argv[last], 30);
	}

	if (isSetServerId == false) {
		fprintf(stderr, "you must specify a server id with -i greater than 0\n");
		r = -1;
	}

	return r;
}

const char* getConfiguration() {
	const char* dir = NULL;

	if (configFromCmdline) {
		dir = configDir;
	}

	return dir;
}

int serverinit(int argc, char** argv) {
	setSpecific(TPE_KEY, TSS_TPERESET);
	int toReturn = 0;

	ACE_OS::strncpy(server, "default", 30);

	if (argc > 0 && parsecmdline(argc, argv) != 0) {
		fprintf(stderr, "usage:%s [-c config] -i id [server]\n", argv[0]);
		toReturn = -1;
		setSpecific(TPE_KEY, TSS_TPESYSTEM);
	}

	if (toReturn != -1 && ptrServer == NULL) {
		const char* configuration = getConfiguration();
		if (configuration != NULL) {
			AtmiBrokerEnv::set_configuration(configuration);
			LOG4CXX_DEBUG(loggerAtmiBrokerServer,
					(char*) "set AtmiBrokerEnv configuration type " << configuration);
		}

		try {
			initializeLogger();
			AtmiBrokerEnv* env = AtmiBrokerEnv::get_instance();
			std::stringstream sid;
			sid << "BLACKTIE_SERVER_NAME=" << domain << server << serverid;
			env->putenv((char *) (sid.str().c_str()));

			LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "serverinit called");
			ptrServer = new AtmiBrokerServer();

			if (!serverInitialized) {
				::serverdone();
				toReturn = -1;
				setSpecific(TPE_KEY, TSS_TPESYSTEM);
			} else {
				// make ADMIN service mandatory for server
				char adm[XATMI_SERVICE_NAME_LENGTH + 1];
				ACE_OS::snprintf(adm, XATMI_SERVICE_NAME_LENGTH + 1, "%s_ADMIN_%d", server, serverid);
				tpadvertise(adm, ADMIN);

				if(errorBootAdminService == 2) {
					userlog(log4cxx::Level::getWarn(), loggerAtmiBrokerServer,
							(char*) "Maybe the same server id running");
					throw std::exception();
				}

				ptrServer->advertiseAtBootime();

				userlog(log4cxx::Level::getInfo(), loggerAtmiBrokerServer,
						(char*) "Server %d Running", serverid);
				//signal(SIGINT, server_sigint_handler_callback);
				ACE_Sig_Action
						sa(
								reinterpret_cast<ACE_SignalHandler> (server_sigint_handler_callback));

				// Make sure we specify that SIGINT will be masked out
				// during the signal handler's execution.
				ACE_Sig_Set ss;
				ss.sig_add(SIGINT);
				sa.mask(ss);
				sa.register_action(SIGINT);
			}

		} catch (...) {
			userlog(log4cxx::Level::getError(), loggerAtmiBrokerServer,
					(char*) "Server startup failed");
			toReturn = -1;
			setSpecific(TPE_KEY, TSS_TPESYSTEM);
		}
	}
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
				ACE_OS::snprintf(adm, XATMI_SERVICE_NAME_LENGTH + 1, "%s_ADMIN_%d", server, serverid);
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

					if (servers[i]->serviceVector[j].securityType) {
						service.securityType = strdup(
								servers[i]->serviceVector[j].securityType);
					} else {
						service.securityType = NULL;
					}
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
			serverInitialized = true;

			LOG4CXX_DEBUG(loggerAtmiBrokerServer,
					(char*) "server_init(): finished.");
		}
	} catch (CORBA::Exception& e) {
		userlog(log4cxx::Level::getError(), loggerAtmiBrokerServer,
				(char*) "serverinit - Unexpected CORBA exception: %s",
				e._name());
		setSpecific(TPE_KEY, TSS_TPESYSTEM);
	}
}

// ~AtmiBrokerServer destructor.
//
AtmiBrokerServer::~AtmiBrokerServer() {
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "destructor");
	server_done();
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "Server done");

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
		if (service->securityType != NULL) {
			free(service->securityType);
		}
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
	if(errorBootAdminService == 3) {
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
	finish->lock();
	finish->notify();
	finish->unlock();
}

int AtmiBrokerServer::pause() {
	if (!isPause) {
		char adm[XATMI_SERVICE_NAME_LENGTH + 1];
		ACE_OS::snprintf(adm, XATMI_SERVICE_NAME_LENGTH + 1, "%s_ADMIN_%d", server, serverid);
		for (std::vector<ServiceData>::iterator i = serviceData.begin(); i
				!= serviceData.end(); i++) {
			if(ACE_OS::strcmp((*i).serviceInfo->serviceName, adm) != 0) {
				LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "pausing service"
						<< (*i).serviceInfo->serviceName);
				for (std::vector<ServiceDispatcher*>::iterator j =
						(*i).dispatchers.begin(); j != (*i).dispatchers.end(); j++) {
					ServiceDispatcher* dispatcher = (*j);
					if (dispatcher->pause() != 0) {
						LOG4CXX_WARN(loggerAtmiBrokerServer, (char*) "pause service dispatcher "
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
	if(isPause) {
		for (std::vector<ServiceData>::iterator i = serviceData.begin(); i
				!= serviceData.end(); i++) {
			LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "resuming service"
					<< (*i).serviceInfo->serviceName);
			for (std::vector<ServiceDispatcher*>::iterator j =
					(*i).dispatchers.begin(); j != (*i).dispatchers.end(); j++) {
				ServiceDispatcher* dispatcher = (*j);
				if (dispatcher->resume() != 0) {
					LOG4CXX_WARN(loggerAtmiBrokerServer, (char*) "resume service dispatcher "
							<< dispatcher << " failed");
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
	ACE_OS::snprintf(adm, XATMI_SERVICE_NAME_LENGTH + 1, "%s_ADMIN_%d", server, serverid);
	
	str = (char*)malloc(size);
	len += ACE_OS::sprintf(str + len, "<server>");
	len += ACE_OS::sprintf(str + len, "<name>%s</name>", serverName);
	len += ACE_OS::sprintf(str + len, "<services>");
	for (std::vector<ServiceStatus>::iterator i = serviceStatus.begin(); i
			!= serviceStatus.end(); i++) {
		if(strcmp(adm, (*i).name) != 0 && 
			(svc == NULL || ACE_OS::strcmp(svc, (*i).name) == 0)) {
			int svcsize = sizeof(char) * (50 + strlen((*i).name));
			size += svcsize;
			str = (char*)realloc(str, size);

			len += ACE_OS::sprintf(str + len,
					"<service><name>%.15s</name><status>%d</status></service>",
					(*i).name, isPause && (*i).status? 2 : (*i).status);
			if(svc != NULL) break;
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
	LOG4CXX_WARN(loggerAtmiBrokerServer, (char*) "Could not advertise service, was not registered in Environment.xml: " << svcname);
	return false;
}

bool AtmiBrokerServer::advertiseService(char * svcname,
		void(*func)(TPSVCINFO *)) {

	if (!svcname || strlen(svcname) == 0) {
		setSpecific(TPE_KEY, TSS_TPEINVAL);
		return false;
	}

	bool isadm = false;
	char adm[XATMI_SERVICE_NAME_LENGTH + 1];
	ACE_OS::snprintf(adm, XATMI_SERVICE_NAME_LENGTH + 1, "%s_ADMIN_%d", server, serverid);
	if (strcmp(adm, svcname) == 0) {
		isadm = true;
		LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "advertise Admin svc "
				<< adm);
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
		LOG4CXX_WARN(loggerAtmiBrokerServer, (char*) "Could not advertise service, was not registered for server in Environment.xml: " << svcname);
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
			return false;
		}
	}

	Connection* connection = connections.getServerConnection(serviceName);
	if (connection == NULL) {
		setSpecific(TPE_KEY, TSS_TPESYSTEM);
		free(serviceName);
		return false;
	}

	bool toReturn = false;

	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "advertiseService(): "
			<< serviceName);

	// create reference for Service Queue and cache
	try {
		LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "create_service_queue: "
				<< serviceName);

		if (connection->requiresAdminCall()) {
			long commandLength;
			long responseLength = 1;

			commandLength = strlen(serverName) + strlen(serviceName) + 15;

			char* command = (char*) ::tpalloc((char*) "X_OCTET", NULL,
					commandLength);
			char* response = (char*) ::tpalloc((char*) "X_OCTET", NULL,
					responseLength);
			memset(command, '\0', commandLength);

			sprintf(command, "tpadvertise,%s,%s,", serverName, serviceName);

			if (tpcall((char*) "BTStompAdmin", command, commandLength,
					&response, &responseLength, TPNOTRAN) != 0) {
				LOG4CXX_ERROR(loggerAtmiBrokerServer,
						"Could not advertise service with command: " << command);
				tpfree(command);
				free(serviceName);
				return false;
			} else if (responseLength != 1) {
				LOG4CXX_ERROR(loggerAtmiBrokerServer,
						"Service returned with unexpected response: "
								<< response << " with length "
								<< responseLength);
				tpfree(command);
				free(serviceName);
				return false;
			} else if (response[0] != 1) {
				if(!isadm || (errorBootAdminService = response[0]) == 2) {
					LOG4CXX_ERROR(loggerAtmiBrokerServer,
						"Service returned with error: " << command);
					tpfree(command);
					free(serviceName);
					return false;
				}
			}
			tpfree(command);
		}
		LOG4CXX_DEBUG(loggerAtmiBrokerServer,
				(char*) "invoked create_service_queue: " << serviceName);

		Destination* destination;
		destination = connection->createDestination(serviceName);

		LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "created destination: "
				<< serviceName);

		addDestination(destination, func, service);
		updateServiceStatus(service, func, true);
		LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "added destination: "
				<< serviceName);
	} catch (CORBA::Exception& e) {
		LOG4CXX_ERROR(loggerAtmiBrokerServer,
				(char*) "CORBA::Exception creating the destination: "
						<< serviceName << " Exception: " << e._name());
		setSpecific(TPE_KEY, TSS_TPEMATCH);
		try {
			removeAdminDestination(serviceName);
		} catch (...) {
			LOG4CXX_ERROR(loggerAtmiBrokerServer,
					(char*) "Could not remove the destination: " << serviceName);
		}
		free(serviceName);
		return false;
	} catch (...) {
		LOG4CXX_ERROR(loggerAtmiBrokerServer,
				(char*) "Could not create the destination: " << serviceName);
		setSpecific(TPE_KEY, TSS_TPEMATCH);
		try {
			removeAdminDestination(serviceName);
		} catch (...) {
			LOG4CXX_ERROR(loggerAtmiBrokerServer,
					(char*) "Could not remove the destination: " << serviceName);
		}
		free(serviceName);
		return false;
	}

	advertisedServices.push_back(serviceName);
	LOG4CXX_INFO(loggerAtmiBrokerServer, (char*) "advertised service "
			<< serviceName);
	toReturn = true;
	return toReturn;
}

void AtmiBrokerServer::unadvertiseService(char * svcname) {
	char* serviceName = (char*) ::malloc(XATMI_SERVICE_NAME_LENGTH + 1);
	memset(serviceName, '\0', XATMI_SERVICE_NAME_LENGTH + 1);
	strncat(serviceName, svcname, XATMI_SERVICE_NAME_LENGTH);

	Connection* connection = connections.getServerConnection(serviceName);
	if (connection == NULL) {
		return;
	}

	//	Connection* connz = connections.getServerConnection("BAR");
	//	delete connz;

	for (std::vector<char*>::iterator i = advertisedServices.begin(); i
			!= advertisedServices.end(); i++) {
		char* name = (*i);
		if (strcmp(serviceName, name) == 0) {
			removeAdminDestination(serviceName);

			LOG4CXX_DEBUG(loggerAtmiBrokerServer,
					(char*) "remove_service_queue: " << serviceName);
			Destination * destination = removeDestination(serviceName);
			LOG4CXX_DEBUG(loggerAtmiBrokerServer,
					(char*) "preparing to destroy" << serviceName);

			connection->destroyDestination(destination);
			LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "destroyed"
					<< serviceName);
			advertisedServices.erase(i);
			free(name);
			LOG4CXX_INFO(loggerAtmiBrokerServer,
					(char*) "unadvertised service " << serviceName);
			break;
		}
	}
	free(serviceName);
}

void AtmiBrokerServer::removeAdminDestination(char* serviceName) {
	Connection* connection = connections.getServerConnection(serviceName);

	if (connection->requiresAdminCall()) {
		long commandLength;
		long responseLength = 1;

		commandLength = strlen(serverName) + strlen(serviceName) + 21;

		char* command = (char*) ::tpalloc((char*) "X_OCTET", NULL,
				commandLength);
		char* response = (char*) ::tpalloc((char*) "X_OCTET", NULL,
				responseLength);
		memset(command, '\0', commandLength);

		sprintf(command, "decrementconsumer,%s,%s,", serverName, serviceName);

		if (tpcall((char*) "BTStompAdmin", command, commandLength, &response,
				&responseLength, TPNOTRAN) != 0) {
			LOG4CXX_ERROR(loggerAtmiBrokerServer,
					"Could not advertise service with command: " << command);
		} else if (responseLength != 1) {
			LOG4CXX_ERROR(loggerAtmiBrokerServer,
					"Service returned with unexpected response: " << response
							<< " with length " << responseLength);
		} else if (response[0] == 0) {
			LOG4CXX_ERROR(loggerAtmiBrokerServer,
					"Service returned with error: " << command);
		}
		tpfree(command);
	}
}

bool AtmiBrokerServer::isAdvertised(char * serviceName) {
	bool toReturn = false;
	for (std::vector<char*>::iterator i = advertisedServices.begin(); i
			!= advertisedServices.end(); i++) {
		if (strncmp(serviceName, (*i), XATMI_SERVICE_NAME_LENGTH) == 0) {
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

	Connection* connection = connections.getServerConnection(
			(char*) service->serviceName);
	if (connection == NULL) {
		return;
	}

	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "createPool");
	for (int i = 0; i < entry.serviceInfo->poolSize; i++) {
		ServiceDispatcher* dispatcher = new ServiceDispatcher(destination,
				connection, destination->getName(), func, isPause);
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

			for (std::vector<ServiceDispatcher*>::iterator j =
					(*i).dispatchers.begin(); j != (*i).dispatchers.end();) {
				ServiceDispatcher* dispatcher = (*j);
				if (dispatcher != NULL) {
					LOG4CXX_TRACE(loggerAtmiBrokerServer,
							(char*) "Waiting for dispatcher notified "
									<< aServiceName);
					dispatcher->wait();
					LOG4CXX_TRACE(loggerAtmiBrokerServer,
							(char*) "Deleting dispatcher " << aServiceName);
					delete dispatcher;
					LOG4CXX_TRACE(loggerAtmiBrokerServer,
							(char*) "Dispatcher deleted " << aServiceName);
				} else {
					LOG4CXX_TRACE(loggerAtmiBrokerServer,
							(char*) "NULL Dispatcher detected for"
									<< aServiceName);
				}
				LOG4CXX_TRACE(loggerAtmiBrokerServer,
						(char*) "Erasing dispatcher " << aServiceName);
				j = (*i).dispatchers.erase(j);
			}
			LOG4CXX_DEBUG(loggerAtmiBrokerServer,
					(char*) "waited for dispatcher: " << aServiceName);

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
			unadvertiseService(svcname);
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
