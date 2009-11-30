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
package org.jboss.blacktie.jatmibroker.admin;

import junit.framework.TestCase;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.RunServer;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionFactory;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.X_OCTET;

public class AdministrationTest extends TestCase {
	private static final Logger log = LogManager
			.getLogger(AdministrationTest.class);
	
	private RunServer runServer = new RunServer();
	private Connection connection;
	private String service = "default_ADMIN_1";
	
	private String callAdmin(String command, char expect) throws Exception {
		int sendlen = command.length() + 1;
		X_OCTET sendbuf = (X_OCTET) connection.tpalloc("X_OCTET", null);
		sendbuf.setByteArray(command.getBytes());

		Response buf = connection.tpcall(service, sendbuf, sendlen, 0);
		assertTrue(buf != null);
		byte[] received = ((X_OCTET) buf.getBuffer()).getByteArray();
		assertTrue(received[0] == expect);
		
		return new String(received, 1, received.length - 1);
	}
	
	private void callBAR() throws ConnectionException {
		connection.tpcall("BAR", null, 0, 0);
		log.info("call BAR OK");
	}
	
	public void setUp() throws Exception {
		runServer.serverinit();
		ConnectionFactory connectionFactory = ConnectionFactory
		.getConnectionFactory();
		connection = connectionFactory.getConnection();
	}
	
	public void tearDown() {
		runServer.serverdone();
	}
	
	public void testShutdown() throws Exception {
		callAdmin("serverdone", '1');
	}
	
	public void testAdvertiseAndUnadvertise() throws Exception {
		callBAR();
		callAdmin("unadvertise,BAR", '1');
		try {
			callBAR();
			fail("Should fail when unadvertise BAR");
		} catch (ConnectionException e) {
			assertTrue("Error was: " + e.getTperrno(), 
					//e.getTperrno() == Connection.TPENOENT);
					e.getTperrno() == -1);
		}
		callAdmin("advertise,BAR", '1');
		callBAR();
		
		// can not (un)advertise ADMIN service
		callAdmin("advertise,default_ADMIN_1", '0');
		callAdmin("unadvertise,default_ADMIN_1", '0');
		
		// can not (un)advertise UNKNOW service
		callAdmin("advertise,UNKNOW", '0');
		callAdmin("unadvertise,UNKNOW", '0');
	}
	
	public void testGetServiceCounter() throws Exception {
		int n = -1;
		
		callBAR();
		n = Integer.parseInt(callAdmin("counter,BAR,", '1'));
		assertTrue(n == 1);
	}

	public void testGetServiceStatus() throws Exception {
		String status = callAdmin("status", '1');
		log.info("status is " + status);
		
		status = callAdmin("status,BAR,", '1');
		log.info("status is " + status);
	}

	public void testPauseAndResumeServer() throws Exception {
		callAdmin("pause", '1');
		log.info("pause server OK");

		callAdmin("unadvertise,BAR,", '1');
		callAdmin("advertise,BAR,", '1');

		try {
			log.info("call BAR should time out after 20 second");
			callBAR();
		} catch (ConnectionException e) {
			assertTrue("Error was: " + e.getTperrno(), 
					e.getTperrno() == Connection.TPETIME);
		}

		callAdmin("resume", '1');
		log.info("resume server OK");
		callBAR();
	}
}
