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

// IT_ServantBase -- base class to override PortableServer::ServantBase
// functions for all our servants.
//

#ifndef _IT_SERVANT_BASE_OVERRIDES_H_
#define _IT_SERVANT_BASE_OVERRIDES_H_

#include "atmiBrokerMacro.h"

#ifdef TAO_COMP
#include <tao/ORB.h>
#include "tao/PortableServer/PortableServer.h"
#include "tao/PortableServer/Servant_Base.h"
#elif ORBIX_COMP
#include <omg/orb.hh>
#include <omg/PortableServer.hh>
#endif
#ifdef VBC_COMP
#include <orb.h>
#include <PortableServerExt_c.hh>
#endif
#include <assert.h>

class ATMIBROKER_DLL IT_ServantBaseOverrides: public virtual PortableServer::ServantBase {
public:
	IT_ServantBaseOverrides(PortableServer::POA_ptr);

	virtual ~IT_ServantBaseOverrides();

	virtual PortableServer::POA_ptr _default_POA();

protected:
	PortableServer::POA_var m_poa;

private:

	// Not defined, prevent accidental use.
	//
	IT_ServantBaseOverrides(const IT_ServantBaseOverrides&);
	IT_ServantBaseOverrides& operator=(const IT_ServantBaseOverrides&);
};

#endif
