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

public class TestSpecExampleOne extends TestCase {
	private static final Logger log = LogManager
			.getLogger(TestSpecExampleOne.class);

	public static final int OK = 1;

	public static final int NOT_OK = 0;

	private AtmiBrokerServer server;
	private Connection connection;

	public void setUp() throws ConnectionException, ConfigurationException {
		this.server = new AtmiBrokerServer("standalone-server", null);
		this.server.tpadvertise("TestOne", TestSpecExampleOneService.class
				.getName());
		this.server.tpadvertise("TestTwo", TestSpecExampleOneService.class
				.getName());

		ConnectionFactory connectionFactory = ConnectionFactory
				.getConnectionFactory();
		connection = connectionFactory.getConnection();
	}

	public void tearDown() throws ConnectionException, ConfigurationException {
		connection.close();
		server.close();
	}

	public void test() throws ConnectionException {
		log.info("TestSpecExampleOne::test_specexampleone");
		long dlen = 0;
		long clen = 0; /* contains a character array named input and an */
		int cd; /* integer named output. */
		/* allocate typed buffers */
		Buffer dptr = new Buffer("X_C_TYPE", "dc_buf"); // TODO ,0
		dptr.format(new String[] { "input", "output", "failTest" },
				new Class[] { char[].class, int.class, int.class },
				new int[] { 100, 0, 0 });
		Buffer cptr = new Buffer("X_C_TYPE", "dc_buf"); // TODO ,0
		cptr.format(new String[] { "input", "output", "failTest" },
				new Class[] { char[].class, int.class, int.class },
				new int[] { 100, 0, 0 });
		/* populate typed buffers with input data */
		dptr.setString("input", "debit account 123 by 50");
		cptr.setString("input", "credit account 456 by 50");
		// TODO tx_begin(); /* start global transaction */
		/* issue asynchronous request to DEBIT, while it is processing... */
		cd = connection.tpacall("TestOne", dptr, 0, Connection.TPSIGRSTRT);
		/* ...issue synchronous request to CREDIT */
		Response response = connection.tpcall("TestTwo", cptr, 0,
				Connection.TPSIGRSTRT);
		cptr = response.getBuffer();
		cptr.format(new String[] { "input", "output", "failTest" },
				new Class[] { char[].class, int.class, int.class },
				new int[] { 100, 0, 0 });
		clen = response.getLen();
		/* retrieve DEBIT�s reply */
		response = connection.tpgetrply(cd, Connection.TPSIGRSTRT);
		dptr = response.getBuffer();
		dptr.format(new String[] { "input", "output", "failTest" },
				new Class[] { char[].class, int.class, int.class },
				new int[] { 100, 0, 0 });
		dlen = response.getLen();
		if (dptr.getInt("output") == OK && cptr.getInt("output") == OK) {
			// TODO tx_commit(); /* commit global transaction */
		} else {
			// TODO tx_rollback(); /* rollback global transaction */
		}
	}
}
