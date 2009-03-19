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
#include <cppunit/extensions/HelperMacros.h>

#include "TestSymbolLoader.h"

#include "ace/OS.h"
#include "AtmiBrokerEnv.h"
#include "userlogc.h"

void TestSymbolLoader::test() {
	char* lib = AtmiBrokerEnv::get_instance()->getenv("test-lib");
	char* symbol = AtmiBrokerEnv::get_instance()->getenv("test-symbol");
	ACE_SHLIB_HANDLE handle = ACE_OS::dlopen(lib, ACE_DEFAULT_SHLIB_MODE);

	if (!handle) {
		userlogc((char*) "lookup_dll- %s:%s", lib,  ACE_OS::dlerror());
		CPPUNIT_FAIL("lookup_dll");
	}

	void * sym = NULL;

	try {
		void * sym = ACE_OS::dlsym(handle, symbol);

		if (ACE_OS::dlerror()) {
			userlogc((char*) "lookup_symbol- %s:%s", symbol,  ACE_OS::dlerror());
			ACE_OS::dlclose(handle);
			CPPUNIT_FAIL("lookup_symbol");
		}

		userlogc((char*) "found symbol");
	} catch (std::exception& e) {
		userlogc((char *) "symbol addr%s=%s", sym, e.what());
		CPPUNIT_FAIL("exception");
	}
}
