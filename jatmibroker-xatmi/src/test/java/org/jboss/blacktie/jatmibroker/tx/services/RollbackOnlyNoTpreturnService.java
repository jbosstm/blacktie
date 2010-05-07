package org.jboss.blacktie.jatmibroker.tx.services;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.xatmi.BlacktieService;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;

public class RollbackOnlyNoTpreturnService implements BlacktieService {
	private static final Logger log = LogManager
			.getLogger(RollbackOnlyNoTpreturnService.class);

	public Response tpservice(TPSVCINFO svcinfo) {
		log.info("test_no_tpreturn_service");
		return null;
	}
}
