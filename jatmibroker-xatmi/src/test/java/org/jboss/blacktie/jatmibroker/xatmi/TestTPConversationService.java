package org.jboss.blacktie.jatmibroker.xatmi;

public class TestTPConversationService implements BlacktieService {
	public Response tpservice(TPSVCINFO svcinfo) {
		Session session = svcinfo.getSession();
		Buffer buffer = new Buffer(null, null);
		int iterationCount = 100;
		String received;
		try {
			received = (String) svcinfo.getBuffer().getData();
			if (received.equals("conversate")) {
				for (int i = 0; i < iterationCount; i++) {
					buffer.setData("hi" + i);
					try {
						session.tpsend(buffer, ("hi" + i).length(), 0);
						Buffer tprecv = session.tprecv(0);
						String returnedData = (String) tprecv.getData();
						if (!returnedData.equals("yo" + i)) {
							buffer.setData(received);
							return new Response((short) 0, 0, buffer, received
									.length(), 0);
						}
					} catch (ConnectionException t) {
						t.printStackTrace();
						return null;
					}
				}
			} else {
				buffer.setData(received);
				return new Response((short) 0, 0, buffer, received.length(), 0);
			}
			String response = "hi" + iterationCount;
			buffer.setData(response);
			return new Response((short) 0, 0, buffer, response.length(), 0);
		} catch (Throwable e) {
			e.printStackTrace();
			return null;
		}
	}
}