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

#ifndef LocalResourceManagerCache_h
#define LocalResourceManagerCache_h

#include "atmiBrokerTxMacro.h"

#ifdef TAO_COMP
#include "XAC.h"
#elif ORBIX_COMP
#include "XA.hh"
#endif
#ifdef VBC_COMP
#include "XA_c.hh"
#endif

#include "LocalResourceManager.h"

#include <deque>
#include <string>

class BLACKTIE_TX_DLL LocalResourceManagerCache {
public:
	LocalResourceManagerCache();

	virtual ~LocalResourceManagerCache(void);

	LocalResourceManagerCache(const LocalResourceManagerCache &src);
	LocalResourceManagerCache& operator=(const LocalResourceManagerCache &);
	bool operator==(const LocalResourceManagerCache&);

public:

	static LocalResourceManagerCache * getLocalResourceManagerCache();

	static void discardLocalResourceManagerCache();

	virtual LocalResourceManager * create_local_resource_manager(const std::string& resource_manager_name, const std::string& open_string, const std::string& close_string, XA::ThreadModel thread_model, CORBA::Boolean automatic_association, CORBA::Boolean dynamic_registration_optimization, const std::string& library_name, const std::string& xa_symbol_name);

	virtual LocalResourceManager * find_local_resource_manager(const std::string& resource_manager_name, const std::string& open_string, const std::string& close_string, XA::ThreadModel thread_model, CORBA::Boolean automatic_association, CORBA::Boolean dynamic_registration_optimization);
private:
	std::deque<LocalResourceManager*> localResourceManagerQueue;
};
#endif
