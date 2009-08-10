package org.jboss.blacktie.jatmibroker.xatmi;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

public class TestSpecExampleTwoService implements BlacktieService {
	private static final Logger log = LogManager
			.getLogger(TestSpecExampleTwoService.class);

	public Response tpservice(TPSVCINFO svcinfo) {
		return null;
	}
}
