package org.jboss.blacktie.jatmibroker.xatmi;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

public class TestTPRecvService implements BlacktieService {
	private static final Logger log = LogManager
			.getLogger(TestTPRecvService.class);

	public Response tpservice(TPSVCINFO svcinfo) {
		log.info("testtprecv_service");
		return null;
	}
}
