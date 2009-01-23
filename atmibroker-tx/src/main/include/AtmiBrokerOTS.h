/*
 * JBoss, Home of Professional Open Source
 * Copyright 2008, Red Hat Middleware LLC, and others contributors as indicated
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
/*
 * BREAKTHRUIT PROPRIETARY - NOT TO BE DISCLOSED OUTSIDE BREAKTHRUIT, LLC.
 */
// copyright 2006, 2008 BreakThruIT

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

#include "Worker.h"

#include "CurrentImpl.h"

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

	int suspend(long& tranid);

	int resume(long tranid);

	int tx_close(void);

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

	static void  init_orb(char*, Worker*&, CORBA::ORB_ptr&, CosNaming::NamingContextExt_var&, CosNaming::NamingContext_var&);
	void setCorbaObjects(CORBA::ORB_var orb, CosNaming::NamingContextExt_var namingContextExt, CosNaming::NamingContext_var namingContext);
	CORBA::ORB_ptr& getOrb();
	CosNaming::NamingContextExt_ptr& getNamingContextExt();
	CosNaming::NamingContext_ptr& getNamingContext();

private:
	void createTransactionPolicy();

//	CORBA::Policy_var transactionPolicy;

	Worker* ots_worker;
	CORBA::ORB_var ots_orb;
	CosNaming::NamingContextExt_var ots_namingContextExt;
	CosNaming::NamingContext_var ots_namingContext;

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

	std::vector<ControlInfo*> controlInfoVector;

	static AtmiBrokerOTS* ptrAtmiBrokerOTS;

};

#endif //AtmiBroker_OTS_H
