package org.jboss.blacktie.jatmibroker.xatmi;

import org.jboss.blacktie.jatmibroker.xatmi.buffers.Buffer;
import org.jboss.blacktie.jatmibroker.xatmi.buffers.X_OCTET;

public class EchoServiceTestService implements BlacktieService {
	public Response tpservice(TPSVCINFO svcinfo) {
		Buffer data = svcinfo.getData();
		Buffer buffer = new X_OCTET(data.getSize());
		buffer.setData(data.getData());
		Response response = new Response((short) 0, 0, buffer, 0);
		return response;
	}
}
