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
package org.jboss.blacktie.administration;

import junit.framework.TestCase;

import org.jboss.blacktie.jatmibroker.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.xatmi.Buffer;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionFactory;
import org.jboss.blacktie.jatmibroker.xatmi.Response;

public class BlacktieStompAdministrationService extends TestCase {
	private Connection connection;

	public void setUp() throws ConnectionException, ConfigurationException {
		ConnectionFactory connectionFactory = ConnectionFactory
				.getConnectionFactory();
		connection = connectionFactory.getConnection();
		processCommand("tpadvertise,TestTPFree");
	}

	public void tearDown() throws ConnectionException, ConfigurationException {
		processCommand("tpadvertise,TestTPFree");
		connection.close();
	}

	public void test() {
		String[] commands = new String[] { "tpunadvertise,TestTPFree",
				"tpadvertise,TestTPFree" };
		for (int i = 0; i < commands.length; i++) {
			try {
				processCommand(commands[i]);
			} catch (ConnectionException e) {
				e.printStackTrace();
				fail(e.getMessage());
			}
		}
	}

	private void processCommand(String command) throws ConnectionException {
		byte[] toSend = command.getBytes();
		Buffer buffer = new Buffer(null, null);
		buffer.setData(toSend);

		Response response = connection.tpcall(
				"BlacktieStompAdministrationService", buffer,
				buffer.getData().length, 0);

		byte[] responseData = response.getBuffer().getData();
		assertEquals(1, responseData[0]);
	}
}
