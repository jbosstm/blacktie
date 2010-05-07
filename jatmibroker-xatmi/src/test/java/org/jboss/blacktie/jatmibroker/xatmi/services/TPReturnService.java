package org.jboss.blacktie.jatmibroker.xatmi.services;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.xatmi.BlacktieService;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;

public class TPReturnService implements BlacktieService {
	private static final Logger log = LogManager
			.getLogger(TPReturnService.class);

	public Response tpservice(TPSVCINFO svcinfo) {
		log.info("testtpreturn_service");
		throw new RuntimeException("tpreturn exception");
	}
}
