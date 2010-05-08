package org.jboss.blacktie.jatmibroker.xatmi.services;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.xatmi.Service;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;

public class TPACallService implements Service {
	private static final Logger log = LogManager
			.getLogger(TPACallService.class);

	public Response tpservice(TPSVCINFO svcinfo) {
		log.info("testtpacall_service");
		// int len = 20;
		// char *toReturn = (char*) malloc(len);
		// strcpy(toReturn, "testtpacall_service");
		// tpreturn(TPSUCCESS, 0, toReturn, len, 0);
		// free(toReturn);
		throw new RuntimeException("Error simulated");
	}
}
