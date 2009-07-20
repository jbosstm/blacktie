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
package org.jboss.blacktie.jatmibroker.transport;

import java.util.Properties;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;

public abstract class TransportFactory {

	private static final Logger log = LogManager
			.getLogger(TransportFactory.class);

	public static TransportFactory loadTransportFactory(String serviceName,
			Properties properties) throws ConfigurationException,
			ConnectionException {
		log.debug("Loading transport for: " + serviceName);
		String transportLibrary = (String) properties.get("blacktie."
				+ serviceName + ".transportLib");

		if (transportLibrary == null) {
			throw new ConfigurationException("Transport library for "
					+ serviceName + " was not defined");
		}
		log.debug("Transport library was: " + transportLibrary );
		// Determine the transport class to load
		String className = null;
		if (transportLibrary.contains("corba")) {
			className = "org.jboss.blacktie.jatmibroker.transport.corba.TransportFactoryImpl";
		} else if (transportLibrary.contains("stomp")) {
			className = "org.jboss.blacktie.jatmibroker.transport.jms.TransportFactoryImpl";
		} else if (transportLibrary.contains("hybrid")) {
			className = "org.jboss.blacktie.jatmibroker.transport.hybrid.TransportFactoryImpl";
		}
		if (className == null) {
			throw new ConfigurationException("TransportLibrary was not defined");
		}
		log.debug("Transport class was: " + className);

		try {
			Class clazz = Class.forName(className);
			TransportFactory newInstance = (TransportFactory) clazz
					.newInstance();
			newInstance.setProperties(properties);
			log.debug("TransportFactory was prepared");
			return newInstance;
		} catch (Throwable t) {
			throw new ConnectionException(-1,
					"Could not load the connection factory", t);
		}
	}

	protected abstract void setProperties(Properties properties)
			throws ConfigurationException;

	public abstract Transport createTransport() throws ConnectionException;
}
