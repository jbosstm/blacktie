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

public class TestTPReturn extends TestCase {
	private static final Logger log = LogManager.getLogger(TestTPReturn.class);
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

	// 8.1 8.3 not possible in java
	// public void test_tpreturn_nonservice() {
	// this.server.tpadvertise("TestOne", TestTPReturnService.class.getName());
	//
	// log.info("test_tpreturn_nonservice");
	// // THIS IS ILLEGAL STATE TABLE
	// int len = 25;
	// char *toReturn = (char*) malloc(len);
	// strcpy(toReturn, "test_tpreturn_nonservice");
	// tpreturn(TPSUCCESS, 0, toReturn, len, 0);
	// free(toReturn);
	// }

	// REMOVE SERVICE JUST THROWS EXCEPTION
	public void test_tpreturn_nonbuffer() throws ConnectionException {
		log.info("test_tpreturn_nonbuffer");

		// Do local work
		this.server.tpadvertise("TestOne", TestTPReturnService.class.getName());

		int sendlen = "tprnb".length() + 1;
		Buffer sendbuf = new Buffer("X_OCTET", null);
		sendbuf.setData("tprnb".getBytes());

		try {
			connection.tpcall("TestOne", sendbuf, sendlen, 0);
			fail("Managed to send call");
		} catch (ConnectionException e) {
			assertTrue(e.getTperrno() == Connection.TPESVCERR);
		}
	}

	public void test_tpreturn_tpurcode() throws ConnectionException {
		log.info("test_tpreturn_tpurcode");

		// Do local work
		this.server.tpadvertise("TestOne", TestTPReturnServiceTpurcode.class
				.getName());

		int sendlen = 3;
		Buffer sendbuf = new Buffer("X_OCTET", null);
		sendbuf.setData("24".getBytes());
		Response success = connection.tpcall("TestOne", sendbuf, sendlen, 0);
		assertTrue(success != null);
		assertTrue(success.getRcode() == 24);

		sendbuf.setData("77".getBytes());
		success = connection.tpcall("TestOne", sendbuf, sendlen, 0);
		assertTrue(success != null);
		assertTrue(success.getRcode() == 77);
	}
}
