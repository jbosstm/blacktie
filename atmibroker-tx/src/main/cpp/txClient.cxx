/*
 * JBoss, Home of Professional Open Source
 * Copyright 2009, Red Hat, Inc., and others contributors as indicated
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
#include "txClient.h"
#include "AtmiBrokerOTS.h"

// includes for looking up orbs
#include "tao/ORB_Core.h"
#include "tao/ORB_Table.h"
#include "tao/ORB_Core_Auto_Ptr.h"
#include "OrbManagement.h"

#include "ThreadLocalStorage.h"
#include "log4cxx/logger.h"

log4cxx::LoggerPtr txClientLogger(log4cxx::Logger::getLogger("TxClient"));

CORBA_CONNECTION* startTxOrb(char* connectionName)
{
	return AtmiBrokerOTS::init_orb(connectionName);
}

void shutdownTxBroker(void)
{
	AtmiBrokerOTS::discard_instance();
}

int associateTx(void *control)
{
	setSpecific(TSS_KEY, control);

	return AtmiBrokerOTS::get_instance()->rm_resume();
}

void * disassociateTx(void)
{
	void *control = getSpecific(TSS_KEY);

	(void) AtmiBrokerOTS::get_instance()->rm_suspend();

	if (control)
		destroySpecific(TSS_KEY);

	return control;
}

CORBA::ORB_ptr find_orb(const char * name)
{
	TAO::ORB_Table * const orb_table = TAO::ORB_Table::instance();
	::TAO_ORB_Core* oc = orb_table->find(name);

	return (oc == 0 ? NULL : oc->orb());
}

CORBA::Object_ptr current_control()
{
	return (CosTransactions::Control_ptr) getSpecific(TSS_KEY);
}

char* txObjectToString(CORBA::Object_ptr ctrl, char * orbname)
{
	if (!CORBA::is_nil(ctrl)) {
		LOG4CXX_LOGLS(txClientLogger, log4cxx::Level::getTrace(), (char *) "current is not NULL");
		TAO::ORB_Table * const orb_table = TAO::ORB_Table::instance();
		::TAO_ORB_Core* oc = orb_table->find(orbname);
		char * p = (oc == 0 ? NULL : oc->orb()->object_to_string(ctrl));
		if (p != NULL)
			return strdup(p);
	} else {
		LOG4CXX_LOGLS(txClientLogger, log4cxx::Level::getTrace(), (char *) "current is NULL");
	}

	return NULL;
}

CORBA::Object_ptr txStringToObject(char * ior, char * orbname)
{
	LOG4CXX_LOGLS(txClientLogger, log4cxx::Level::getTrace(), (char *) "\tconverting ior: " << (ior ? ior : "NULL"));

	if (ior != NULL) {
		TAO::ORB_Table * const orb_table = TAO::ORB_Table::instance();
		::TAO_ORB_Core* oc = orb_table->find(orbname);
		return (oc == 0 ? NULL : oc->orb()->string_to_object(ior));
	}

	return NULL;
}
