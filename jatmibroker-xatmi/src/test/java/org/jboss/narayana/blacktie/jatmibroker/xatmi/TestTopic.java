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
package org.jboss.narayana.blacktie.jatmibroker.xatmi;

import java.util.Arrays;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.narayana.blacktie.jatmibroker.RunServer;
import org.jboss.narayana.blacktie.jatmibroker.core.conf.ConfigurationException;

import junit.framework.TestCase;

public class TestTopic extends TestCase {
	private static final Logger log = LogManager.getLogger(TestTopic.class);
	private RunServer server = new RunServer();
	private Connection connection;
	
	public void setUp() throws ConnectionException, ConfigurationException {
		server.serverinit();

		ConnectionFactory connectionFactory = ConnectionFactory
				.getConnectionFactory();
		connection = connectionFactory.getConnection();
	}

	public void tearDown() throws ConnectionException, ConfigurationException {
		connection.close();
		server.serverdone();
	}
	
	public void test_tpcall_topic() throws ConnectionException {
		log.info("test_tpcall_topic");
		server.tpadvertiseTestTopic();
		
		String toSend = "test_tpcall_topic";
		int sendlen = toSend.length() + 1;
		X_OCTET sendbuf = (X_OCTET) connection
				.tpalloc("X_OCTET", null, sendlen);
		sendbuf.setByteArray(toSend.getBytes());
		
		Response rcvbuf = connection.tpcall(
				RunServer.getServiceNameTestTopic(), sendbuf, 0);
		assertTrue(rcvbuf != null);
		assertTrue(rcvbuf.getBuffer() != null);
		assertTrue(((X_OCTET) rcvbuf.getBuffer()).getByteArray() != null);
		byte[] received = ((X_OCTET) rcvbuf.getBuffer()).getByteArray();
		byte[] expected = new byte[received.length];
		System.arraycopy("testtpacall_service".getBytes(), 0, expected, 0,
				"testtpacall_service".getBytes().length);
		assertTrue(Arrays.equals(received, expected));
	}

}
