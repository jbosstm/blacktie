package org.jboss.blacktie.jatmibroker.xatmi.services;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.xatmi.BlacktieService;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;

public class TPReturnOpenSession2Service implements BlacktieService {
	private static final Logger log = LogManager
			.getLogger(TPReturnOpenSession2Service.class);

	public Response tpservice(TPSVCINFO svcinfo) {
		log.info("testtpreturn_service_opensession2");
		return new Response(Connection.TPSUCCESS, 0, svcinfo.getBuffer(),
				svcinfo.getLen(), 0);
	}
}
