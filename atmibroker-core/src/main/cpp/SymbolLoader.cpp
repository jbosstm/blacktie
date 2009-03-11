/*
 * SymbolLoader.cpp
 *
 *  Created on: Mar 11, 2009
 *      Author: tom
 */

#include "SymbolLoader.h"

#include "ace/OS.h"

#include "log4cxx/logger.h"

log4cxx::LoggerPtr symbolLoaderLogger(log4cxx::Logger::getLogger("symbolLoaderLogger"));

void* lookup_symbol(const char *lib, const char *symbol) {
	LOG4CXX_LOGLS(symbolLoaderLogger, log4cxx::Level::getTrace(),
			(char *) "lookup_symbol " << symbol << (char *) " in library "
					<< lib);

	if (symbol == NULL || lib == NULL)
		return 0;

	ACE_SHLIB_HANDLE handle = ACE_OS::dlopen(lib, ACE_DEFAULT_SHLIB_MODE);

	if (!handle) {
		LOG4CXX_ERROR(symbolLoaderLogger, (char*) "lookup_symbol: " << symbol
				<< (char *) " dlopen error: " << ACE_OS::dlerror());
		return NULL;
	}

	void * sym = NULL;

	try {
		void * sym = ACE_OS::dlsym(handle, symbol);

		if (ACE_OS::dlerror()) {
			LOG4CXX_ERROR(symbolLoaderLogger, (char*) "lookup_symbol: " << symbol
					<< (char *) " dlsym error: " << ACE_OS::dlerror());
			ACE_OS::dlclose(handle);
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

