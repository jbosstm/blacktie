package org.jboss.blacktie.jatmibroker.xatmi;

import java.util.Arrays;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

public class TestTPConversationService implements BlacktieService {
	private static final Logger log = LogManager
			.getLogger(TestTPConversationService.class);

	public Response tpservice(TPSVCINFO svcinfo) {
		try {
			log.info("testTPConversation_service");
			boolean fail = false;
			Buffer sendbuf = new X_OCTET();

			if (!Arrays.equals("conversate".getBytes(), svcinfo.getBuffer()
					.getData())) {
				if (svcinfo.getBuffer() != null) {
					log.error("Got invalid data %s"
							+ new String(svcinfo.getBuffer().getData()));
				} else {
					log.error("GOT A NULL");
				}
				fail = true;
			} else {
				long revent = 0;
				log.info("Chatting");
				for (int i = 0; i < TestTPConversation.interationCount; i++) {
					sendbuf.setData(("hi" + i).getBytes());
					// userlogc((char*) "testTPConversation_service:%s:",
					// sendbuf);
					int result = svcinfo.getSession().tpsend(sendbuf,
							svcinfo.getBuffer().getLength(),
							Connection.TPRECVONLY);
					if (result != -1) {
						try {
							svcinfo.getSession().tprecv(0);
							fail = true;
							break;
						} catch (ConnectionException e) {
							Buffer rcvbuf = e.getReceived();
							if (rcvbuf != null
									&& e.getEvent() == Connection.TPEV_SENDONLY) {
								if (TestTPConversation.strcmp("yo" + i, rcvbuf) != 0) {
									fail = true;
									break;
								}
							} else {
								fail = true;
								break;
							}
						}
					} else {
						fail = true;
						break;
					}
				}
				log.info("Chatted");
			}

			if (fail) {
				return new Response((short) Connection.TPESVCFAIL, 0, sendbuf,
						0, 0);
			} else {
				sendbuf.setData(("hi" + TestTPConversation.interationCount)
						.getBytes());
				return new Response(Connection.TPSUCCESS, 0, sendbuf, svcinfo
						.getBuffer().getLength(), 0);
			}
		} catch (ConnectionException e) {
			return new Response(Connection.TPFAIL, Connection.TPEITYPE, null,
					0, 0);
		}
	}
}