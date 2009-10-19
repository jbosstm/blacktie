package org.jboss.blacktie.jatmibroker.xatmi;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

public class TestTPConversationServiceShort implements BlacktieService {
	private static final Logger log = LogManager
			.getLogger(TestTPConversationServiceShort.class);

	public Response tpservice(TPSVCINFO svcinfo) {
		try {
			log.info("testTPConversation_short_service");
			int sendlen = 4;
			Buffer sendbuf = svcinfo.tpalloc("X_OCTET", null);
			sendbuf.setData("hi0".getBytes());
			svcinfo.getSession().tpsend(sendbuf, sendlen, 0);
			sendbuf.setData("hi1".getBytes());
			return new Response(Connection.TPSUCCESS, 0, sendbuf, sendlen, 0);
		} catch (ConnectionException e) {
			return new Response(Connection.TPFAIL, Connection.TPEITYPE, null,
					0, 0);
		}
	}
}