package org.jboss.blacktie.jatmibroker.xatmi;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

public class TestTPCallServiceXOctet implements BlacktieService {
	private static final Logger log = LogManager
			.getLogger(TestTPCallServiceXOctet.class);

	public Response tpservice(TPSVCINFO svcinfo) {
		log.info("test_tpcall_x_octet_service");
		boolean ok = false;
		if (svcinfo.getBuffer() != null) {
			if (new String(svcinfo.getBuffer().getData())
					.equals("test_tpcall_x_octet")) {
				ok = true;
			}
		}

		int len = 60;
		Buffer toReturn;
		try {
			toReturn = new Buffer("X_OCTET", null);
			if (ok) {
				toReturn.setData("tpcall_x_octet".getBytes());
			} else {
				StringBuffer buffer = new StringBuffer("fail");
				if (svcinfo.getBuffer() != null) {
					buffer.append(new String(svcinfo.getBuffer().getData()));
				} else {
					buffer.append("dud");
				}
				toReturn.setData("fail".getBytes());
			}
			return new Response(Connection.TPSUCCESS, 20, toReturn, len, 0);
		} catch (ConnectionException e) {
			return new Response(Connection.TPFAIL, Connection.TPEOTYPE, null,
					0, 0);
		}
	}
}
