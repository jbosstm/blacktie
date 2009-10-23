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
			X_C_TYPE aptr = (X_C_TYPE) svcinfo.getBuffer();

			byte[] receivedName = new byte[3];
			byte[] charArray = aptr.getByteArray("name");
			System.arraycopy(charArray, 0, receivedName, 0, 3);
			byte[] expectedName = "TOM".getBytes();
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
			X_OCTET toReturn = (X_OCTET) svcinfo.tpalloc("X_OCTET", null);
			if (ok) {
				toReturn.setByteArray("tpcall_x_c_type".getBytes());
			} else {
				toReturn.setByteArray("fail".getBytes());
			}
			return new Response(Connection.TPSUCCESS, 23, toReturn, len, 0);
		} catch (ConnectionException e) {
			return new Response(Connection.TPFAIL, 0, null, 0, 0);
		}
	}
}
