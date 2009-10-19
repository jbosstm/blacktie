package org.jboss.blacktie.jatmibroker.xatmi;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

public class TestTPGetRplyService implements BlacktieService {
	private static final Logger log = LogManager
			.getLogger(TestTPGetRplyService.class);

	public Response tpservice(TPSVCINFO svcinfo) {
		try {
			log.info("testtpgetrply_service");
			Buffer toReturn = svcinfo.tpalloc("X_OCTET", null);
			toReturn.setData("testtpgetrply_service".getBytes());
			return new Response(Connection.TPSUCCESS, 0, toReturn, 22, 0);
		} catch (ConnectionException e) {
			return new Response(Connection.TPFAIL, 0, null, 22, 0);
		}
	}
}
