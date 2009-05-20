package org.jboss.blacktie.jatmibroker.xatmi;

public class TestTPConversationService implements BlacktieService {
	public Response tpservice(TPSVCINFO svcinfo) {
		try {
			ConnectionFactory connectionFactory = ConnectionFactory
					.getConnectionFactory();
			Connection con = connectionFactory.getConnection("", "");

			int iterationCount = 100;
			String received = new String(svcinfo.getData());
			if (received.equals("conversate")) {
				for (int i = 0; i < iterationCount; i++) {
					byte[] toSend = ("hi" + i).getBytes();
					con.tpsend(svcinfo.getCd(), toSend, toSend.length, 0);
					Response tprecv = con.tprecv(svcinfo.getCd(), 0);
					if (!new String(tprecv.getData()).equals("yo" + i)) {
						return new Response((short) 0, 0, tprecv.getData(),
								tprecv.getLen(), 0);
					}
				}
			} else {
				return new Response((short) 0, 0, svcinfo.getData(), svcinfo
						.getLen(), 0);
			}
			return new Response((short) 0, 0, svcinfo.getData(), svcinfo
					.getLen(), 0);
		} catch (ConnectionException t) {
			t.printStackTrace();
			return null;
		}
	}
}