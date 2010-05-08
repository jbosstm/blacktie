package org.jboss.blacktie.jatmibroker.xatmi.services;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.xatmi.Service;
import org.jboss.blacktie.jatmibroker.xatmi.Buffer;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;

public class TPSendTPSendOnlyService implements Service {
	private static final Logger log = LogManager
			.getLogger(TPSendTPSendOnlyService.class);

	public Response tpservice(TPSVCINFO svcinfo) {
		log.info("testtpsend_tpsendonly_service");
		try {
			int result = svcinfo.getSession().tpsend(svcinfo.getBuffer(),
					svcinfo.getLen(), Connection.TPRECVONLY);
		} catch (ConnectionException e) {
			log.error("ConnectionException: ", e);
		}
		try {
			Buffer tprecv = svcinfo.getSession().tprecv(0);
		} catch (ConnectionException e) {
			if (e.getTperrno() != Connection.TPEEVENT) {
				log.error("ConnectionException: ", e);
			}
		}
		return null;
	}
}
