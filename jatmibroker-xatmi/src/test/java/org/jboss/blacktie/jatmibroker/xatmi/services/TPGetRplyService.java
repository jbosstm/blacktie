package org.jboss.blacktie.jatmibroker.xatmi.services;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.xatmi.BlacktieService;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;
import org.jboss.blacktie.jatmibroker.xatmi.X_OCTET;

public class TPGetRplyService implements BlacktieService {
	private static final Logger log = LogManager
			.getLogger(TPGetRplyService.class);

	public Response tpservice(TPSVCINFO svcinfo) {
		try {
			log.info("testtpgetrply_service");
			X_OCTET toReturn = (X_OCTET) svcinfo.tpalloc("X_OCTET", null);
			toReturn.setByteArray("testtpgetrply_service".getBytes());
			return new Response(Connection.TPSUCCESS, 0, toReturn, 22, 0);
		} catch (ConnectionException e) {
			return new Response(Connection.TPFAIL, 0, null, 22, 0);
		}
	}
}
