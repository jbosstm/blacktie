/*
 * SymbolLoader.cpp
 *
 *  Created on: Mar 11, 2009
 *      Author: tom
 */

#include "SymbolLoader.h"

#include "ace/DLL.h"

#include "log4cxx/logger.h"

log4cxx::LoggerPtr symbolLoaderLogger(log4cxx::Logger::getLogger(
		"symbolLoaderLogger"));

void* lookup_symbol(const char *lib, const char *symbol) {
	LOG4CXX_LOGLS(symbolLoaderLogger, log4cxx::Level::getTrace(),
			(char *) "lookup_symbol " << symbol << (char *) " in library "
					<< lib);

	if (symbol == NULL || lib == NULL)
		return 0;

	ACE_DLL dll;
	int retval = dll.open(lib, ACE_DEFAULT_SHLIB_MODE, 0);

	if (retval != 0) {
		LOG4CXX_ERROR(symbolLoaderLogger, (char*) "lookup_symbol: " << symbol
				<< (char *) " dll.open error: " << dll.error());
		return NULL;
	}

	void * sym = NULL;

	try {
		sym = dll.symbol(symbol);

		if (sym == NULL) {
			LOG4CXX_ERROR(symbolLoaderLogger, (char*) "lookup_symbol: "
					<< symbol << (char *) " dlsym error: " << dll.error());
			dll.close();
			return NULL;
		}

		LOG4CXX_TRACE(symbolLoaderLogger, (char *) "symbol addr=" << sym);

		return sym;
	} catch (std::exception& e) {
		LOG4CXX_ERROR(symbolLoaderLogger, (char *) "symbol addr=" << sym
				<< e.what());
		return NULL;
	}
}

