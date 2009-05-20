package org.jboss.blacktie.jatmibroker.ejb.connector.ejb;

import org.jboss.blacktie.jatmibroker.JAtmiBrokerException;
import org.jboss.blacktie.jatmibroker.mdb.BlacktieMDBService;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;

public class EchoServiceTestService extends BlacktieMDBService {

	public EchoServiceTestService() throws JAtmiBrokerException {
	}

	public Response tpservice(TPSVCINFO svcinfo) {
		return new Response((short) 0, 0, svcinfo.getData(), svcinfo.getLen(),
				0);
	}

}