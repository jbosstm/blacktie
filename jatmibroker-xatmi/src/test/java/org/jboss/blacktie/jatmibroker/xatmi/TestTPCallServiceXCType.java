package org.jboss.blacktie.jatmibroker.xatmi;

import java.util.Arrays;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

public class TestTPCallServiceXCType implements BlacktieService {
	private static final Logger log = LogManager
			.getLogger(TestTPCallServiceXCType.class);

	public Response tpservice(TPSVCINFO svcinfo) {
		try {
			log.info("test_tpcall_x_c_type_service");
			boolean ok = false;
			Buffer aptr = svcinfo.getBuffer();
			aptr.format(
					new String[] { "acct_no", "name", "address", "balance" },
					new Class[] { long.class, char[].class, char[].class,
							float[].class }, new int[] { 0, 50, 100, 2 });

			char[] receivedName = new char[3];
			System.arraycopy(aptr.getCharArray("name"), 0, receivedName, 0, 3);
			char[] expectedName = "TOM".toCharArray();
			if (aptr.getLong("acct_no") == 12345678
					&& Arrays.equals(receivedName, expectedName)
					&& aptr.getFloatArray("balance")[0] == 1.1F
					&& aptr.getFloatArray("balance")[1] == 2.2F) {
				ok = true;
			}
			int len = 60;
			Buffer toReturn = new Buffer("X_OCTET", null);
			if (ok) {
				toReturn.setData("tpcall_x_c_type".getBytes());
			} else {
				toReturn.setData("fail".getBytes());
			}
			return new Response(Connection.TPSUCCESS, 23, toReturn, len, 0);
		} catch (ConnectionException e) {
			return new Response(Connection.TPFAIL, 0, null, 0, 0);
		}
	}
}
