package org.jboss.blacktie.jatmibroker.xatmi;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

public class TestTPCallServiceXOctetZero implements BlacktieService {
	private static final Logger log = LogManager
			.getLogger(TestTPCallServiceXOctetZero.class);

	public Response tpservice(TPSVCINFO svcinfo) {
		try {
			byte[] recv = svcinfo.getBuffer().getData();
			byte[] toSend = new byte[recv.length];
			int j = recv.length;
			for (int i = 0; i < toSend.length; i++) {
				toSend[i] = recv[--j];
			}
			Buffer toReturn = new Buffer("X_OCTET", null);
			toReturn.setData(toSend);
			return new Response((short) 0, 0, toReturn, toSend.length, 0);
		} catch (ConnectionException e) {
			return new Response(Connection.TPFAIL, Connection.TPEITYPE, null,
					0, 0);
		}
	}
}
