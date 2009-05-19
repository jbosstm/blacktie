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
package org.jboss.blacktie.jatmibroker.core.corba;

import java.util.Properties;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.JAtmiBrokerException;
import org.jboss.blacktie.jatmibroker.core.Connection;
import org.jboss.blacktie.jatmibroker.core.ConnectionFactory;
import org.jboss.blacktie.jatmibroker.core.OrbManagement;

public class ConnectionFactoryImpl implements ConnectionFactory {

	private static final Logger log = LogManager
			.getLogger(ConnectionFactoryImpl.class);
	private OrbManagement orbManagement;

	public ConnectionFactoryImpl(Properties properties)
			throws JAtmiBrokerException {
		log.debug("Creating connection factory");
		try {
			orbManagement = new OrbManagement(properties, false);
		} catch (Throwable t) {
			throw new JAtmiBrokerException(
					"Could not create the orb management function", t);
		}
		log.debug("Created connection factory");
	}

	public Connection createConnection(String username, String password)
			throws JAtmiBrokerException {
		log.debug("Creating connection");
		ConnectionImpl instance = null;
		try {
			instance = new ConnectionImpl(orbManagement, username, password);
		} catch (Throwable t) {
			throw new JAtmiBrokerException("Could not connect to server", t);
		}
		log.debug("Created connection");
		return instance;
	}

}
