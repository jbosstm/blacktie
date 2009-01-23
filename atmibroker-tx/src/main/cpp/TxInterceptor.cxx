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
#include "TxInterceptor.h"

#include "ThreadLocalStorage.h"

LoggerPtr atmiTxInterceptorLogger(Logger::getLogger("TxInterceptor"));

TxInterceptor::TxInterceptor(CORBA::ORB_ptr* orbPtr, IOP::CodecFactory_var cf, const char* name) : name_(name), orb(orbPtr)
{
        // create a GIOP 1.2 CDR encapsulation Codec (to match whatever is in jacorb.properties)
        IOP::Encoding encoding;

        encoding.format = IOP::ENCODING_CDR_ENCAPS;
        encoding.major_version = 1;
        encoding.minor_version = 2;

        this->codec_ = cf->create_codec (encoding);
}

char* TxInterceptor::name()
{
        return CORBA::string_dup(this->name_);
}

/**
 * convert an object ref to an object
 * caller is responsible for releasing the returned pointer
 */
CosTransactions::Control_ptr TxInterceptor::string_to_control(const char* ior)
{
	if (ior != NULL) {
        	try {
			CORBA::Object_ptr obj = get_orb()->string_to_object(ior);

                	if (!CORBA::is_nil(obj)) {
                		CosTransactions::Control_ptr cptr = CosTransactions::Control::_narrow(obj);
				CORBA::release(obj);	// dispose of it now that we have narrowed the object reference

				LOG4CXX_LOGLS(atmiTxInterceptorLogger, Level::getDebug(), (char *) "narrowed to " << cptr);

				return cptr;
			}
        	} catch (const CORBA::SystemException& ex) {
                	// the ior doesn't correspond to a transaction
                	ex._tao_print_exception("ior does not represent an OTS transaction");
        	}
	}

	return CosTransactions::Control::_nil();
}

/**
 * test whether the current thread contains a transaction and if so
 * returns its IOR
 */
char* TxInterceptor::get_control_ior()
{
	CosTransactions::Control_ptr ctrl = this->current_control();

        return CORBA::is_nil(ctrl) ? NULL : this->get_orb()->object_to_string(ctrl);
}

/**
 * return the current transaction associated with this thread
 */
CosTransactions::Control_ptr TxInterceptor::current_control()
{
	CosTransactions::Current_var& cur = AtmiBrokerOTS::get_instance()->getCurrent();

	if (CORBA::is_nil(cur))
		return NULL;

	return cur->get_control();
}

bool TxInterceptor::isTransactional(const char* op)
{
        return true;
}
