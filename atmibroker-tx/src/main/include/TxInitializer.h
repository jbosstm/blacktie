/*
 * JBoss, Home of Professional Open Source
 * Copyright 2009, Red Hat Middleware LLC, and others contributors as indicated
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
#ifndef TX_INITIALIZER_H
#define TX_INITIALIZER_H

#include "atmiBrokerTxMacro.h"

#include "tao/PortableInterceptorC.h"
#include "tao/PI/PI.h"

extern ATMIBROKER_TX_DLL void register_tx_interceptors(CORBA::ORB_ptr& orbPtr);

class ATMIBROKER_TX_DLL TxInitializer : public virtual PortableInterceptor::ORBInitializer
{
public:
	static TxInitializer* get_instance();
    void set_orb(CORBA::ORB_ptr*);
    virtual void pre_init (PortableInterceptor::ORBInitInfo_ptr info);
    virtual void post_init (PortableInterceptor::ORBInitInfo_ptr info);

protected:
    TxInitializer();
	CORBA::ORB_ptr* get_orb() { return orb; }

	CORBA::ORB_ptr* orb;
	static TxInitializer* instance;
};
#endif
