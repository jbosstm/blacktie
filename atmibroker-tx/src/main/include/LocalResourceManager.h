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

#ifndef LocalResourceManager_h
#define LocalResourceManager_h

#include "atmiBrokerTxMacro.h"

#ifdef TAO_COMP
#include "XAC.h"
#include "xa.h"
#include "ResourceManagerFactoryC.h"
#elif ORBIX_COMP
#include "XA.hh"
namespace XA
{
	typedef struct xa_switch_t XASwitch;
};
#include "ResourceManagerFactory.hh"
#endif
#ifdef VBC_COMP
#include "XA_s.hh"
#include "xa.h"
#include "ResourceManagerFactory_c.hh"
#endif

#include <string>

struct BLACKTIE_TX_DLL _resourceManagerDataStruct {
	std::string resource_manager_name;
	std::string open_string;
	std::string close_string;
	int rmid;
	//XA::XASwitch		xaSwitch;
	XA::XASwitch_ptr xaSwitch;
	XA::ThreadModel thread_model;
	CORBA::Boolean automatic_association;
	CORBA::Boolean dynamic_registration_optimization;
	XA::ResourceManager_ptr resourceManager;
};
typedef _resourceManagerDataStruct ResourceManagerDataStruct;

/**
 "local obj" Implementation of XA::ResourceManager interface.
 this class inherits from the stubs, not the skeletons
 because the XA::ResourceManager interface is a "locality contrained" interface
 */
class BLACKTIE_TX_DLL LocalResourceManager: public virtual XA::ResourceManager {
public:
	LocalResourceManager(ResourceManagerDataStruct& aResourceManagerDataStruct);

	~LocalResourceManager(void);

	LocalResourceManager(const LocalResourceManager &src);
	LocalResourceManager& operator=(const LocalResourceManager &);
	bool operator==(const LocalResourceManager&);

public:
	CORBA::ULong register_before_completion_callback(XA::BeforeCompletionCallback_ptr bcc) throw(CORBA::SystemException);

	void unregister_before_completion_callback(unsigned int key) throw(CORBA::SystemException);

	ResourceManagerDataStruct& getResourceManagerDataStruct();

private:
	ResourceManagerDataStruct m_resourceManagerDataStruct;
	AtmiBrokerXA::ResourceManagerFactory_ptr m_resourceManagerFactory;
};
#endif
