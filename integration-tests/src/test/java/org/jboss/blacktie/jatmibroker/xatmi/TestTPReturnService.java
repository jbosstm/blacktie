package org.jboss.blacktie.jatmibroker.xatmi;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

public class TestTPReturnService implements BlacktieService {
	private static final Logger log = LogManager
			.getLogger(TestTPReturnService.class);

	public Response tpservice(TPSVCINFO svcinfo) {
		log.info("testtpreturn_service");
		throw new RuntimeException("tpreturn exception");
	}
}
