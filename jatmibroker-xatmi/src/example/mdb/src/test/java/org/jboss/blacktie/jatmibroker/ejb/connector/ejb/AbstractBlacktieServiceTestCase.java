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

import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionFactory;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.X_OCTET;

public class AbstractBlacktieServiceTestCase extends TestCase {
	private Connection connection;

	public AbstractBlacktieServiceTestCase() throws ConnectionException {
	}

	public void setUp() throws ConnectionException, ConfigurationException {
		ConnectionFactory connectionFactory = ConnectionFactory
				.getConnectionFactory();
		connection = connectionFactory.getConnection();
	}

	public void tearDown() throws ConnectionException, ConfigurationException {
		connection.close();
	}

	public void test() throws ConnectionException {
		X_OCTET buffer = (X_OCTET) connection.tpalloc("X_OCTET", null, 4);
		buffer.setByteArray("echo".getBytes());

		Response response = connection.tpcall("EchoService", buffer, 0);
		String responseData = new String(
				((X_OCTET) response.getBuffer()).getByteArray());
		assertEquals("echo", responseData);
	}
}
