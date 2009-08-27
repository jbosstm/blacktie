package org.jboss.blacktie.jatmibroker.xatmi;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

public class TestTPACallService implements BlacktieService {
	private static final Logger log = LogManager
			.getLogger(TestTPACallService.class);

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
