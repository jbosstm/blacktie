package org.jboss.blacktie.jatmibroker.xatmi;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

public class TestSpecExampleTwoService implements BlacktieService {
	private static final Logger log = LogManager
			.getLogger(TestSpecExampleTwoService.class);

	public Response tpservice(TPSVCINFO svcinfo) {
		try {
			log.info("inquiry_svc");
			short rval;
			/* extract initial typed buffer sent as part of tpconnect() */
			Buffer ptr = svcinfo.getBuffer();

			/*
			 * Parse input string, ptr->input, and retrieve records. Return 10
			 * records at a time to client. Records are placed in ptr->output,
			 * an array of account records.
			 */
			for (int i = 0; i < 5; i++) {
				/* gather from DBMS next 10 records into ptr->output array */
				svcinfo.getSession().tpsend(ptr, 0, Connection.TPSIGRSTRT);
			}
			// TODO DO OK AND FAIL
			if (ptr.getInt("failTest") == 0) {
				rval = Connection.TPSUCCESS;
			} else {
				rval = Connection.TPFAIL; /* global transaction will not commit */
			}
			/* terminate service routine, send no data, and */
			/* terminate connection */
			return new Response(rval, 0, null, 0, 0);
		} catch (ConnectionException e) {
			return new Response(Connection.TPFAIL, 0, null, 0, 0);
		}
	}
}
