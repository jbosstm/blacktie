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
#include "OTSPolicyFactory.h"

CORBA::Policy_ptr OTSPolicyFactory::create_policy(CORBA::PolicyType type, const CORBA::Any & value)
{
		if (type != AtmiTx::OTS_POLICY_TYPE)
				throw CORBA::PolicyError(CORBA::BAD_POLICY_TYPE);

		CORBA::UShort val;

		if (!(value >>= val))
				throw CORBA::INTERNAL();

		OTSPolicy* p = 0;
		ACE_NEW_THROW_EX(p, OTSPolicy(val), CORBA::NO_MEMORY());

		return p;
}
