package org.jboss.blacktie.jatmibroker.xatmi;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

public class TestTPGetRplyTwoService implements BlacktieService {
	private static final Logger log = LogManager
			.getLogger(TestTPGetRplyTwoService.class);

	public Response tpservice(TPSVCINFO svcinfo) {
		try {
			String response = "test_tpgetrply_TPGETANY_two";
			log.info(response);

			X_OCTET toReturn = (X_OCTET) svcinfo.tpalloc("X_OCTET", null);
			toReturn.setByteArray(response.getBytes());
			Thread.sleep(5 * 1000);
			return new Response(Connection.TPSUCCESS, 0, toReturn, response
					.getBytes().length, 0);
		} catch (Throwable t) {
			return new Response(Connection.TPFAIL, 0, null, 22, 0);
		}
	}
}