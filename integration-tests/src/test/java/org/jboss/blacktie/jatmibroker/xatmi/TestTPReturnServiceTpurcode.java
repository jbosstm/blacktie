package org.jboss.blacktie.jatmibroker.xatmi;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

public class TestTPReturnServiceTpurcode implements BlacktieService {
	private static final Logger log = LogManager
			.getLogger(TestTPReturnServiceTpurcode.class);

	public Response tpservice(TPSVCINFO svcinfo) {
		log.info("testtpreturn_service_tpurcode");
		int len = 0;
		try {
			Buffer toReturn = new X_OCTET();
			if (TestTPConversation.strcmp(svcinfo.getBuffer(), "24") == 0) {
				return new Response(Connection.TPSUCCESS, 24, toReturn, len, 0);
			} else {
				return new Response(Connection.TPSUCCESS, 77, toReturn, len, 0);
			}
		} catch (ConnectionException e) {
			return new Response(Connection.TPFAIL, 0, null, 0, 0);
		}
	}
}
