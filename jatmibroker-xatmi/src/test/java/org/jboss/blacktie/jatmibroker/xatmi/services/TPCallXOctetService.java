package org.jboss.blacktie.jatmibroker.xatmi.services;

import java.util.Arrays;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.xatmi.Service;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;
import org.jboss.blacktie.jatmibroker.xatmi.X_OCTET;

public class TPCallXOctetService implements Service {
	private static final Logger log = LogManager
			.getLogger(TPCallXOctetService.class);

	public Response tpservice(TPSVCINFO svcinfo) {
		log.info("test_tpcall_x_octet_service");
		boolean ok = false;
		if (svcinfo.getBuffer() != null) {
			byte[] received = ((X_OCTET) svcinfo.getBuffer()).getByteArray();
			byte[] expected = new byte["test_tpcall_x_octet".getBytes().length + 1];
			System.arraycopy("test_tpcall_x_octet".getBytes(), 0, expected, 0,
					received.length - 1);
			if (Arrays.equals(received, expected)) {
				ok = true;
			}
		}

		int len = 60;
		X_OCTET toReturn;
		try {
			toReturn = (X_OCTET) svcinfo.tpalloc("X_OCTET", null);
			if (ok) {
				toReturn.setByteArray("tpcall_x_octet".getBytes());
			} else {
				StringBuffer buffer = new StringBuffer("fail");
				if (svcinfo.getBuffer() != null) {
					buffer.append(new String(((X_OCTET) svcinfo.getBuffer())
							.getByteArray()));
				} else {
					buffer.append("dud");
				}
				toReturn.setByteArray("fail".getBytes());
			}
			return new Response(Connection.TPSUCCESS, 20, toReturn, len, 0);
		} catch (ConnectionException e) {
			return new Response(Connection.TPFAIL, Connection.TPEOTYPE, null,
					0, 0);
		}
	}
}
