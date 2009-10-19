package org.jboss.blacktie.jatmibroker.xatmi;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

public class TestSpecExampleOneService implements BlacktieService {
	private static final Logger log = LogManager
			.getLogger(TestSpecExampleOneService.class);

	public Response tpservice(TPSVCINFO svcinfo) {
		log.info("debit_credit_svc");
		short rval;
		/* extract request typed buffer */
		Buffer dc_ptr = svcinfo.getBuffer();
		/*
		 * Depending on service name used to invoke this routine, perform either
		 * debit or credit work.
		 */
		if (!svcinfo.getName().equals("DEBIT")) {
			/*
			 * Parse input data and perform debit as part of global transaction.
			 */
		} else {
			/*
			 * Parse input data and perform credit as part of global
			 * transaction.
			 */
		}
		// TODO MAKE TWO TESTS
		try {
			if (dc_ptr.getInt("failTest") == 0) {
				rval = Connection.TPSUCCESS;
				dc_ptr.setInt("output", TestSpecExampleOne.OK);
			} else {
				rval = Connection.TPFAIL; /* global transaction will not commit */
				dc_ptr.setInt("output", TestSpecExampleOne.NOT_OK);
			}
		} catch (ConnectionException e) {
			return new Response(Connection.TPFAIL, Connection.TPEITYPE, dc_ptr,
					dc_ptr.getLength(), 0);
		}
		/* send reply and return from service routine */
		return new Response(rval, 0, dc_ptr, 0, 0);
	}
}
