package org.jboss.blacktie.jatmibroker.tx.services;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.Service;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;
import org.jboss.blacktie.jatmibroker.xatmi.X_OCTET;

public class RollbackOnlyTpcallTPESVCFAILService implements Service {
	private static final Logger log = LogManager
			.getLogger(RollbackOnlyTpcallTPESVCFAILService.class);

	public Response tpservice(TPSVCINFO svcinfo) throws ConnectionException {
		log.info("test_tpcall_TPESVCFAIL_service");
		int len = 60;
		X_OCTET toReturn = (X_OCTET) svcinfo.getConnection().tpalloc("X_OCTET",
				null, len);
		toReturn.setByteArray("test_tpcall_TPESVCFAIL_service".getBytes());
		return new Response(Connection.TPFAIL, 0, toReturn, 0);
	}
}
