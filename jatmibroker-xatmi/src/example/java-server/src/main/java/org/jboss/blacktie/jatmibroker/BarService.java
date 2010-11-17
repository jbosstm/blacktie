package org.jboss.blacktie.jatmibroker;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.Service;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;
import org.jboss.blacktie.jatmibroker.xatmi.X_OCTET;

public class BarService implements Service {
	private static final Logger log = LogManager.getLogger(BarService.class);

	public Response tpservice(TPSVCINFO svcinfo) throws ConnectionException,
			ConfigurationException {
		int sendlen;

		log.info("bar called  - svc: %s data %s len: %d flags: %d"
				+ svcinfo.getName() + " " + svcinfo.getBuffer() + " "
				+ svcinfo.getLen() + " " + svcinfo.getFlags());

		sendlen = 15;
		X_OCTET buffer = (X_OCTET) svcinfo.getConnection().tpalloc("X_OCTET",
				null, sendlen);
		buffer.setByteArray("BAR SAYS HELLO".getBytes());

		return new Response(Connection.TPSUCCESS, 0, buffer, 0);
	}
}
