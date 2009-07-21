package org.jboss.blacktie.jatmibroker.xatmi;

public class TestTPConversationService implements BlacktieService {
	public Response tpservice(TPSVCINFO svcinfo) {
		Session session = svcinfo.getSession();
		Buffer buffer = new Buffer(null, null);
		int iterationCount = 100;
		byte[] received = svcinfo.getBuffer().getData();
		if (new String(received).equals("conversate")) {
			for (int i = 0; i < iterationCount; i++) {
				byte[] toSend = ("hi" + i).getBytes();
				buffer.setData(toSend);
				try {
					session.tpsend(buffer, toSend.length, 0);
					Buffer tprecv = session.tprecv(0);
					byte[] returnedData = tprecv.getData();
					if (!new String(returnedData).equals(("yo" + i))) {
						buffer.setData(received);
						return new Response((short) 0, 0, buffer,
								received.length, 0);
					}
				} catch (ConnectionException t) {
					t.printStackTrace();
					return null;
				}
			}
		} else {
			buffer.setData(received);
			return new Response((short) 0, 0, buffer, received.length, 0);
		}
		String response = "hi" + iterationCount;
		buffer.setData(response.getBytes());
		return new Response((short) 0, 0, buffer, response.length(), 0);
	}
}