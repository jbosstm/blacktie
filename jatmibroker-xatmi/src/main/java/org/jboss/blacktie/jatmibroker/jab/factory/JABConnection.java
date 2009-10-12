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

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.jab.JABException;
import org.jboss.blacktie.jatmibroker.jab.JABSession;
import org.jboss.blacktie.jatmibroker.jab.JABTransaction;
import org.jboss.blacktie.jatmibroker.jab.TransactionException;
import org.jboss.blacktie.jatmibroker.xatmi.Buffer;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.blacktie.jatmibroker.xatmi.Response;

/**
 * The JAB connection abstraction allows invocations to services to several
 * services to be multiplexed over the same connection. It also provides a
 * factory method to begin transactions.
 * 
 * @see JABConnectionFactory
 */
public class JABConnection {
	/**
	 * The logger to debug using
	 */
	private static final Logger log = LogManager.getLogger(JABConnection.class);

	/**
	 * The list of open transactions
	 */
	private List<Transaction> transactions = new ArrayList<Transaction>();
	private Connection connection;
	private JABSession session;

	private Transaction transaction;

	/**
	 * The constructor is hidden from classes as it is intended to be used
	 * solely by the JABConnectionFactory class.
	 * 
	 * @param connection
	 *            The connection to send requests on
	 * @param session
	 *            The session to use for creating transactions
	 * 
	 * @throws JABException
	 *             If the connection cannot be established
	 */
	JABConnection(Connection connection, JABSession session)
			throws JABException {
		this.connection = connection;
		this.session = session;
	}

	/**
	 * Begin a new transaction. This transaction reference can then be used in
	 * calls to "call" and completed via the options on the JABTransaction class
	 * itself.
	 * 
	 * @param timeout
	 *            Specify a timeout for the transaction
	 * @return The transaction object
	 * @throws JABException
	 *             In case the transaction cannot be created
	 */
	public synchronized Transaction beginTransaction(int timeout)
			throws TransactionException {
		if (transaction != null) {
			return transaction;
		}
		transaction = new Transaction(this, session, timeout);
		return transaction;
	}

	/**
	 * Invoke the service specified by the first parameter. The buffer to send
	 * is provided as the second parameter. If the service should be invoked
	 * within the scope of a transaction, the last parameter should also be
	 * provided. This can be obtained via the beginTransaction method.
	 * 
	 * @param serviceName
	 *            The name of the service
	 * @param toSend
	 *            The buffer to use
	 * @param transaction
	 *            The transaction to use, may be null
	 * @return The buffer returned from the remote service
	 * 
	 * @throws TransactionException
	 *             In case the transaction cannot be handled
	 * @throws JABException
	 *             In case the service cannot be contacted
	 */
	public synchronized JABResponse call(String serviceName, JABBuffer toSend,
			Transaction transaction) throws TransactionException,
			JABException {
		log.debug("call");
		JABResponse responseMessage;
		JABTransaction tx = transaction.getJABTransaction();
		JABTransaction prev = null;

		try {
			if (tx == null) {
				log.debug("service_request tx is null");
				prev = JABTransaction.suspend();
			} else if (!tx.equals(JABTransaction.current())) {
				log.debug("service_request suspend " + prev + " resume " + tx);
				prev = JABTransaction.suspend();
				JABTransaction.resume(tx);
			} else {
				log.debug("service_request tx same as current");
			}

			Buffer request = new Buffer("X_OCTET", null);
			request.setData(toSend.getValue());
			log.debug("service_request tpcall");
			Response response = connection.tpcall(serviceName, request, request
					.getLength(), Connection.TPNOTIME);
			responseMessage = new JABResponse(response);
			log.debug("service_request responsed");
		} catch (Exception e) {
			log.warn("service_request exception: " + e.getMessage());
			throw new JABException("Could not send tpcall", e);
		} finally {
			if (prev != null) {
				if (tx != null) {
					log.debug("service_request resp: suspending current: "
							+ JABTransaction.current());
					JABTransaction.suspend();
				}

				log.debug("service_request resuming prev: " + prev);
				JABTransaction.resume(prev);
			}
		}
		return responseMessage;
	}

	/**
	 * Shutdown the connection object/
	 * 
	 * @throws JABException
	 *             In case associated state cannot be cleaned up
	 */
	public synchronized void close() throws JABException {
		Iterator<Transaction> iterator = transactions.iterator();
		while (iterator.hasNext()) {
			Transaction next = iterator.next();
			synchronized (next) {
				next.rollback();
			}
			iterator.remove();
		}
		try {
			connection.close();
		} catch (ConnectionException e) {
			throw new JABException("Could not close the connection: "
					+ e.getMessage(), e);
		}
	}

	/**
	 * Remove a completed transaction from the list.
	 * 
	 * @param transaction
	 *            The completed transaction
	 */
	synchronized void removeTransaction(Transaction transaction) {
		transactions.remove(transaction);
	}
}
