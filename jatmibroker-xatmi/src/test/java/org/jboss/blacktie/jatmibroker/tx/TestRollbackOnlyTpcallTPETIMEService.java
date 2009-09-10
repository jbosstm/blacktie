package org.jboss.blacktie.jatmibroker.tx;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.xatmi.BlacktieService;
import org.jboss.blacktie.jatmibroker.xatmi.Buffer;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;

public class TestRollbackOnlyTpcallTPETIMEService implements BlacktieService {
	private static final Logger log = LogManager
			.getLogger(TestRollbackOnlyTpcallTPETIMEService.class);

	public Response tpservice(TPSVCINFO svcinfo) {
		try {
			int timeout = 11;
			log.info("test_tpcall_TPETIME_service, sleeping for %d seconds"
					+ timeout);
			Thread.sleep(timeout * 1000);
			log.info("test_tpcall_TPETIME_service, slept for %d seconds"
					+ timeout);

			int len = 60;
			Buffer toReturn = new Buffer("X_OCTET", null);
			toReturn.setData("test_tpcall_TPETIME_service".getBytes());
			return new Response(Connection.TPSUCCESS, 0, toReturn, len, 0);
		} catch (ConnectionException e) {
			return new Response(Connection.TPFAIL, 0, null, 0, 0);
		} catch (InterruptedException e) {
			return new Response(Connection.TPFAIL, 0, null, 0, 0);
		}
	}
}
