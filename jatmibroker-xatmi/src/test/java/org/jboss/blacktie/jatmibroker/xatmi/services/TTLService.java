package org.jboss.blacktie.jatmibroker.xatmi.services;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.Service;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;
import org.jboss.blacktie.jatmibroker.xatmi.X_OCTET;

public class TTLService implements Service {
	private static final Logger log = LogManager.getLogger(TTLService.class);
	private static int n = 0;

	public Response tpservice(TPSVCINFO svcinfo) {
		log.info("test_ttl_service");

		try {
			int timeout = 45;
			X_OCTET dptr = (X_OCTET) svcinfo.getBuffer();
			String data = new String(dptr.getByteArray());
			log.info("test_ttl_service get data: " + data);

			int len = 60;
			X_OCTET toReturn = (X_OCTET) svcinfo.tpalloc("X_OCTET", null);

			if (data.contains("counter")) {
				String counter = String.valueOf(n);
				toReturn.setByteArray(counter.getBytes());
				len = counter.length();
			} else {
				n++;
				try {
					log.info("test_ttl_service sleep for " + timeout
							+ " seconds");
					Thread.sleep(timeout * 1000);
					log.info("test_ttl_service slept for " + timeout
							+ " seconds");
					toReturn.setByteArray("test_ttl_service".getBytes());
				} catch (Exception e) {
					log.error("sleep failed with " + e);
				}
			}
			return new Response(Connection.TPSUCCESS, 22, toReturn, len, 0);
		} catch (ConnectionException e) {
			return new Response(Connection.TPFAIL, 0, null, 0, 0);
		}
	}
}
