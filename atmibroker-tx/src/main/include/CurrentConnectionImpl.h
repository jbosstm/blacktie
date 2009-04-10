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

#ifndef CurrentConnectionImpl_h
#define CurrentConnectionImpl_h

#include "atmiBrokerTxMacro.h"

#ifdef TAO_COMP
#include "XAC.h"
#elif ORBIX_COMP
#include "XA.hh"
#endif
#ifdef VBC_COMP
#include "XA_s.hh"
#endif

#include "xa.h"

#include "LocalResourceManager.h"

/**
 "local obj" Implementation of XA::CurrentConnection interface.
 this class inherits from the stubs, not the skeletons
 because the XA::CurrentConnection interface is a "locality contrained" interface
 */
class BLACKTIE_TX_DLL CurrentConnectionImpl: public virtual XA::CurrentConnection {
public:
	CurrentConnectionImpl(LocalResourceManager& localResourceManager);

	virtual ~CurrentConnectionImpl(void);

	CurrentConnectionImpl(const CurrentConnectionImpl &src);
	CurrentConnectionImpl& operator=(const CurrentConnectionImpl &);
	bool operator==(const CurrentConnectionImpl&);

public:
	virtual void start(CosTransactions::Coordinator_ptr tx, const CosTransactions::otid_t &) throw(CORBA::SystemException);

	virtual void suspend(CosTransactions::Coordinator_ptr tx, const CosTransactions::otid_t &) throw(CORBA::SystemException);

	virtual void resume(CosTransactions::Coordinator_ptr tx, const CosTransactions::otid_t &) throw(CORBA::SystemException);

	virtual void end(CosTransactions::Coordinator_ptr tx, const CosTransactions::otid_t &, CORBA::Boolean success) throw(CORBA::SystemException);

	virtual XA::ThreadModel thread_model() throw(CORBA::SystemException);

	virtual CORBA::Long rmid() throw(CORBA::SystemException);

	virtual void translateOtidToXid(const CosTransactions::otid_t& otid, XID& aXid);

private:
	LocalResourceManager& m_localResourceManager;
};
#endif
