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

#ifndef ResourceManagerCache_h
#define ResourceManagerCache_h

#include "atmiBrokerTxMacro.h"

#ifdef TAO_COMP
#include "XAC.h"
#elif ORBIX_COMP
#include "XA.hh"
#endif
#ifdef VBC_COMP
#include "XA_s.hh"
#endif

#include "LocalResourceManager.h"

#include <deque>

class BLACKTIE_TX_DLL ResourceManagerCache {
public:
	ResourceManagerCache();

	virtual ~ResourceManagerCache(void);

	ResourceManagerCache(const ResourceManagerCache &src);
	ResourceManagerCache& operator=(const ResourceManagerCache &);
	bool operator==(const ResourceManagerCache&);

public:

	static ResourceManagerCache * getResourceManagerCache();

	virtual XA::ResourceManager_ptr create_resource_manager(const std::string& resource_manager_name, const std::string& open_string, const std::string& close_string, XA::ThreadModel thread_model, CORBA::Boolean automatic_association, CORBA::Boolean dynamic_registration_optimization, const std::string& library_name, const std::string& xa_symbol_name);

	virtual XA::ResourceManager_ptr find_resource_manager(const std::string& resource_manager_name, const std::string& open_string, const std::string& close_string, XA::ThreadModel thread_model, CORBA::Boolean automatic_association, CORBA::Boolean dynamic_registration_optimization);
private:
	std::deque<ResourceManagerDataStruct*> resourceManagerQueue;
};
#endif
