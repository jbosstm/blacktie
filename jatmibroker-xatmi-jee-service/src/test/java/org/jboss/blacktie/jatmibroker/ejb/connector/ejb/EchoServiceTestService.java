package org.jboss.blacktie.jatmibroker.ejb.connector.ejb;

import org.jboss.blacktie.jatmibroker.JAtmiBrokerException;
import org.jboss.blacktie.jatmibroker.mdb.MDBBlacktieService;
import org.jboss.blacktie.jatmibroker.xatmi.Buffer;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;

public class EchoServiceTestService extends MDBBlacktieService {

	public EchoServiceTestService() throws JAtmiBrokerException {
	}

	public Response tpservice(TPSVCINFO svcinfo) {
		Buffer buffer = new Buffer(null, null, 0);
		buffer.setData(svcinfo.getBuffer().getData());
		buffer.setLen(svcinfo.getBuffer().getLen());
		return new Response((short) 0, 0, buffer, 0);
	}
}