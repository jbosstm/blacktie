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
			X_COMMON dptr = (X_COMMON) svcinfo.getBuffer();

			if (dptr.getLong("acct_no") == 12345678
					&& dptr.getShort("amount") == 50) {
				ok = true;
			}

			int len = 60;
			X_OCTET toReturn = (X_OCTET) svcinfo.tpalloc("X_OCTET", null);
			if (ok) {
				toReturn.setByteArray("tpcall_x_common".getBytes());
			} else {
				toReturn.setByteArray("fail".getBytes());
			}
			return new Response(Connection.TPSUCCESS, 22, toReturn, len, 0);
		} catch (ConnectionException e) {
			return new Response(Connection.TPFAIL, 0, null, 0, 0);
		}
	}
}
