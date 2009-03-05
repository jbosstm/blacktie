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
#ifndef OTS_POLICY_FACTORY_H
#define OTS_POLICY_FACTORY_H

#include "ace/config-all.h"

#include "tao/PI/PI.h"
#include "tao/LocalObject.h"

#include "OTSPolicy.h"

/**
 * Policy Factory for OTSPolicy
 */
class OTSPolicyFactory :
        public virtual PortableInterceptor::PolicyFactory,
        public virtual ::CORBA::LocalObject
{
public:
        virtual CORBA::Policy_ptr create_policy (CORBA::PolicyType, const CORBA::Any &);
};
#endif  // OTS_POLICY_FACTORY_H 
