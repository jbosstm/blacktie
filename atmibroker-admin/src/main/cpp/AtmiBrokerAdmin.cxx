#include "log4cxx/logger.h"
#include "ace/OS_NS_stdlib.h"
#include "ace/OS_NS_stdio.h"
#include "ace/OS_NS_string.h"
#include "AtmiBrokerAdmin.h"
#include "AtmiBrokerServerControl.h"
#include "xatmi.h"
#include "userlog.h"
#include "string.h"

log4cxx::LoggerPtr loggerAtmiBrokerAdmin(log4cxx::Logger::getLogger(
			"AtmiBrokerAdmin"));

#ifdef __cplusplus
extern "C" {
#endif

void ADMIN(TPSVCINFO* svcinfo) {
	char* req = (char*) malloc ((svcinfo->len + 1) * sizeof(char));
	memset(req, 0, svcinfo->len + 1);
	ACE_OS::strncpy(req, svcinfo->data, svcinfo->len);

	char* toReturn = NULL;
	long  len = 1;
	toReturn = tpalloc((char*) "X_OCTET", NULL, len);
	toReturn[0] = '0';

	if(strncmp(req, "serverdone", 10) == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerAdmin, (char*) "get serverdone command");
		toReturn[0] = '1';
		server_sigint_handler_callback(0);
	} else if(strncmp(req, "advertise", 9) == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerAdmin, (char*) "get advertise command");
		strtok(req, ",");
		char* svc = strtok(NULL, ",");
		if(svc != NULL && advertiseByAdmin(svc) == 0) {
			LOG4CXX_DEBUG(loggerAtmiBrokerAdmin, (char*) "advertise service " << svc << " OK");
			toReturn[0] = '1';
		} else {
			LOG4CXX_WARN(loggerAtmiBrokerAdmin, (char*) "advertise service " << svc << " FAIL");
		}
	} else if(strncmp(req, "unadvertise", 11) == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerAdmin, (char*) "get unadvertise command");
		strtok(req, ",");
		char* svc = strtok(NULL, ",");
		if (svc != NULL && tpunadvertise(svc) == 0) {
			toReturn[0] = '1';
			LOG4CXX_DEBUG(loggerAtmiBrokerAdmin, (char*) "unadvertise service " << svc << " OK");
		} else {
			LOG4CXX_WARN(loggerAtmiBrokerAdmin, (char*) "unadvertise service " << svc << " FAIL");
		}
	} else if(strncmp(req, "status", 6) == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerAdmin, (char*) "get status command");
		toReturn = tprealloc(toReturn, 1024);
		len += getServiceStatus(&toReturn[1]) + 1;
		toReturn[0] = '1';
	} else if(strncmp(req, "counter", 6) == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerAdmin, (char*) "get counter command");
		strtok(req, ",");
		char* svc = strtok(NULL, ",");
		long counter = 0;

		if(svc != NULL) {
			toReturn = tprealloc(toReturn, 16);
			counter = getServiceMessageCounter(svc);
			len += ACE_OS::sprintf(&toReturn[1], "%ld", counter) + 1;
			toReturn[0] = '1';
		}
	}

	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerAdmin,
			(char*) "service done");
	free(req);
	tpreturn(TPSUCCESS, 0, toReturn, len, 0);
}

#ifdef __cplusplus
}
#endif
