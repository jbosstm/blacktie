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
package org.jboss.blacktie.jatmibroker.tx;

import org.omg.CORBA.LocalObject;
import org.omg.IOP.Codec;
import org.omg.IOP.ENCODING_CDR_ENCAPS;
import org.omg.IOP.Encoding;
import org.omg.PortableInterceptor.ORBInitInfo;
import org.omg.PortableInterceptor.ORBInitializer;

public class TxIORInterceptorInitializer extends LocalObject implements
		ORBInitializer {
	public TxIORInterceptorInitializer() {
	}

	public void pre_init(ORBInitInfo info) {
	}

	public void post_init(ORBInitInfo info) {
		try {
			// Use CDR encapsulation with GIOP 1.2 encoding
			Encoding encoding = new Encoding(ENCODING_CDR_ENCAPS.value,
					(byte) 1, /* GIOP version */
					(byte) 2 /* GIOP revision */);
			Codec codec = info.codec_factory().create_codec(encoding);
			info.add_ior_interceptor(new TxIORInterceptor(codec));
		} catch (Exception e) {
			throw new RuntimeException("IOR initializer error", e);
		}
	}
}
