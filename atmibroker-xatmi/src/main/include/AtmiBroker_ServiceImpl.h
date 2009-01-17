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

//-----------------------------------------------------------------------------
// Edit the idlgen.cfg to have your own copyright notice placed here.
//-----------------------------------------------------------------------------

// Class: AtmiBroker_ServiceImpl
// A POA servant which implements of the AtmiBroker::Service interface
//

#ifndef ATMIBROKER_SERVICEIMPL_H_
#define ATMIBROKER_SERVICEIMPL_H_

#include "atmiBrokerMacro.h"

#ifdef TAO_COMP
#include "tao/ORB.h"
#include "AtmiBrokerS.h"
#include "CosTransactionsS.h"
#include "AtmiBrokerC.h"
#elif ORBIX_COMP
#include <omg/CosTransactions.hh>
#include <omg/orb.hh>
#include "AtmiBrokerS.hh"
#include "AtmiBroker.hh"
#endif
#ifdef VBC_COMP
#include "CosTransactions_s.hh"
#include <orb.h>
#include "AtmiBroker_s.hh"
#include "AtmiBroker_c.hh"
#endif

#include "xatmi.h"
#include "it_servant_base_overrides.h"
#include <queue>

class AtmiBroker_ServiceFactoryImpl;

class ATMIBROKER_DLL AtmiBroker_ServiceImpl: public virtual IT_ServantBaseOverrides, public virtual POA_AtmiBroker::Service {
public:
	AtmiBroker_ServiceImpl(AtmiBroker_ServiceFactoryImpl* aParent, PortableServer::POA_ptr, int aIndex, char *serviceName, void(*func)(TPSVCINFO *));

	virtual ~AtmiBroker_ServiceImpl();

	// _create() -- create a new servant.
	// Hides the difference between direct inheritance and tie servants.
	//
	static POA_AtmiBroker::Service*
	_create(AtmiBroker_ServiceFactoryImpl* aParent, PortableServer::POA_ptr, int aIndex, char *serviceName, void(*func)(TPSVCINFO *));

	// IDL operations
	//
	virtual void send_data(CORBA::Boolean inConversation, const AtmiBroker::octetSeq& idata, CORBA::Long ilen, CORBA::Long flags, CosTransactions::Control_ptr control) throw (CORBA::SystemException );

	// IDL attributes
	//
	char* serviceName() throw (CORBA::SystemException);

	void tpreturn(int rval, long rcode, char* data, long len, long flags);
	int tpsend(int id, char* idata, long ilen, long flags, long *revent);
	int tprecv(int id, char ** odata, long *olen, long flags, long* event);

	CORBA::Boolean isInUse();
	void setInUse(CORBA::Boolean anInd);

	long getClientId();
	void setClientId(long aClientId);

	bool sameBuffer(char *toCheck);
protected:
	AtmiBroker_ServiceFactoryImpl* parent;
	CORBA::Short returnStatus;
	CORBA::Boolean inUse;
	AtmiBroker::ClientCallback_ptr callbackRef;
	int index;
	long clientId;
	char* dataType;

	CosTransactions::Control_var tx_control;
	CosTransactions::Coordinator_var tx_coordinator;
	CosTransactions::PropagationContext_var tx_propagation_context;
	CosTransactions::otid_t otid;

	void createConnectionTransactionAssociation(CosTransactions::Control_ptr control);
	void endConnectionTransactionAssociation();

	CORBA::String_var m_serviceName;

private:

	const AtmiBroker::octetSeq* m_octetSeq;
	const AtmiBroker::TypedBuffer* m_typedBuffer;

	// Instance variables for attributes.
	//
	// The following are not implemented
	//
	AtmiBroker_ServiceImpl(const AtmiBroker_ServiceImpl &);
	AtmiBroker_ServiceImpl& operator=(const AtmiBroker_ServiceImpl &);

	void (*m_func)(TPSVCINFO *);
};

#endif
