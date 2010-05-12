package org.jboss.blacktie.jatmibroker.xatmi.services;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.Service;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;
import org.jboss.blacktie.jatmibroker.xatmi.X_OCTET;

public class TPCancelService implements Service {
	private static final Logger log = LogManager
			.getLogger(TPCancelService.class);

	public Response tpservice(TPSVCINFO svcinfo) throws ConnectionException {
		log.info("testtpcancel_service");
		if ((svcinfo.getFlags() & Connection.TPNOREPLY) != Connection.TPNOREPLY) {
			int len = 21;
			X_OCTET toReturn;
			String received = new String(((X_OCTET) svcinfo.getBuffer())
					.getByteArray());
			if (received.equals("cancel")) {
				toReturn = (X_OCTET) svcinfo.getConnection().tpalloc("X_OCTET",
						null);
				toReturn.setByteArray("testtpcancel_service".getBytes());
				return new Response(Connection.TPSUCCESS, 0, toReturn, len, 0);
			} else {
				return new Response(Connection.TPFAIL, 1, null, 0, 0);
			}
		} else {
			return null;
		}
	}
}
