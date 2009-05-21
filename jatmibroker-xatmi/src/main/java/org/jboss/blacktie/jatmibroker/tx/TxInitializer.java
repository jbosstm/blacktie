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

import org.jboss.blacktie.jatmibroker.transport.OrbManagement;
import org.omg.CORBA.LocalObject;
import org.omg.IOP.Codec;
import org.omg.IOP.ENCODING_CDR_ENCAPS;
import org.omg.IOP.Encoding;
import org.omg.PortableInterceptor.ORBInitInfo;
import org.omg.PortableInterceptor.ORBInitializer;

public class TxInitializer extends LocalObject implements
		ORBInitializer {

	static TxInitializer instance;

	private OrbManagement orbManagement;

	private static TxInitializer get_instance()
	{
		if (instance == null)
			instance = new TxInitializer();

		return instance;
	}

	/**
	 * Call this method just before initializing an orb.
	 * Each interceptor created when the initializer runs will then have the orb
	 * injected. In this way each intereceptor will have a reference to the orb
	 * it is intercepting requests on behalf of.
	 */
	public static void setOrbManagement(OrbManagement orbManagement)
	{
		get_instance().orbManagement = orbManagement;
	}

	public TxInitializer() {
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
			info.add_client_request_interceptor(new TxRequestInterceptor(
				orbManagement, "TxClientRequestInterceptor", codec));
			info.add_server_request_interceptor(new TxRequestInterceptor(
				orbManagement, "TxServerRequestInterceptor", codec));
			info.add_ior_interceptor(new TxIORInterceptor(codec));
			info.register_policy_factory(TxIORInterceptor.OTS_POLICY_TYPE, new OTSPolicyFactory());
		} catch (org.omg.IOP.CodecFactoryPackage.UnknownEncoding e) {
			throw new RuntimeException(
					"Request initializer error - unknown encoding: ", e);
		} catch (org.omg.PortableInterceptor.ORBInitInfoPackage.DuplicateName e) {
			throw new RuntimeException(
					"Request initializer error - duplicate interceptor: ", e);
		}
	}
}

