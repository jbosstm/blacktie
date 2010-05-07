package org.jboss.blacktie.jatmibroker.xatmi;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

public class TestTPReturnServiceOpenSession2 implements BlacktieService {
	private static final Logger log = LogManager
			.getLogger(TestTPReturnServiceOpenSession2.class);

	public Response tpservice(TPSVCINFO svcinfo) {
		log.info("testtpreturn_service_opensession2");
		return new Response(Connection.TPSUCCESS, 0, svcinfo.getBuffer(),
				svcinfo.getLen(), 0);
	}
}
