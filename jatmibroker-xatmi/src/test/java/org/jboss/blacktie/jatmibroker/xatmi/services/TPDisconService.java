package org.jboss.blacktie.jatmibroker.xatmi.services;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.xatmi.Service;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;

public class TPDisconService implements Service {
	private static final Logger log = LogManager
			.getLogger(TPDisconService.class);

	public Response tpservice(TPSVCINFO svcinfo) {
		log.info("testtpdiscon_service");
		try {
			Thread.sleep(2000);
		} catch (InterruptedException e) {
			log.error("Was interrupted");
		}
		return null;
	}
}
