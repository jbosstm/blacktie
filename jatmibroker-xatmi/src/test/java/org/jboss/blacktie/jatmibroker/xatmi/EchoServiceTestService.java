package org.jboss.blacktie.jatmibroker.xatmi;

public class EchoServiceTestService implements BlacktieService {
	public Response tpservice(TPSVCINFO svcinfo) {
		Buffer toReturn = new Buffer(null, null, svcinfo.getBuffer().getLen());
		toReturn.setData(svcinfo.getBuffer().getData());
		return new Response((short) 0, 0, toReturn, 0);
	}
}
