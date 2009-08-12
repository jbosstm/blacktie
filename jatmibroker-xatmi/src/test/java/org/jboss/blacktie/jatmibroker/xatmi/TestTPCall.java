/*
 * JBoss, Home of Professional Open Source
 * Copyright 2008, Red Hat, Inc., and others contributors as indicated
 * by the @authors tag. All rights reserved.
 * See the copyright.txt in the distribution for a
 * full listing of individual contributors.
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License, v. 2.1.
 * This program is distributed in the hope that it will be useful, but WITHOUT A
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 * You should have received a copy of the GNU Lesser General Public License,
 * v.2.1 along with this distribution; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */
package org.jboss.blacktie.jatmibroker.xatmi;

import java.util.Arrays;

import junit.framework.TestCase;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.core.server.AtmiBrokerServer;

public class TestTPCall extends TestCase {
	private static final Logger log = LogManager.getLogger(TestTPCall.class);
	private AtmiBrokerServer server;
	private Connection connection;

	public void setUp() throws ConnectionException, ConfigurationException {
		this.server = new AtmiBrokerServer("standalone-server", null);

		ConnectionFactory connectionFactory = ConnectionFactory
				.getConnectionFactory();
		connection = connectionFactory.getConnection();
	}

	public void tearDown() throws ConnectionException, ConfigurationException {
		connection.close();
		server.close();
	}

	public void test_tpcall_unknown_service() throws ConnectionException {
		log.info("test_tpcall_unknown_service");

		String message = "test_tpcall_unknown_service";
		int sendlen = message.length() + 1;
		Buffer sendbuf = new Buffer("X_OCTET", null);
		sendbuf.setData("test_tpcall_unknown_service".getBytes());

		try {
			Response rcvbuf = connection.tpcall("UNKNOWN_SERVICE", sendbuf,
					sendlen, 0);
			fail("Expected TPENOENT, got a buffer with rval: "
					+ rcvbuf.getRval());
		} catch (ConnectionException e) {
			if (e.getTperrno() != Connection.TPENOENT) {
				fail("Expected TPENOENT, got: " + e.getTperrno());
			}
		}
	}

	public void test_tpcall_x_octet() throws ConnectionException {
		log.info("test_tpcall_x_octet");
		this.server.tpadvertise("TestOne", TestTPCallServiceXOctet.class
				.getName());

		String toSend = "test_tpcall_x_octet";
		int sendlen = toSend.length() + 1;
		Buffer sendbuf = new Buffer("X_OCTET", null);
		sendbuf.setData(toSend.getBytes());

		Response rcvbuf = connection.tpcall("TestOne", sendbuf, sendlen, 0);
		assertTrue(rcvbuf != null);
		assertTrue(rcvbuf.getBuffer() != null);
		assertTrue(rcvbuf.getBuffer().getData() != null);
		byte[] received = rcvbuf.getBuffer().getData();
		byte[] expected = new byte[received.length];
		System.arraycopy("tpcall_x_octet".getBytes(), 0, expected, 0,
				"tpcall_x_octet".getBytes().length);
		assertTrue(Arrays.equals(received, expected));
	}

	public void test_tpcall_x_common() throws ConnectionException {
		log.info("test_tpcall_x_common");
		this.server.tpadvertise("TestOne", TestTPCallServiceXCommon.class
				.getName());

		Buffer dptr = new Buffer("X_COMMON", "deposit");
		dptr.format(new String[] { "acct_no", "amount", "balance", "status",
				"status_len" }, new Class[] { long.class, short.class,
				short.class, char[].class, short.class }, new int[] { 0, 0, 0,
				128, 0 });

		dptr.setLong("acct_no", 12345678);
		dptr.setShort("amount", (short) 50);

		Response rcvbuf = connection.tpcall("TestOne", dptr, 0, 0);
		assertTrue(rcvbuf.getRcode() == 22);
		byte[] received = rcvbuf.getBuffer().getData();
		byte[] expected = new byte[received.length];
		System.arraycopy("tpcall_x_common".getBytes(), 0, expected, 0,
				"tpcall_x_common".getBytes().length);
		assertTrue(Arrays.equals(received, expected));
	}

	public void test_tpcall_x_c_type() throws ConnectionException {
		log.info("test_tpcall_x_c_type");
		this.server.tpadvertise("TestOne", TestTPCallServiceXCType.class
				.getName());

		Buffer aptr = new Buffer("X_C_TYPE", "acct_info");
		aptr.format(new String[] { "acct_no", "name", "address", "balance" },
				new Class[] { long.class, char[].class, char[].class,
						float[].class }, new int[] { 0, 50, 100, 2 });

		aptr.setLong("acct_no", 12345678);
		aptr.setCharArray("name", "TOM".toCharArray());
		float[] balances = new float[2];
		balances[0] = 1.1F;
		balances[1] = 2.2F;
		aptr.setFloatArray("balance", balances);

		Response rcvbuf = connection.tpcall("TestOne", aptr, 0,
				Connection.TPNOCHANGE);
		assertTrue(rcvbuf.getRcode() == 23);
		byte[] received = rcvbuf.getBuffer().getData();
		byte[] expected = new byte[received.length];
		System.arraycopy("tpcall_x_c_type".getBytes(), 0, expected, 0,
				"tpcall_x_c_type".getBytes().length);
		assertTrue(Arrays.equals(received, expected));
	}
}
