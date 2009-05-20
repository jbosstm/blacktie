package org.jboss.blacktie.jatmibroker.xatmi;

public class TestTPConversationService implements BlacktieService {
	public Response tpservice(TPSVCINFO svcinfo) {
		Session session = svcinfo.getSession();
		Buffer buffer = new Buffer(null, null);
		int iterationCount = 100;
		byte[] got = svcinfo.getBuffer().getData();
		String received = new String(got);
		if (received.equals("conversate")) {
			for (int i = 0; i < iterationCount; i++) {
				byte[] toSend = ("hi" + i).getBytes();
				buffer.setData(toSend);
				try {
					session.tpsend(buffer, toSend.length, 0);
					Buffer tprecv = session.tprecv(0);
					byte[] rcvd = tprecv.getData();
					String returnedData = new String(rcvd);
					if (!returnedData.equals("yo" + i)) {
						buffer.setData(rcvd);
						return new Response((short) 0, 0, buffer, rcvd.length,
								0);
					}
				} catch (ConnectionException t) {
					t.printStackTrace();
					return null;
				}
			}
		} else {
			buffer.setData(got);
			return new Response((short) 0, 0, buffer, got.length, 0);
		}
		byte[] toSend = ("hi" + iterationCount).getBytes();
		buffer.setData(toSend);
		return new Response((short) 0, 0, buffer, toSend.length, 0);
	}
}