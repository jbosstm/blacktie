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
package org.jboss.blacktie.jatmibroker.tx;

import junit.framework.TestCase;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.RunServer;
import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.xatmi.Buffer;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionFactory;
import org.jboss.blacktie.jatmibroker.xatmi.Session;
import org.jboss.blacktie.jatmibroker.xatmi.TestTPConversation;

public class TestRollbackOnly extends TestCase {
	private static final Logger log = LogManager
			.getLogger(TestRollbackOnly.class);
	private RunServer server = new RunServer();
	private Connection connection;
	private int sendlen;
	private Buffer sendbuf;

	public void setUp() throws ConnectionException, ConfigurationException {
		server.serverinit();

		ConnectionFactory connectionFactory = ConnectionFactory
				.getConnectionFactory();
		connection = connectionFactory.getConnection();

		sendlen = "TestRbkOnly".length() + 1;
		sendbuf = connection.tpalloc("X_OCTET", null);
		sendbuf.setData("TestRbkOnly".getBytes());
	}

	public void tearDown() throws ConnectionException, ConfigurationException {
		connection.close();
		server.serverdone();
	}

	public void test_tpcall_TPETIME() throws ConnectionException {
		log.info("test_tpcall_TPETIME");
		server.tpadvertiseTestRollbackOnlyTpcallTPETIMEService();
		assertTrue(TX.tx_open() == TX.TX_OK);
		assertTrue(TX.tx_begin() == TX.TX_OK);

		try {
			connection.tpcall(server.getServiceNameTestRollbackOnly(), sendbuf,
					sendlen, 0);
			fail("Expected e.getTperrno() == Connection.TPETIME");
		} catch (ConnectionException e) {
			assertTrue(e.getTperrno() == Connection.TPETIME);
		}

		TXINFO txinfo = new TXINFO();
		int inTx = TX.tx_info(txinfo);
		log.info("inTx=" + inTx);
		assertTrue(txinfo.transaction_state == TX.TX_ROLLBACK_ONLY);
		assertTrue(TX.tx_commit() == TX.TX_ROLLBACK);
	}

	public void x_test_tpcall_TPEOTYPE() throws ConnectionException {
		log.info("test_tpcall_TPETIME");
		server.tpadvertiseTestTpcallTPEOTYPEService();

		assertTrue(TX.tx_open() == TX.TX_OK);
		assertTrue(TX.tx_begin() == TX.TX_OK);

		try {
			connection.tpcall(server.getServiceNameTestRollbackOnly(), sendbuf,
					sendlen, Connection.TPNOCHANGE);
			fail("Expected e.getTperrno() == TPEOTYPE");
		} catch (ConnectionException e) {
			assertTrue(e.getTperrno() == Connection.TPEOTYPE);
		}

		TXINFO txinfo = new TXINFO();
		int inTx = TX.tx_info(txinfo);
		log.info("inTx=" + inTx);
		assertTrue(txinfo.transaction_state == TX.TX_ROLLBACK_ONLY);
		assertTrue(TX.tx_commit() == TX.TX_ROLLBACK);
	}

	public void test_tpcall_TPESVCFAIL() throws ConnectionException {
		log.info("test_tpcall_TPESVCFAIL");
		server.tpadvertiseTestRollbackOnlyTpcallTPESVCFAILService();

		assertTrue(TX.tx_open() == TX.TX_OK);
		assertTrue(TX.tx_begin() == TX.TX_OK);

		try {
			connection.tpcall(server.getServiceNameTestRollbackOnly(), sendbuf,
					sendlen, 0);
			fail("Expected e.getTperrno() == TPESVCFAIL");
		} catch (ConnectionException e) {
			assertTrue(new String(e.getReceived().getData()),
					TestTPConversation.strcmp(e.getReceived(),
							"test_tpcall_TPESVCFAIL_service") == 0);
			assertTrue(e.getTperrno() == Connection.TPESVCFAIL);
		}

		TXINFO txinfo = new TXINFO();
		int inTx = TX.tx_info(txinfo);
		log.info("inTx=" + inTx);
		assertTrue(txinfo.transaction_state == TX.TX_ROLLBACK_ONLY);
		assertTrue(TX.tx_commit() == TX.TX_ROLLBACK);
	}

	public void test_tprecv_TPEV_DISCONIMM() throws ConnectionException {
		log.info("test_tprecv_TPEV_DISCONIMM");
		server.tpadvertiseTestRollbackOnlyTprecvTPEVDISCONIMMService();

		assertTrue(TX.tx_open() == TX.TX_OK);
		assertTrue(TX.tx_begin() == TX.TX_OK);

		Session cd = connection.tpconnect(server
				.getServiceNameTestRollbackOnly(), sendbuf, sendlen,
				Connection.TPSENDONLY);
		cd.tpdiscon();

		TXINFO txinfo = new TXINFO();
		int inTx = TX.tx_info(txinfo);
		log.info("inTx=" + inTx);
		assertTrue(txinfo.transaction_state == TX.TX_ROLLBACK_ONLY);
		assertTrue(TX.tx_commit() == TX.TX_ROLLBACK);
	}

	public void test_tprecv_TPEV_SVCFAIL() throws ConnectionException {
		log.info("test_tprecv_TPEV_SVCFAIL");
		server.tpadvertiseTestRollbackOnlyTprecvTPEVSVCFAILService();

		assertTrue(TX.tx_open() == TX.TX_OK);
		assertTrue(TX.tx_begin() == TX.TX_OK);

		Session cd = connection.tpconnect(server
				.getServiceNameTestRollbackOnly(), sendbuf, sendlen,
				Connection.TPRECVONLY);

		try {
			cd.tprecv(0);
			fail("Expected e.getEvent() == Connection.TPEV_SVCFAIL");
		} catch (ConnectionException e) {
			assertTrue(e.getEvent() == Connection.TPEV_SVCFAIL);
			assertTrue(e.getTperrno() == Connection.TPEEVENT);
			Buffer rcvbuf = e.getReceived();
			assertTrue(new String(rcvbuf.getData()), TestTPConversation.strcmp(
					rcvbuf, "test_tprecv_TPEV_SVCFAIL_service") == 0);
		}

		TXINFO txinfo = new TXINFO();
		int inTx = TX.tx_info(txinfo);
		log.info("inTx=" + inTx);
		assertTrue(txinfo.transaction_state == TX.TX_ROLLBACK_ONLY);
		assertTrue(TX.tx_commit() == TX.TX_ROLLBACK);
	}

	public void test_no_tpreturn() throws ConnectionException {
		log.info("test_no_tpreturn");
		server.tpadvertiseTestRollbackOnlyNoTpreturnService();

		assertTrue(TX.tx_open() == TX.TX_OK);
		assertTrue(TX.tx_begin() == TX.TX_OK);

		try {
			connection.tpcall(server.getServiceNameTestRollbackOnly(), sendbuf,
					sendlen, 0);
			fail("Expected e.getTperrno() == Connection.TPESVCERR");
		} catch (ConnectionException e) {
			assertTrue(e.getTperrno() == Connection.TPESVCERR);
		}

		TXINFO txinfo = new TXINFO();
		int inTx = TX.tx_info(txinfo);
		log.info("inTx=" + inTx);
		assertTrue(txinfo.transaction_state == TX.TX_ROLLBACK_ONLY);
		assertTrue(TX.tx_commit() == TX.TX_ROLLBACK);
	}
}