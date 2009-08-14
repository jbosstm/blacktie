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
package org.jboss.blacktie.jatmibroker.ejb.connector.ejb;

import junit.framework.TestCase;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.jab.JABException;
import org.jboss.blacktie.jatmibroker.jab.JABSession;
import org.jboss.blacktie.jatmibroker.jab.JABSessionAttributes;
import org.jboss.blacktie.jatmibroker.jab.JABTransaction;
import org.jboss.blacktie.jatmibroker.xatmi.Buffer;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionFactory;
import org.jboss.blacktie.jatmibroker.xatmi.Response;

public class TxBlacktieServiceTestCase extends TestCase {
	private static final Logger log = LogManager
			.getLogger(TxBlacktieServiceTestCase.class);
	private Connection connection;

	public TxBlacktieServiceTestCase() throws ConnectionException {
	}

	public void setUp() throws ConnectionException, ConfigurationException {
		ConnectionFactory connectionFactory = ConnectionFactory
				.getConnectionFactory();
		connection = connectionFactory.getConnection();
	}

	public void tearDown() throws ConnectionException, ConfigurationException {
		connection.close();
	}

	private JABTransaction startTx() throws JABException {
		JABSessionAttributes attrs = new JABSessionAttributes(null);
		JABSession session = new JABSession(attrs);

		// for (Map.Entry e : attrs.getProperties().entrySet())
		// log.info(e.getKey() + " = " + e.getValue());

		try {
			return new JABTransaction(session, 5000);
		} catch (Exception e) {
			throw new JABException(e.getMessage(), e);
		}
	}

	public void test1() throws ConnectionException, JABException {
		byte[] args = "test=test1,tx=true".getBytes();
		Buffer buffer = new Buffer("X_OCTET", null);
		buffer.setData(args);

		JABTransaction transaction = startTx();
		Response response = connection.tpcall("TxEchoService", buffer,
				args.length, 0);
		String responseData = new String(response.getBuffer().getData());
		transaction.commit();
		assertEquals("test=test1,tx=true", responseData);
	}

	public void test2() throws ConnectionException, JABException {
		byte[] args = "test=test2,tx=true".getBytes();
		Buffer buffer = new Buffer("X_OCTET", null);
		buffer.setData(args);

		Response response = connection.tpcall("TxEchoService", buffer,
				args.length, 0);
		String responseData = new String(response.getBuffer().getData());
		assertNotSame("test=test2,tx=true", responseData);
	}

	public void test3() throws ConnectionException, JABException {
		byte[] args = "test=test3,tx=false".getBytes();
		Buffer buffer = new Buffer("X_OCTET", null);
		buffer.setData(args);

		Response response = connection.tpcall("TxEchoService", buffer,
				args.length, 0);
		String responseData = new String(response.getBuffer().getData());
		assertEquals("test=test3,tx=false", responseData);
	}

	public void test4() throws ConnectionException, JABException {
		byte[] args = "test=test4,tx=false".getBytes();
		Buffer buffer = new Buffer("X_OCTET", null);
		buffer.setData(args);

		JABTransaction transaction = startTx();
		Response response = connection.tpcall("TxEchoService", buffer,
				args.length, 0);
		String responseData = new String(response.getBuffer().getData());
		transaction.commit();
		assertNotSame("test=test4,tx=false", responseData);
	}

	/*
	 * Test that the AS can create a transaction and propagate it too another
	 * blacktie service. TODO Disabled until we can resolve the clash between
	 * the JMS transaction the Blacktie clients transaction (JMS queues have
	 * local transactions)
	 */
	public void test5() throws ConnectionException, JABException {
		byte[] args = "test=test5,tx=create".getBytes();
		Buffer buffer = new Buffer("X_OCTET", null);
		buffer.setData(args);

		Response response = connection.tpcall("TxEchoService", buffer,
				args.length, 0);
		String responseData = new String(response.getBuffer().getData());
		assertEquals("test=test5,tx=create", responseData);
	}
}
