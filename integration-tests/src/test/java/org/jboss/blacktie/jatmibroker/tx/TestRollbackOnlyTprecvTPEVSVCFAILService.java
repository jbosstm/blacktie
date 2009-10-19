package org.jboss.blacktie.jatmibroker.tx;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.xatmi.BlacktieService;
import org.jboss.blacktie.jatmibroker.xatmi.Buffer;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;

public class TestRollbackOnlyTprecvTPEVSVCFAILService implements
		BlacktieService {
	private static final Logger log = LogManager
			.getLogger(TestRollbackOnlyTprecvTPEVSVCFAILService.class);

	public Response tpservice(TPSVCINFO svcinfo) {
		try {
			log.info("test_tprecv_TPEV_SVCFAIL_service");
			int len = 60;
			Buffer toReturn = svcinfo.tpalloc("X_OCTET", null);
			toReturn.setData("test_tprecv_TPEV_SVCFAIL_service".getBytes());
			return new Response(Connection.TPFAIL, 0, toReturn, len, 0);
		} catch (ConnectionException e) {
			return new Response(Connection.TPFAIL, Connection.TPEITYPE, null,
					0, 0);
		}
	}
}
