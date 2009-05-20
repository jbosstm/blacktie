package org.jboss.blacktie.jatmibroker.xatmi;

public class TestTPConversationService implements BlacktieService {
	public Response tpservice(TPSVCINFO svcinfo) {
		try {
			Session session = svcinfo.getSession();
			Buffer buffer = new Buffer(null, null, 0);
			int iterationCount = 100;
			String received = new String(svcinfo.getBuffer().getData());
			if (received.equals("conversate")) {
				for (int i = 0; i < iterationCount; i++) {
					byte[] toSend = ("hi" + i).getBytes();
					buffer.setData(toSend);
					buffer.setLen(toSend.length);
					session.tpsend(buffer, 0);
					Buffer tprecv = session.tprecv(0);
					if (!new String(tprecv.getData()).equals("yo" + i)) {
						buffer.setData(tprecv.getData());
						buffer.setLen(tprecv.getLen());
						return new Response((short) 0, 0, buffer, 0);
					}
				}
			} else {
				buffer.setData(svcinfo.getBuffer().getData());
				buffer.setLen(svcinfo.getBuffer().getLen());
				return new Response((short) 0, 0, buffer, 0);
			}
			byte[] toSend = ("hi" + iterationCount).getBytes();
			buffer.setData(toSend);
			buffer.setLen(toSend.length);
			return new Response((short) 0, 0, buffer, 0);
		} catch (ConnectionException t) {
			t.printStackTrace();
			return null;
		}
	}
}