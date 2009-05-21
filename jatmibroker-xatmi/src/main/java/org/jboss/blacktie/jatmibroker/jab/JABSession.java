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
package org.jboss.blacktie.jatmibroker.jab;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionFactory;

/**
 * The JABSession holds a connection to the XATMI server
 */
public class JABSession {
	private static final Logger log = LogManager.getLogger(JABSession.class);
	private JABSessionAttributes jabSessionAttributes;
	private Connection connection;

	/**
	 * Create a new connection to the configured XATMI server
	 * 
	 * @param aJABSessionAttributes
	 * @throws JABException
	 */
	public JABSession(JABSessionAttributes aJABSessionAttributes,
			String username, String password) throws JABException {
		super();
		log.debug("JABSession constructor");
		try {
			jabSessionAttributes = aJABSessionAttributes;
			ConnectionFactory connectionFactory = ConnectionFactory
					.getConnectionFactory();
			connection = connectionFactory.getConnection(username, password);
		} catch (Exception e) {
			throw new JABException("Error connect to domain: "
					+ jabSessionAttributes.getProperties().get(
							"blacktie.domain.name"), e);
		}
	}

	/**
	 * Close the connection with the server
	 * 
	 * @throws JABException
	 */
	public void endSession() throws JABException {
		log.debug("JABSession endSession");
		try {
			connection.close();
		} catch (ConnectionException e) {
			throw new JABException("Could not close the connection", e);
		} finally {
			connection = null;
			jabSessionAttributes = null;
		}
	}

	JABSessionAttributes getJABSessionAttributes() {
		return jabSessionAttributes;
	}

	Connection getConnection() {
		return connection;
	}
}
