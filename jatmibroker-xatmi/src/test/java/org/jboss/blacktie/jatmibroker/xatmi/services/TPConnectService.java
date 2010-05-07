package org.jboss.blacktie.jatmibroker.xatmi.services;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.xatmi.BlacktieService;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;

public class TPConnectService implements BlacktieService {
	private static final Logger log = LogManager
			.getLogger(TPConnectService.class);

	public Response tpservice(TPSVCINFO svcinfo) {
		log.info("testtpconnect_service");
		return new Response(Connection.TPSUCCESS, 0, null, 0, 0);
	}
}
