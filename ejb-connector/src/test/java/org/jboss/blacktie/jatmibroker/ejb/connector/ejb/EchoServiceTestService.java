package org.jboss.blacktie.jatmibroker.ejb.connector.ejb;

import org.jboss.blacktie.jatmibroker.xatmi.connector.Response;
import org.jboss.blacktie.jatmibroker.xatmi.connector.TPSVCINFO;
import org.jboss.blacktie.jatmibroker.xatmi.connector.buffers.Buffer;
import org.jboss.blacktie.jatmibroker.xatmi.connector.buffers.X_OCTET;
import org.jboss.blacktie.jatmibroker.xatmi.ejb.AbstractBlacktieService;

public class EchoServiceTestService extends AbstractBlacktieService {

	public EchoServiceTestService() {
	}

	public Response tpservice(TPSVCINFO svcinfo) {
		Buffer data = svcinfo.getData();
		Buffer buffer = new X_OCTET(data.getSize());
		buffer.setData(data.getData());
		Response response = new Response((short) 0, 0, buffer, 0);
		return response;
	}

}