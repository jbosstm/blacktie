package org.jboss.blacktie.jatmibroker.ejb.connector.ejb;

import org.jboss.blacktie.jatmibroker.xatmi.ConnectorException;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;
import org.jboss.blacktie.jatmibroker.xatmi.buffers.Buffer;
import org.jboss.blacktie.jatmibroker.xatmi.buffers.X_OCTET;
import org.jboss.blacktie.jatmibroker.xatmi.ejb.BlacktieMDBService;

public class EchoServiceTestService extends BlacktieMDBService {

	public EchoServiceTestService() throws ConnectorException {
	}

	public Response tpservice(TPSVCINFO svcinfo) {
		Buffer data = svcinfo.getData();
		Buffer buffer = new X_OCTET(data.getSize());
		buffer.setData(data.getData());
		Response response = new Response((short) 0, 0, buffer, 0);
		return response;
	}

}