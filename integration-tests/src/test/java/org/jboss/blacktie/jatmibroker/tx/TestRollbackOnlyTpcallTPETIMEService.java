package org.jboss.blacktie.jatmibroker.tx;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.tx.services.RollbackOnlyTpcallTPETIMEService;
import org.jboss.blacktie.jatmibroker.xatmi.BlacktieService;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;
import org.jboss.blacktie.jatmibroker.xatmi.X_OCTET;

public class TestRollbackOnlyTpcallTPETIMEService implements BlacktieService {
	private static final Logger log = LogManager
			.getLogger(RollbackOnlyTpcallTPETIMEService.class);

	public Response tpservice(TPSVCINFO svcinfo) {
		try {
			int timeout = 21;
			log.info("test_tpcall_TPETIME_service, sleeping for " + timeout
					+ " seconds");
			Thread.sleep(timeout * 1000);
			log.info("test_tpcall_TPETIME_service, slept for " + timeout
					+ " seconds");

			int len = 60;
			X_OCTET toReturn = (X_OCTET) svcinfo.tpalloc("X_OCTET", null);
			toReturn.setByteArray("test_tpcall_TPETIME_service".getBytes());
			return new Response(Connection.TPSUCCESS, 0, toReturn, len, 0);
		} catch (ConnectionException e) {
			return new Response(Connection.TPFAIL, 0, null, 0, 0);
		} catch (InterruptedException e) {
			return new Response(Connection.TPFAIL, 0, null, 0, 0);
		}
	}
}
