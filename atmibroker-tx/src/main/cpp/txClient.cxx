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
#include "OrbManagement.h"

#include "ace/OS_NS_string.h"

#include "ThreadLocalStorage.h"
#include "log4cxx/logger.h"

log4cxx::LoggerPtr txClientLogger(log4cxx::Logger::getLogger("TxClient"));

void * start_tx_orb(char* connectionName)
{
	return AtmiBrokerOTS::init_orb(connectionName);
}

void shutdown_tx_broker(void)
{
	AtmiBrokerOTS::discard_instance();
}

int associate_tx(void *control)
{
	setSpecific(TSS_KEY, control);

	return AtmiBrokerOTS::get_instance()->rm_resume();
}

int associate_serialized_tx(char *orbname, char* control)
{
	CORBA::Object_ptr p = atmi_string_to_object(control, orbname);

	if (!CORBA::is_nil(p)) {
		CosTransactions::Control_ptr cptr = CosTransactions::Control::_narrow(p);
		CORBA::release(p); // dispose of it now that we have narrowed the object reference

		return associate_tx(cptr);
	}

	return TMER_INVAL;
}

void * disassociate_tx(void)
{
	void *control = getSpecific(TSS_KEY);

	(void) AtmiBrokerOTS::get_instance()->rm_suspend();

	if (control)
		destroySpecific(TSS_KEY);

	return control;
}

char* serialize_tx(char *orbname)
{
	CORBA::ORB_ptr orb = find_orb(orbname);
	CosTransactions::Control_ptr ctrl = (CosTransactions::Control_ptr) getSpecific(TSS_KEY);

	if (!CORBA::is_nil(orb) && !CORBA::is_nil(ctrl))
		return ACE_OS::strdup(orb->object_to_string(ctrl));

	return NULL;
}
