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

import org.jboss.blacktie.jatmibroker.jab.JABException;
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
	 * The list of open transactions
	 */
	private List<JABTransaction> transactions = new ArrayList<JABTransaction>();

	/**
	 * The constructor is hidden from classes as it is intended to be used
	 * solely by the JABConnectionFactory class.
	 * 
	 * @throws JABException
	 *             If the connection cannot be established
	 */
	JABConnection() throws JABException {
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
	public synchronized JABTransaction beginTransaction(int timeout)
			throws JABException {
		return new JABTransaction(this, timeout);
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
	 * @throws JABException
	 *             In case the service cannot be contacted
	 */
	public synchronized JABResponse call(String serviceName, JABBuffer toSend,
			JABTransaction transaction) throws JABException {
		return new JABResponse(new Response((short) -1, -1, null, -1, -1));
	}

	/**
	 * Shutdown the connection object/
	 * 
	 * @throws JABException
	 *             In case associated state cannot be cleaned up
	 */
	public synchronized void close() throws JABException {
		Iterator<JABTransaction> iterator = transactions.iterator();
		while (iterator.hasNext()) {
			JABTransaction next = iterator.next();
			synchronized (next) {
				next.rollback();
			}
			iterator.remove();
		}
	}

	/**
	 * Remove a completed transaction from the list.
	 * 
	 * @param transaction
	 *            The completed transaction
	 */
	synchronized void removeTransaction(JABTransaction transaction) {
		transactions.remove(transaction);
	}
}
