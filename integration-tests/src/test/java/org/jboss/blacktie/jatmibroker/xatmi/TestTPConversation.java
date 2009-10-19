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

import junit.framework.TestCase;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.RunServer;
import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;

public class TestTPConversation extends TestCase {
	private static final Logger log = LogManager
			.getLogger(TestTPConversation.class);
	private RunServer server = new RunServer();

	private Connection connection;
	private int sendlen;
	private Buffer sendbuf;
	private Session cd;

	static int interationCount = 100;

	public void setUp() throws ConnectionException, ConfigurationException {
		server.serverinit();

		ConnectionFactory connectionFactory = ConnectionFactory
				.getConnectionFactory();
		connection = connectionFactory.getConnection();
		sendlen = 11;
		sendbuf = new X_OCTET();

	}

	public void tearDown() throws ConnectionException, ConfigurationException {
		connection.close();
		server.serverdone();
	}

	public void test_conversation() throws ConnectionException {
		log.info("test_conversation");
		server.tpadvertiseTestTPConversation();

		sendbuf.setData("conversate".getBytes());
		cd = connection.tpconnect(server.getServiceNameTestTPConversation(),
				sendbuf, sendlen, Connection.TPRECVONLY);
		long revent = 0;
		log.info("Started conversation");
		for (int i = 0; i < interationCount; i++) {
			try {
				Buffer result = cd.tprecv(0);
				fail("Did not get sendonly event");
			} catch (ConnectionException e) {
				assertTrue(e.getTperrno() == Connection.TPEEVENT);
				assertTrue(e.getEvent() == Connection.TPEV_SENDONLY);
				Buffer rcvbuf = e.getReceived();
				String expectedResult = ("hi" + i);
				assertTrue(strcmp(expectedResult, rcvbuf) == 0);

				sendbuf.setData(("yo" + i).getBytes());
				// userlogc((char*) "test_conversation:%s:", sendbuf);
				int result = cd.tpsend(sendbuf, sendlen, Connection.TPRECVONLY);
				assertTrue(result != -1);
			}
		}
		log.info("Conversed");
		Response rcvbuf = connection.tpgetrply(cd.getCd(), 0);

		String expectedResult = ("hi" + interationCount);
		log.info("Expected: " + expectedResult + " Received: "
				+ new String(rcvbuf.getBuffer().getData()));
		assertTrue(strcmp(expectedResult, rcvbuf.getBuffer()) == 0);
	}

	public void test_short_conversation() throws ConnectionException {
		server.tpadvertiseTestTPConversa2();

		log.info("test_short_conversation");
		cd = connection.tpconnect(server.getServiceNameTestTPConversa2(), null,
				0, Connection.TPRECVONLY);
		assertTrue(cd != null);

		Buffer rcvbuf = cd.tprecv(0);
		assertTrue(rcvbuf != null);
		assertTrue(strcmp("hi0", rcvbuf) == 0);

		rcvbuf = connection.tpgetrply(cd.getCd(), 0).getBuffer();
		assertTrue(strcmp("hi1", rcvbuf) == 0);
	}

	public static int strcmp(String string, Buffer buffer) {
		byte[] expected = string.getBytes();
		byte[] received = buffer.getData();
		if (received.length < expected.length) {
			return -1;
		}
		for (int i = 0; i < expected.length; i++) {
			if (expected[i] != received[i]) {
				return -1;
			}
		}

		for (int i = expected.length; i < received.length; i++) {
			if (received[i] != '\0') {
				return -1;
			}
		}
		return 0;
	}

	public static int strcmp(Buffer buffer, String string) {
		return strcmp(string, buffer);
	}

}
