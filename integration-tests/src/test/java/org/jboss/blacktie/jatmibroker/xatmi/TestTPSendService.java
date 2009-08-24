package org.jboss.blacktie.jatmibroker.xatmi;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

public class TestTPSendService implements BlacktieService {
	private static final Logger log = LogManager
			.getLogger(TestTPSendService.class);

	public Response tpservice(TPSVCINFO svcinfo) {
		log.info("testtpsend_service");
		return null;
	}
}
