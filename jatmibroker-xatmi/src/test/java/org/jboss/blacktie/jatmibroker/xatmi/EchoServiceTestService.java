package org.jboss.blacktie.jatmibroker.xatmi;

public class EchoServiceTestService implements BlacktieService {
	public Response tpservice(TPSVCINFO svcinfo) {
		return new Response((short) 0, 0, svcinfo.getData(), svcinfo.getLen(),
				0);
	}
}
