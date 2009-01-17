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

// Class: AtmiBroker_ServiceFactoryImpl
// A POA servant which implements of the AtmiBroker::ServiceFactory interface
//

#ifndef ATMIBROKER_SERVICEFACTORYIMPL_H_
#define ATMIBROKER_SERVICEFACTORYIMPL_H_

#include "atmiBrokerMacro.h"

#ifdef TAO_COMP
#include "tao/ORB.h"
#include "AtmiBrokerS.h"
#include "AtmiBrokerC.h"
#elif ORBIX_COMP
#include <omg/orb.hh>
#include "AtmiBrokerS.hh"
#include "AtmiBroker.hh"
#endif
#ifdef VBC_COMP
#include <orb.h>
#include "AtmiBroker_s.hh"
#include "AtmiBroker_c.hh"
#endif

#include <vector>
#include "it_servant_base_overrides.h"
#include "AtmiBroker_ServiceImpl.h"

class ATMIBROKER_DLL AtmiBroker_ServiceFactoryImpl: public virtual IT_ServantBaseOverrides, public virtual POA_AtmiBroker::ServiceFactory {
public:
	AtmiBroker_ServiceFactoryImpl(PortableServer::POA_ptr, char * serviceName, char *servicePoaName, char *descriptorFileName);

	virtual ~AtmiBroker_ServiceFactoryImpl();

	// _create() -- create a new servant.
	// Hides the difference between direct inheritance and tie servants.
	//
	//static POA_AtmiBroker::ServiceFactory* _create(PortableServer::POA_ptr);


	// IDL operations
	//
	virtual AtmiBroker::Service_ptr get_service(CORBA::Long client_id, CORBA::String_out id) throw (CORBA::SystemException );

	virtual char*
	get_service_id(CORBA::Long client_id, CORBA::String_out id) throw (CORBA::SystemException );

	virtual void end_conversation(CORBA::Long client_id, const char* id) throw (CORBA::SystemException );

	virtual AtmiBroker::Service_ptr find_service(CORBA::Long client_id, const char* id) throw (CORBA::SystemException );

	virtual char *
	find_service_id(CORBA::Long client_id, const char* id) throw (CORBA::SystemException );

	virtual AtmiBroker::ServiceInfo*
	get_service_info() throw (CORBA::SystemException );

	void createServantCache(void(*func)(TPSVCINFO *));
	int getMaxObjects();

protected:

	std::vector<AtmiBroker::Service_var> corbaObjectVector;
	std::vector<AtmiBroker_ServiceImpl *> servantVector;
	AtmiBroker::ServiceInfo serviceInfo;
	/*****
	 int 				maxObjects;
	 int 				minObjects;
	 int 				minAvailableObjects;
	 int 				logLevel;
	 *****/
	char* serviceName;
	char* servicePoaName;
	char* descriptorFileName;
	PortableServer::POA_var factoryPoaPtr;
	PortableServer::POA_var servicePoaPtr;

	void createCacheInstance(int i, void(*func)(TPSVCINFO *));
	virtual void createPoa();
	virtual void createReference(PortableServer::ObjectId& anId, AtmiBroker::Service_var* refPtr);
	virtual AtmiBroker_ServiceImpl * createServant(int aIndex, void(*func)(TPSVCINFO *));
	virtual void getDescriptorData();

private:
	// The following are not implemented
	//
	AtmiBroker_ServiceFactoryImpl(const AtmiBroker_ServiceFactoryImpl &);
	AtmiBroker_ServiceFactoryImpl& operator=(const AtmiBroker_ServiceFactoryImpl &);
};

#endif
