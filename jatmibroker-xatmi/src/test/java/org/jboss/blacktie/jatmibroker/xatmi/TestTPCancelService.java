package org.jboss.blacktie.jatmibroker.xatmi;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

public class TestTPCancelService implements BlacktieService {
	private static final Logger log = LogManager
			.getLogger(TestTPCancelService.class);

	public Response tpservice(TPSVCINFO svcinfo) {
		log.info("testtpcancel_service");
		if ((svcinfo.getFlags() & Connection.TPNOREPLY) != Connection.TPNOREPLY) {
			int len = 21;
			X_OCTET toReturn;
			try {
				String received = new String(((X_OCTET) svcinfo.getBuffer())
						.getByteArray());
				if (received.equals("cancel")) {
					toReturn = (X_OCTET) svcinfo.tpalloc("X_OCTET", null);
					toReturn.setByteArray("testtpcancel_service".getBytes());
					return new Response(Connection.TPSUCCESS, 0, toReturn, len,
							0);
				} else {
					return new Response(Connection.TPFAIL, 1, null, 0, 0);
				}
			} catch (ConnectionException e) {
				return new Response(Connection.TPFAIL, 2, null, 0, 0);
			}
		} else {
			return null;
		}
	}
}
