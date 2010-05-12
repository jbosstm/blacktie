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
package org.jboss.blacktie.jatmibroker.jab.factory;

import java.util.Arrays;

import junit.framework.TestCase;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.RunServer;
import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;

public class JABFactoryTestCase extends TestCase {
	private static final Logger log = LogManager
			.getLogger(JABFactoryTestCase.class);
	private RunServer runServer = new RunServer();

	public void setUp() throws InterruptedException, ConfigurationException,
			ConnectionException {
		runServer.serverinit();
	}

	public void tearDown() throws ConnectionException {
		runServer.serverdone();
	}

	public void test_tpcall_x_octet() throws Exception {
		runServer.tpadvertisetpcallXOctet();
		JABConnectionFactory factory = JABConnectionFactory.getInstance();
		JABConnection connection = factory.getConnection("connection");
		JABBuffer toSend = new JABBuffer();
		toSend.setArrayValue("X_OCTET", "test_tpcall_x_octet".getBytes());
		JABResponse call = connection.call(RunServer
				.getServiceNametpcallXOctet(), toSend, null, "X_OCTET", null);
		byte[] expected = new byte[60];
		System.arraycopy("tpcall_x_octet".getBytes(), 0, expected, 0, 14);
		byte[] received = call.getByteArray("X_OCTET");
		assertTrue(Arrays.equals(expected, received));
		factory.closeConnection("connection");
	}

	public void test_tpcall_x_c_type() throws Exception {
		runServer.tpadvertisetpcallXCType();
		JABConnectionFactory factory = JABConnectionFactory.getInstance();
		JABConnection connection = factory.getConnection("connection");

		// Assemble the message ByteArrayOutputStream baos = new
		JABBuffer toSend = new JABBuffer();
		toSend.setValue("acct_no", 12345678l);
		toSend.setArrayValue("name", "TOM".getBytes());
		float[] foo = new float[2];
		foo[0] = 1.1F;
		foo[1] = 2.2F;
		toSend.setArrayValue("foo", foo);

		double[] balances = new double[2];
		balances[0] = 1.1;
		balances[1] = 2.2;
		toSend.setArrayValue("balances", balances);

		JABResponse call = connection.call(RunServer
				.getServiceNametpcallXOctet(), toSend, null, "X_C_TYPE",
				"acct_info");

		byte[] expected = new byte[60];
		System.arraycopy("tpcall_x_c_type".getBytes(), 0, expected, 0, 15);
		byte[] received = call.getByteArray("X_OCTET");
		assertTrue(Arrays.equals(expected, received));
		factory.closeConnection("connection");
	}
}
