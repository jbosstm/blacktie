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

import org.jboss.blacktie.jatmibroker.jab.JABException;

/**
 * The JABTransaction provides the programmer access to the underlying
 * transaction object it can be committed or rolled back at most once.
 * 
 * @see JABConnection
 */
public class JABTransaction {

	/**
	 * The connection that created the transaction, the transaction must be
	 * disassociated from the connection upon completion
	 */
	private JABConnection connection;

	/**
	 * Any timeout for the transaction can be provided here
	 */
	private int timeout;

	/**
	 * The constructor is hidden from the programmer as it should be created
	 * using the factory method beginTransaction of JABConnection
	 * 
	 * @param connection
	 *            The connection that initialised this transaction
	 * @param timeout
	 *            The timeout for the transaction
	 * @throws JABException
	 *             In case the transaction cannot be created
	 */
	JABTransaction(JABConnection connection, int timeout) throws JABException {
		this.connection = connection;
		this.timeout = timeout;
	}

	/**
	 * Commit the work performed within the scope of this transaction
	 * 
	 * @throws JABException
	 *             In case the transaction cannot be committed
	 */
	public synchronized void commit() throws JABException {
		connection.removeTransaction(this);
	}

	/**
	 * Discard the work associated with this transaction
	 * 
	 * @throws JABException
	 *             In case the transaction cannot be rolled back successfully
	 */
	public synchronized void rollback() throws JABException {
		connection.removeTransaction(this);
	}
}
