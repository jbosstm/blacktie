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

//-----------------------------------------------------------------------------
// Edit the idlgen.cfg to have your own copyright notice placed here.
//-----------------------------------------------------------------------------

// Class: AtmiBrokerXA_ResourceManagerFactoryImpl
// A POA servant which implements of the AtmiBroker::ResourceManagerFactory interface
//

#ifndef AtmiBrokerXA_RESOURCE_MANAGER_FACTORY_IMPL_H_
#define AtmiBrokerXA_RESOURCE_MANAGER_FACTORY_IMPL_H_

#include "atmiBrokerTxMacro.h"

#ifdef TAO_COMP
#include "tao/ORB.h"
#include "ResourceManagerFactoryS.h"
#elif ORBIX_COMP
#include <omg/orb.hh>
#include "ResourceManagerFactoryS.hh"
#endif
#ifdef VBC_COMP
#include <orb.h>
#include "ResourceManagerFactory_s.hh"
#endif

// TODO READD AND THE EXTENSION POINT #include "it_servant_base_overrides.h"

class AtmiBrokerXA_ResourceManagerFactoryFactoryImpl;

class ATMIBROKER_TX_DLL AtmiBrokerXA_ResourceManagerFactoryImpl: public virtual PortableServer::ServantBase, public virtual POA_AtmiBrokerXA::ResourceManagerFactory {
public:
	AtmiBrokerXA_ResourceManagerFactoryImpl(PortableServer::POA_ptr);

	virtual ~AtmiBrokerXA_ResourceManagerFactoryImpl();

	// _create() -- create a new servant.
	// Hides the difference between direct inheritance and tie servants.
	//
	static POA_AtmiBrokerXA::ResourceManagerFactory*
	_create(PortableServer::POA_ptr);

	// IDL operations
	//
	virtual XA::ResourceManager_ptr create_resource_manager(const char * resource_manager_name, const char * open_string, const char * close_string, XA::ThreadModel thread_model, CORBA::Boolean automatic_association, CORBA::Boolean dynamic_registration_optimization) throw (CORBA::SystemException );

	virtual char *
	create_resource_manager_ior(const char * resource_manager_name, const char * open_string, const char * close_string, XA::ThreadModel thread_model, CORBA::Boolean automatic_association, CORBA::Boolean dynamic_registration_optimization) throw (CORBA::SystemException );

protected:

private:
	// Instance variables for attributes.
	//
	// The following are not implemented
	//
	AtmiBrokerXA_ResourceManagerFactoryImpl(const AtmiBrokerXA_ResourceManagerFactoryImpl &);
	AtmiBrokerXA_ResourceManagerFactoryImpl& operator=(const AtmiBrokerXA_ResourceManagerFactoryImpl &);
};

#endif
