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

public class TestSpecExampleOne extends TestCase {
	private static final Logger log = LogManager
			.getLogger(TestSpecExampleOne.class);

	public static final int OK = 1;

	public static final int NOT_OK = 0;

	private RunServer server = new RunServer();
	private Connection connection;

	public void setUp() throws ConnectionException, ConfigurationException {
		server.serverinit();
		server.tpadvertiseCREDIT();
		server.tpadvertiseDEBIT();

		ConnectionFactory connectionFactory = ConnectionFactory
				.getConnectionFactory();
		connection = connectionFactory.getConnection();
	}

	public void tearDown() throws ConnectionException, ConfigurationException {
		connection.close();
		server.serverdone(); // server.close();
	}

	public void test() throws ConnectionException {
		log.info("TestSpecExampleOne::test_specexampleone");
		long dlen = 0;
		long clen = 0; /* contains a character array named input and an */
		int cd; /* integer named output. */
		/* allocate typed buffers */
		Buffer dptr = new R_PBF("dc_buf"); // TODO ,0
		dptr.format(new String[] { "input", "output", "failTest" },
				new Class[] { char[].class, int.class, int.class }, new int[] {
						100, 0, 0 });
		Buffer cptr = new R_PBF("dc_buf"); // TODO ,0
		cptr.format(new String[] { "input", "output", "failTest" },
				new Class[] { char[].class, int.class, int.class }, new int[] {
						100, 0, 0 });
		/* populate typed buffers with input data */
		dptr.setCharArray("input", "debit account 123 by 50".toCharArray());
		cptr.setCharArray("input", "credit account 456 by 50".toCharArray());
		// TODO tx_begin(); /* start global transaction */
		/* issue asynchronous request to DEBIT, while it is processing... */
		cd = connection.tpacall(server.getServiceNameDEBIT(), dptr, 0,
				Connection.TPSIGRSTRT);
		/* ...issue synchronous request to CREDIT */
		Response response = connection.tpcall(server.getServiceNameCREDIT(),
				cptr, 0, Connection.TPSIGRSTRT);
		cptr = response.getBuffer();
		cptr.format(new String[] { "input", "output", "failTest" },
				new Class[] { char[].class, int.class, int.class }, new int[] {
						100, 0, 0 });
		clen = response.getLen();
		/* retrieve DEBIT�s reply */
		response = connection.tpgetrply(cd, Connection.TPSIGRSTRT);
		dptr = response.getBuffer();
		dptr.format(new String[] { "input", "output", "failTest" },
				new Class[] { char[].class, int.class, int.class }, new int[] {
						100, 0, 0 });
		dlen = response.getLen();
		if (dptr.getInt("output") == OK && cptr.getInt("output") == OK) {
			// TODO tx_commit(); /* commit global transaction */
		} else {
			// TODO tx_rollback(); /* rollback global transaction */
		}
	}
}
