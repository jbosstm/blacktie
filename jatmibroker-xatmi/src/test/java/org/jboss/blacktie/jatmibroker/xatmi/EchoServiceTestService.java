package org.jboss.blacktie.jatmibroker.xatmi;

public class EchoServiceTestService implements BlacktieService {
	public Response tpservice(TPSVCINFO svcinfo) {
		byte[] toSend = svcinfo.getBuffer().getData();
		Buffer toReturn = new Buffer(null, null);
		toReturn.setData(toSend);
		return new Response((short) 0, 0, toReturn, toSend.length, 0);
	}
}
