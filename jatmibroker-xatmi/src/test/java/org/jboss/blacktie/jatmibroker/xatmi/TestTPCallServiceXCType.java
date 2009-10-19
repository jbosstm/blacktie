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

			char[] receivedName = new char[3];
			System.arraycopy(aptr.getCharArray("name"), 0, receivedName, 0, 3);
			char[] expectedName = "TOM".toCharArray();
			long accountNumber = aptr.getLong("acct_no");

			float fooOne = aptr.getFloatArray("foo")[0];
			float fooTwo = aptr.getFloatArray("foo")[1];

			double balanceOne = aptr.getDoubleArray("balances")[0];
			double balanceTwo = aptr.getDoubleArray("balances")[1];
			if (accountNumber == 12345678
					&& Arrays.equals(receivedName, expectedName)
					&& fooOne == 1.1F && fooTwo == 2.2F && balanceOne == 1.1
					&& balanceTwo == 2.2) {
				ok = true;
			}
			int len = 60;
			Buffer toReturn = svcinfo.tpalloc("X_OCTET", null);
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
