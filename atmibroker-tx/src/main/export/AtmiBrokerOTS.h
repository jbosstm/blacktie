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

// CorbaTransaction.h

#ifndef AtmiBroker_OTS_H
#define AtmiBroker_OTS_H

#ifdef TAO_COMP
#include <orbsvcs/CosNamingS.h>
#include "CosTransactionsS.h"
#include "XAS.h"
#elif ORBIX_COMP
#include <omg/CosNaming.hh>
#include <omg/CosTransactions.hh>
#include <omg/XA.hh>
#endif
#ifdef VBC_COMP
#include <CosNaming_c.hh>
#include "CosTransactions_s.hh"
#include "XA_s.hh"
#endif

#include <iostream>
#include <vector>

#include "CorbaConnection.h"

#include "CurrentImpl.h"
#include "XAResourceManagerFactory.h"
#include "tx.h"

extern "C" {
struct _control_info {
	long id;
	CosTransactions::Control_ptr control;
};
typedef _control_info ControlInfo;
}
;

class ATMIBROKER_TX_DLL AtmiBrokerOTS {

public:

	AtmiBrokerOTS();

	~AtmiBrokerOTS();

	int tx_open(void);

	int tx_begin(void);

	int tx_commit(void);

	int tx_rollback(void);

	int tx_close(void);

	int set_commit_return(COMMIT_RETURN);
	COMMIT_RETURN get_commit_return() {return whenReturn;}
	int set_transaction_control(TRANSACTION_CONTROL);
	TRANSACTION_CONTROL get_transaction_control() {return txControlMode;}
	int set_transaction_timeout(TRANSACTION_TIMEOUT);
	TRANSACTION_TIMEOUT get_transaction_timeout() {return txTimeout;}
	int info(TXINFO *);
	bool isChained() {return txControlMode == TX_CHAINED;}
	bool reportHeuristics() {return whenReturn == TX_COMMIT_COMPLETED;}

	int suspend(long& tranid);
	int resume(long tranid);

	int rm_end(void);
	int rm_resume(void);
	int rm_suspend(void);

	void createXAConnectorAndResourceManager();
	CurrentImpl * getCurrentImpl();
	CosTransactions::Current_var& getCurrent();
	CosTransactions::Control_ptr getSuspended(long tranid);
	XA::CurrentConnection_var& getXaCurrentConnection();
	XA::ResourceManager_var& getXaResourceManager();
	XA::Connector_var& getXaConnector();

	//struct xa_switch_t&								getXaosw();
	XA::XASwitch_ptr getXaosw();

	static AtmiBrokerOTS* get_instance();
	static void discard_instance();

	static CORBA_CONNECTION* init_orb(char* name);
	CORBA::ORB_ptr getOrb();
	CosNaming::NamingContextExt_ptr getNamingContextExt();
	CosNaming::NamingContext_ptr getNamingContext();

private:
	void createTransactionPolicy();
	int tx_complete(bool);
	int chainTransaction(int);

	CORBA_CONNECTION* ots_connection;

	// OTS References
	long nextControlId;
	CurrentImpl * currentImpl;
	CosTransactions::Current_var tx_current;
	CosTransactions::TransactionFactory_var tx_factory;

	// XA References
	XA::CurrentConnection_var xa_current_connection;
	XA::ResourceManager_var xa_resource_manager;
	XA::Connector_var xa_connector;
	//struct xa_switch_t								xaosw;
	XA::XASwitch_ptr xaosw;

	XAResourceManagerFactory xaRMFac;

	std::vector<ControlInfo*> controlInfoVector;

	COMMIT_RETURN whenReturn;
	TRANSACTION_CONTROL txControlMode;
	TRANSACTION_TIMEOUT txTimeout;

	static AtmiBrokerOTS* ptrAtmiBrokerOTS;

};

#endif //AtmiBroker_OTS_H
