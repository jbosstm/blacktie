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
#include "TxIORInterceptor.h"

#include "tao/OctetSeqC.h"
#include "tao/LocalObject.h"
#include "tao/PI/ClientRequestInfo.h"

#include <iostream>

LoggerPtr loggerTxIORInterceptor(Logger::getLogger("ATMITxIORInterceptor"));

TxIORInterceptor::TxIORInterceptor(CORBA::ORB_ptr* orbPtr, IOP::CodecFactory_var cf) :
	TxInterceptor(orbPtr, cf, "ATMITxIORInterceptor")
{
}

/**
 * tag the input IORInfo as transactional 
 */
void TxIORInterceptor::addOTSTag(PortableInterceptor::IORInfo_ptr info)
{
        LOG4CXX_LOGLS(loggerTxIORInterceptor, Level::getDebug(), (char*) "addOTSTag: ");

        IOP::TaggedComponent comp;	// create an IOR tag
        comp.tag = AtmiTx::TAG_OTS_POLICY;

        // we want to encode a tranaction policy into the tag
        CORBA::Any policy_val;
        policy_val <<= AtmiTx::ADAPTS;
        CORBA::OctetSeq_var ev = this->codec_->encode_value(policy_val);

        comp.component_data.replace(ev->maximum(),	// max
                                   ev->length(),	// len
                                   ev->get_buffer(),	// buffer
                                   0);			// no release

        try {
                // Add the tagged component to all profiles.
                info->add_ior_component (comp);
                info->add_ior_component_to_profile (comp, IOP::TAG_INTERNET_IOP);
        } catch (CORBA::SystemException& ex) {
                ex._tao_print_exception ("TxIORInterceptor add tag: ");
        }
}

/**
 * if the target object type is transactional then add an IOR tag to its profile
 * to mark it as transactional
 * Request processing interceptors may then inspect an IOR to decide whether to apply
 * implicit transaction propagation.
 */
void TxIORInterceptor::establish_components(PortableInterceptor::IORInfo_ptr info)
{
        try {
                PortableInterceptor::ObjectReferenceTemplate_var at = info->adapter_template();
                PortableInterceptor::AdapterName_var an = at->adapter_name ();

		// if an->length is 1 then this is the root POA:
		// do not apply the tag to the root POA
                if (an->length () > 1) {
                        LOG4CXX_LOGLS(loggerTxIORInterceptor, Level::getDebug(), (char*) "establish_components: ");

                        CORBA::Policy_var policy(info->get_effective_policy(AtmiTx::OTS_POLICY_TYPE));
                        AtmiTx::OTSPolicy_var txpolicy(AtmiTx::OTSPolicy::_narrow(policy.in()));

                        addOTSTag(info);
                }
        } catch (CORBA::SystemException& ex) {
		// should be 'IDL:omg.org/CORBA/INV_POLICY:1.0' with minor code (3) to indicate that
		// the target IOR is not transactional
                //TODO check minor code and print a warning if appropriate
                //ex._tao_print_exception ("TxIORInterceptor exception: ");
                ex.completed( CORBA::COMPLETED_YES );
        }
}
