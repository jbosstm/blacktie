package org.jboss.blacktie.jatmibroker.xatmi;

public class TestTPCallService implements BlacktieService {
	public Response tpservice(TPSVCINFO svcinfo) {
		byte[] recv;
		try {
			recv = ((String) svcinfo.getBuffer().getData()).getBytes();
			byte[] toSend = new byte[recv.length];
			int j = recv.length;
			for (int i = 0; i < toSend.length; i++) {
				toSend[i] = recv[--j];
			}
			Buffer toReturn = new Buffer(null, null);
			toReturn.setData(new String(toSend));
			return new Response((short) 0, 0, toReturn, toSend.length, 0);
		} catch (ConnectionException e) {
			e.printStackTrace();
			return null;
		}
	}
}
