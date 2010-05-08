package org.jboss.blacktie.jatmibroker.tx.services;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.Service;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;
import org.jboss.blacktie.jatmibroker.xatmi.X_COMMON;

public class RollbackOnlyTpcallTPEOTYPEService implements Service {
	private static final Logger log = LogManager
			.getLogger(RollbackOnlyTpcallTPEOTYPEService.class);

	public Response tpservice(TPSVCINFO svcinfo) {
		try {
			log.info("test_tpcall_TPEOTYPE_service");
			int len = 60;
			X_COMMON toReturn = (X_COMMON) svcinfo.getConnection().tpalloc(
					"X_COMMON", "test");
			toReturn.setByteArray("key", "test_tpcall_TPEOTYPE_service"
					.getBytes());
			return new Response(Connection.TPSUCCESS, 0, toReturn, len, 0);
		} catch (ConnectionException e) {
			return new Response(Connection.TPFAIL, 0, null, 0, 0);
		}
	}
}
