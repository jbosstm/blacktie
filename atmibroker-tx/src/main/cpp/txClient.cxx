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
#include "CurrentTx.h"

#include "ace/OS_NS_string.h"
#include "ace/Thread.h"

#include "ThreadLocalStorage.h"
#include "log4cxx/logger.h"

log4cxx::LoggerPtr txClientLogger(log4cxx::Logger::getLogger("TxClient"));

int set_rollback_only()
{
	return AtmiBrokerOTS::get_instance()->tx_rollback_only();
}

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
	return associate_tx(control, ACE_OS::thr_self());
}

int associate_tx(void *control, int creator)
{
	setSpecific(TSS_KEY, new CurrentTx(control, creator));

	try {
		LOG4CXX_LOGLS(txClientLogger, log4cxx::Level::getDebug(), (char *) "Associating tx " << control << " id " << creator);
		return AtmiBrokerOTS::get_instance()->rm_resume();
	} catch (...) {
		LOG4CXX_LOGLS(txClientLogger, log4cxx::Level::getDebug(), (char *) "Error resuming RMs");
		return XAER_NOTA;
	}
}

int associate_serialized_tx(char *orbname, char* control)
{
	AtmiBrokerOTS::get_instance(); // Prepare an orb
	CORBA::Object_ptr p = atmi_string_to_object(control, orbname);

	if (!CORBA::is_nil(p)) {
		CosTransactions::Control_ptr cptr = CosTransactions::Control::_narrow(p);
		CORBA::release(p); // dispose of it now that we have narrowed the object reference

		return associate_tx(cptr, 0);
	}

	return TMER_INVAL;
}

void * disassociate_tx(void)
{
	void *control = get_control();

	if (control) {
		LOG4CXX_LOGLS(txClientLogger, log4cxx::Level::getDebug(),
				(char *) "disassociate_tx: caller=" << ACE_OS::thr_self());

		try {
			(void) AtmiBrokerOTS::get_instance()->rm_suspend();
		} catch (...) {
			LOG4CXX_LOGLS(txClientLogger, log4cxx::Level::getError(), (char *) "Error suspending RMs");
		}

		CurrentTx *tx = (CurrentTx *) getSpecific(TSS_KEY);
		delete tx;
		destroySpecific(TSS_KEY);

		return control;
	} else {
		LOG4CXX_LOGLS(txClientLogger, log4cxx::Level::getDebug(), (char *) "NO CONTROL");
		return NULL;
	}
}

void * disassociate_tx_if_not_owner(void)
{
	CurrentTx *tx = (CurrentTx *) getSpecific(TSS_KEY);

	if (tx) {
		ACE_thread_t thr_id = ACE_OS::thr_self();

		LOG4CXX_LOGLS(txClientLogger, log4cxx::Level::getDebug(),
			(char *) "disassociate_tx: caller=" << thr_id << " owner " << (int) thr_id);

		if (tx->thr_id() != (int) thr_id) {
			try {
				LOG4CXX_LOGLS(txClientLogger, log4cxx::Level::getDebug(), (char *) "suspending RMs ...");
				(void) AtmiBrokerOTS::get_instance()->rm_suspend();
			} catch (...) {
				LOG4CXX_LOGLS(txClientLogger, log4cxx::Level::getDebug(), (char *) "Error suspending RMs");
			}

			delete tx;
			destroySpecific(TSS_KEY);
		}

		return tx->control();
	}

	return 0;
}

void * get_control()
{
	CurrentTx *tx = (CurrentTx *) getSpecific(TSS_KEY);

	return (tx ? tx->control() : 0);
}

char* serialize_tx(char *orbname)
{
	CORBA::ORB_ptr orb = find_orb(orbname);
	CosTransactions::Control_ptr ctrl = (CosTransactions::Control_ptr) get_control();

	if (!CORBA::is_nil(orb) && !CORBA::is_nil(ctrl))
		return ACE_OS::strdup(orb->object_to_string(ctrl));

	return NULL;
}
