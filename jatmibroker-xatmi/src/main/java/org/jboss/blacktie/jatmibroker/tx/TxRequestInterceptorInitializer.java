package org.jboss.blacktie.jatmibroker.tx;

import org.omg.CORBA.LocalObject;
import org.omg.IOP.Codec;
import org.omg.IOP.ENCODING_CDR_ENCAPS;
import org.omg.IOP.Encoding;
import org.omg.PortableInterceptor.ORBInitInfo;
import org.omg.PortableInterceptor.ORBInitializer;

public class TxRequestInterceptorInitializer extends LocalObject implements
		ORBInitializer {
	public TxRequestInterceptorInitializer() {
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
					"TxClientRequestInterceptor", codec));
			info.add_server_request_interceptor(new TxRequestInterceptor(
					"TxServerRequestInterceptor", codec));
		} catch (org.omg.IOP.CodecFactoryPackage.UnknownEncoding e) {
			throw new RuntimeException(
					"Request initializer error - unknown encoding: ", e);
		} catch (org.omg.PortableInterceptor.ORBInitInfoPackage.DuplicateName e) {
			throw new RuntimeException(
					"Request initializer error - duplicate interceptor: ", e);
		}
	}
}
