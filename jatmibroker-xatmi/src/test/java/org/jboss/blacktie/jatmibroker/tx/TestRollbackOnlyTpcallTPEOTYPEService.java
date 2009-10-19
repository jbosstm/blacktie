package org.jboss.blacktie.jatmibroker.tx;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.xatmi.BlacktieService;
import org.jboss.blacktie.jatmibroker.xatmi.Buffer;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.blacktie.jatmibroker.xatmi.R_PBF;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;

public class TestRollbackOnlyTpcallTPEOTYPEService implements BlacktieService {
	private static final Logger log = LogManager
			.getLogger(TestRollbackOnlyTpcallTPEOTYPEService.class);

	public Response tpservice(TPSVCINFO svcinfo) {
		try {
			log.info("test_tpcall_TPEOTYPE_service");
			int len = 60;
			Buffer toReturn = new R_PBF("test");
			toReturn.format(new String[] { "key" },
					new Class[] { char[].class }, new int[] { 28 });
			toReturn.setCharArray("key", "test_tpcall_TPEOTYPE_service"
					.toCharArray());
			return new Response(Connection.TPSUCCESS, 0, toReturn, len, 0);
		} catch (ConnectionException e) {
			return new Response(Connection.TPFAIL, 0, null, 0, 0);
		}
	}
}
