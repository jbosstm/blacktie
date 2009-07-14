#include "log4cxx/logger.h"
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
	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerAdmin,
			(char*) "get request");

	char* req = svcinfo->data;
	char* toReturn = NULL;
	int   len = 0;
	if(strcmp(req, "serverdone") == 0) {
		LOG4CXX_INFO(loggerAtmiBrokerAdmin, (char*) "get serverdone command");
		len = 1;
		toReturn = tpalloc((char*) "X_OCTET", NULL, len);
		toReturn[0] = '1';
	}

	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerAdmin,
			(char*) "service done");
	tpreturn(TPSUCCESS, 0, toReturn, len, 0);
}

#ifdef __cplusplus
}
#endif
