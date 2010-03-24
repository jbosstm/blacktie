package org.jboss.blacktie.jatmibroker.xatmi;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

public class TestTPSendTPSendOnlyService implements BlacktieService {
	private static final Logger log = LogManager
			.getLogger(TestTPSendTPSendOnlyService.class);

	public Response tpservice(TPSVCINFO svcinfo) {
		log.info("testtpsend_tpsendonly_service");
		try {
			int result = svcinfo.getSession().tpsend(svcinfo.getBuffer(),
					svcinfo.getBuffer().getRawData().length,
					Connection.TPRECVONLY);
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
