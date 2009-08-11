package org.jboss.blacktie.jatmibroker.xatmi;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

public class TestTPCallServiceXCommon implements BlacktieService {
	private static final Logger log = LogManager
			.getLogger(TestTPCallServiceXCommon.class);

	public Response tpservice(TPSVCINFO svcinfo) {
		try {
			log.info("test_tpcall_x_common_service");
			boolean ok = false;
			Buffer dptr = svcinfo.getBuffer();
			dptr.format(new String[] { "acct_no", "amount", "balance",
					"status", "status_len" }, new Class[] { long.class,
					short.class, short.class, char[].class, short.class },
					new int[] { 0, 0, 0, 128, 0 });

			if (dptr.getLong("acct_no") == 12345678
					&& dptr.getShort("amount") == 50) {
				ok = true;
			}

			int len = 60;
			Buffer toReturn = new Buffer("X_OCTET", null);
			if (ok) {
				toReturn.setData("tpcall_x_common".getBytes());
			} else {
				toReturn.setData("fail".getBytes());
			}
			return new Response(Connection.TPSUCCESS, 22, toReturn, len, 0);
		} catch (ConnectionException e) {
			return new Response(Connection.TPFAIL, 0, null, 0, 0);
		}
	}
}
