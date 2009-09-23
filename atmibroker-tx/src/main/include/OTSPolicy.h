/*
 * JBoss, Home of Professional Open Source
 * Copyright 2009, Red Hat, Inc., and others contributors as indicated
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
#ifndef OTS_POLICY_H
#define OTS_POLICY_H

#include "TxPolicyC.h"
#include "tao/LocalObject.h"

/**
 * simulate CosTransactions OTSPolicy
 */
class OTSPolicy :
	public virtual AtmiTx::OTSPolicy,
	public virtual ::CORBA::LocalObject
{
public:
	OTSPolicy(CORBA::UShort val);

	virtual CORBA::UShort tpv(void);
	virtual CORBA::PolicyType policy_type(void);
	virtual CORBA::Policy_ptr copy(void);
	virtual void destroy(void);

protected:
	~OTSPolicy(void);

private:
	const CORBA::UShort tpv_;
};

#endif // OTS_POLICY_H
