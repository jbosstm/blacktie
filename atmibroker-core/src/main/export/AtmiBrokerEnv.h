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

#ifndef AtmiBroker_ENV_H
#define AtmiBroker_ENV_H

#include "atmiBrokerCoreMacro.h"

#include <iostream>
#include <vector>

#include "AtmiBrokerEnvXml.h"

class ATMIBROKER_CORE_DLL AtmiBrokerEnv {

public:

	AtmiBrokerEnv();

	~AtmiBrokerEnv();

	char* getenv(char* anEnvName);

	int putenv(char* anEnvNameValue);

	std::vector<envVar_t>& getEnvVariableInfoSeq();

	static AtmiBrokerEnv* get_instance();
	static void discard_instance();

	static int ENV_VARIABLE_SIZE;
	static char* ENVIRONMENT_FILE;

private:

	int readenv(char* aEnvFileName, char* label);

	std::vector<envVar_t> envVariableInfoSeq;
	bool readEnvironment;

	static AtmiBrokerEnv* ptrAtmiBrokerEnv;
};

#endif //AtmiBroker_ENV_H
