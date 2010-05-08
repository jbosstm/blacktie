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

public class TestTPSend extends TestCase {
	private static final Logger log = LogManager.getLogger(TestTPSend.class);
	private RunServer server = new RunServer();
	private Connection connection;
	private int sendlen;
	private X_OCTET sendbuf;
	private Session cd;

	public void setUp() throws ConnectionException, ConfigurationException {
		server.serverinit();

		ConnectionFactory connectionFactory = ConnectionFactory
				.getConnectionFactory();
		connection = connectionFactory.getConnection();

		sendlen = "tpsend".length() + 1;
		sendbuf = (X_OCTET) connection.tpalloc("X_OCTET", null);
		sendbuf.setByteArray("tpsend".getBytes());

	}

	public void tearDown() throws ConnectionException, ConfigurationException {
		if (cd != null) {
			cd.tpdiscon();
		}

		connection.close();
		server.serverdone();
	}

	public void test_tpsend_recvonly() throws ConnectionException {
		log.info("test_tpsend_recvonly");
		server.tpadvertiseTestTPSend();

		cd = connection.tpconnect(server.getServiceNameTestTPSend(), sendbuf,
				sendlen, Connection.TPRECVONLY);
		try {
			cd.tpsend(sendbuf, sendlen, 0);
			fail("expected proto error");
		} catch (ResponseException e) {
			assertTrue((e.getEvent() == Connection.TPEV_SVCERR)
					|| (e.getTperrno() == Connection.TPEPROTO));
		} catch (ConnectionException e) {
			fail("expected proto error");
		}
	}

	public void test_tpsend_tpsendonly() throws ConnectionException {
		log.info("test_tpsend_tpsendonly");
		server.tpadvertiseTestTPSendTPSendOnly();

		cd = connection.tpconnect(server.getServiceNameTestTPSendTPSendOnly(),
				sendbuf, sendlen, Connection.TPRECVONLY);

		try {
			cd.tprecv(0);
			fail("Expected SENDONLY event");
		} catch (ResponseException e) {
			assertTrue(e.getTperrno() == Connection.TPEEVENT);
			assertTrue(e.getEvent() == Connection.TPEV_SENDONLY);
		}
		try {
			cd.tprecv(0);
			fail("Expected TPEPROTO");
		} catch (ConnectionException e) {
			assertTrue(e.getTperrno() == Connection.TPEPROTO);
		}

		cd.tpsend(sendbuf, sendlen, 0);
	}
}
