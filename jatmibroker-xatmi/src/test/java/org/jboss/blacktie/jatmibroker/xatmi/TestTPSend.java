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
import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.core.server.AtmiBrokerServer;

public class TestTPSend extends TestCase {
	private static final Logger log = LogManager.getLogger(TestTPSend.class);
	private AtmiBrokerServer server;
	private Connection connection;
	private int sendlen;
	private Buffer sendbuf;
	private Session cd;

	public void setUp() throws ConnectionException, ConfigurationException {
		this.server = new AtmiBrokerServer("standalone-server", null);
		this.server.tpadvertise("TestOne", TestTPSendService.class.getName());

		ConnectionFactory connectionFactory = ConnectionFactory
				.getConnectionFactory();
		connection = connectionFactory.getConnection();

		sendlen = "tpsend".length() + 1;
		sendbuf = new Buffer("X_OCTET", null);
		sendbuf.setData("tpsend".getBytes());

	}

	public void tearDown() throws ConnectionException, ConfigurationException {
		if (cd != null) {
			cd.tpdiscon();
		}

		connection.close();
		server.close();
	}

	public void test_tpsend_recvonly() throws ConnectionException {
		log.info("test_tpsend_recvonly");
		cd = connection.tpconnect("TestOne", sendbuf, sendlen,
				Connection.TPRECVONLY);
		try {
			cd.tpsend(sendbuf, sendlen, 0);
		} catch (ConnectionException e) {
			assertTrue((e.getEvent() == Connection.TPEV_SVCERR)
					|| (e.getTperrno() == Connection.TPEPROTO));
		}
	}
}
