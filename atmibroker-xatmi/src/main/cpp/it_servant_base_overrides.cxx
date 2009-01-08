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

// IT_ServantBase -- base class to override PortableServer::ServantBase
// functions for all our servants.
//
#ifdef TAO_COMP
#include <tao/ORB.h>
#elif ORBIX_COMP
#include <omg/orb.hh>
#endif
#ifdef VBC_COMP
#include <orb.h>
#endif
#include "it_servant_base_overrides.h"

IT_ServantBaseOverrides::IT_ServantBaseOverrides(PortableServer::POA_ptr the_poa) :
	m_poa(the_poa->_duplicate(the_poa)) {
}

IT_ServantBaseOverrides::~IT_ServantBaseOverrides() {
}

PortableServer::POA_ptr IT_ServantBaseOverrides::_default_POA() {
	return m_poa->_duplicate(m_poa);
}

