package org.jboss.blacktie.jatmibroker.xatmi;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

public class TestTPConnectService implements BlacktieService {
	private static final Logger log = LogManager
			.getLogger(TestTPConnectService.class);

	public Response tpservice(TPSVCINFO svcinfo) {
		log.info("testtpconnect_service");
		return new Response(Connection.TPSUCCESS, 0, null, 0, 0);
	}
}
