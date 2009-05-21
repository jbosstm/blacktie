package org.jboss.blacktie.jatmibroker.ejb.connector.ejb;

import org.jboss.blacktie.jatmibroker.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.mdb.MDBBlacktieService;
import org.jboss.blacktie.jatmibroker.xatmi.Buffer;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;

public class EchoServiceTestService extends MDBBlacktieService {

	public EchoServiceTestService() throws ConfigurationException {
	}

	public Response tpservice(TPSVCINFO svcinfo) {
		byte[] rcvd = svcinfo.getBuffer().getData();
		Buffer buffer = new Buffer(null, null);
		buffer.setData(rcvd);
		return new Response((short) 0, 0, buffer, rcvd.length, 0);
	}
}