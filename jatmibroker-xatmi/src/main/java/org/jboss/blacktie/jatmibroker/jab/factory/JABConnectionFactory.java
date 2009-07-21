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

import java.util.HashMap;
import java.util.Map;

import org.jboss.blacktie.jatmibroker.jab.JABException;

/**
 * The connection factory is the entry point into the JAB API for client
 * applications. It is a singleton class which is accessed via the
 * JABConnectionFactory.getInstance() method. Connections obtained via the
 * getConnection(String) method, should be closed via the
 * closeConnection(String) method.
 */
public class JABConnectionFactory {

	/**
	 * The instance
	 */
	private static JABConnectionFactory instance;

	/**
	 * The list of open connections
	 */
	private Map<String, JABConnection> connections = new HashMap<String, JABConnection>();

	/**
	 * Obtain a reference to the single instance of JABConnectionFactory per
	 * classloader.
	 * 
	 * @return The instance
	 * @throws JABException
	 *             In case the connection factory cannot be loaded
	 */
	public synchronized static JABConnectionFactory getInstance()
			throws JABException {
		if (instance == null) {
			instance = new JABConnectionFactory();
		}
		return instance;
	}

	/**
	 * The constructor is not intended to be invoked by clients of the factory
	 * 
	 * @throws JABException
	 *             In case the connection factory cannot be created
	 */
	private JABConnectionFactory() throws JABException {
	}

	/**
	 * Obtain a reference to the connection identified by the parameter
	 * 
	 * @param connectionName
	 *            The name of the connection to retrieve
	 * @return The single connection named connectionName
	 * @throws JABException
	 *             If the connection cannot be created
	 */
	public JABConnection getConnection(String connectionName)
			throws JABException {
		JABConnection toReturn = connections.get(connectionName);
		if (toReturn == null) {
			toReturn = new JABConnection();
			connections.put(connectionName, toReturn);
		}
		return toReturn;
	}

	/**
	 * Close the JAB connection to the server
	 * 
	 * @param connectionName
	 *            The name of the connection to close
	 * @throws JABException
	 *             In case the connection cannot be closed
	 */
	public void closeConnection(String connectionName) throws JABException {
		JABConnection connection = connections.remove(connectionName);
		if (connection != null) {
			connection.close();
		}
	}
}
