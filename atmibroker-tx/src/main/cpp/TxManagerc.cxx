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

#include "TxManager.h"
#include "txClient.h"
#include "OrbManagement.h"

using namespace atmibroker::tx;

log4cxx::LoggerPtr txmclogger(log4cxx::Logger::getLogger("TxLogManagerc"));

/* X/Open tx interface */

int tx_open(void) {
	FTRACE(txmclogger, "ENTER");
	return TxManager::get_instance()->open();
}

int tx_begin(void) {
	FTRACE(txmclogger, "ENTER");
	return TxManager::get_instance()->begin();
}

int tx_commit(void) {
	FTRACE(txmclogger, "ENTER");
	return TxManager::get_instance()->commit();
}

int tx_rollback(void) {
	FTRACE(txmclogger, "ENTER");
	return TxManager::get_instance()->rollback();
}

int tx_close(void) {
	FTRACE(txmclogger, "ENTER");
	return TxManager::get_instance()->close();
}

int tx_set_commit_return(COMMIT_RETURN when_return) {
	FTRACE(txmclogger, "ENTER");
	return TxManager::get_instance()->set_commit_return(when_return);
}
int tx_set_transaction_control(TRANSACTION_CONTROL control) {
	FTRACE(txmclogger, "ENTER");
	return TxManager::get_instance()->set_transaction_control(control);
}
int tx_set_transaction_timeout(TRANSACTION_TIMEOUT timeout) {
	FTRACE(txmclogger, "ENTER");
	return TxManager::get_instance()->set_transaction_timeout(timeout);
}
int tx_info(TXINFO *info) {
	FTRACE(txmclogger, "ENTER");
	return TxManager::get_instance()->info(info);
}

/* Blacktie tx interface additions */
int txx_rollback_only()
{
	FTRACE(txmclogger, "ENTER");
    return TxManager::get_instance()->rollback_only();
}

void * txx_start(char* connectionName)
{
	FTRACE(txmclogger, "ENTER");
    return TxManager::get_instance()->init_orb(connectionName);
}

void txx_stop(void)
{
	FTRACE(txmclogger, "ENTER");
    TxManager::discard_instance();
	FTRACE(txmclogger, "<");
}

int txx_bind(void *control)
{
	FTRACE(txmclogger, "ENTER " << control);
    return TxManager::get_instance()->tx_resume((CosTransactions::Control_ptr) control, ACE_OS::thr_self(), TMRESUME);
}

int txx_bind(void *control, int tid)
{
	FTRACE(txmclogger, "ENTER " << tid);
    return TxManager::get_instance()->tx_resume((CosTransactions::Control_ptr) control, tid, TMRESUME);
}

int txx_associate_serialized(char *orbname, char* ctrlIOR)
{
	FTRACE(txmclogger, "ENTER" << orbname);
    return TxManager::get_instance()->tx_resume(ctrlIOR, orbname, TMRESUME);
}

void *txx_unbind(void)
{
	FTRACE(txmclogger, "ENTER");
    return (void *) TxManager::get_instance()->tx_suspend(0, TMSUSPEND | TMMIGRATE);
}

void *txx_unbind_if_not_owner(void)
{
	FTRACE(txmclogger, "ENTER");
    void *ctrl = (void *) TxManager::get_instance()->tx_suspend(ACE_OS::thr_self(), TMSUSPEND | TMMIGRATE);
	FTRACE(txmclogger, "< with control " << ctrl);

	return ctrl;
}

void *txx_get_control()
{
	FTRACE(txmclogger, "ENTER");
    void *ctrl = (void *) TxManager::get_ots_control();
	FTRACE(txmclogger, "< with control " << ctrl);
	return ctrl;
}

void txx_release_control(void *control)
{
	FTRACE(txmclogger, "ENTER");
    CosTransactions::Control_ptr cp = (CosTransactions::Control_ptr) control;

	try {
		if (!CORBA::is_nil(cp))
			CORBA::release(cp);
		else
			FTRACE(txmclogger, "< nothing to release");
	} catch (...) {
	}
}

char* txx_serialize(char *orbname)
{
	FTRACE(txmclogger, "ENTER " << orbname);
    CORBA::ORB_ptr orb = find_orb(orbname);
    CosTransactions::Control_ptr ctrl = TxManager::get_ots_control();

    if (!CORBA::is_nil(orb) && !CORBA::is_nil(ctrl)) {
		CORBA::String_var cs = orb->object_to_string(ctrl);
		char *cstr = ACE_OS::strdup(cs);
		CORBA::release(ctrl);

        return cstr;
	}

	FTRACE(txmclogger, "< No tx ior");
    return NULL;
}
