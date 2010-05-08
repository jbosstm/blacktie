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
package org.jboss.blacktie.jatmibroker.xatmi.server;

import java.util.HashMap;
import java.util.Map;
import java.util.Properties;
import java.util.StringTokenizer;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.conf.AtmiBrokerEnvXML;
import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.core.server.ServiceData;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;

/**
 * Create a server instance reading the configuration for the server defined by
 * the name.
 */
public class BlackTieServer {
	/**
	 * A logger to use
	 */
	private static final Logger log = LogManager
			.getLogger(BlackTieServer.class);

	/**
	 * The name of the server.
	 */
	private String serverName;

	/**
	 * The services currently advertised by this server.
	 */
	private Map<String, ServiceData> serviceData = new HashMap<String, ServiceData>();

	/**
	 * The properties the server was created with.
	 */
	private Properties properties;

	/**
	 * Initialize the server
	 * 
	 * @param serverName
	 *            The name of the server
	 * @throws ConfigurationException
	 *             If the server does not exist
	 * @throws ConnectionException
	 *             If the server cannot connect to the infrastructure configured
	 */
	public BlackTieServer(String serverName) throws ConfigurationException,
			ConnectionException {
		this.serverName = serverName;
		AtmiBrokerEnvXML server = new AtmiBrokerEnvXML();
		properties = server.getProperties();

		/**
		 * Launch all startup services.
		 */
		String services = (String) properties.get("blacktie." + serverName
				+ ".services");
		if (services != null) {
			StringTokenizer st = new StringTokenizer(services, ",", false);
			while (st.hasMoreElements()) {
				String serviceName = st.nextToken();
				String functionName = (String) properties.get("blacktie."
						+ serviceName + ".java_class_name");
				tpadvertise(serviceName, functionName);
			}
		}

	}

	/**
	 * Advertise a blacktie service with the specified name
	 * 
	 * @param serviceName
	 *            The name of the service
	 * @throws ConnectionException
	 *             If the service cannot be advertised
	 */
	public void tpadvertise(String serviceName, String serviceClassName)
			throws ConnectionException {
		int min = Math.min(Connection.XATMI_SERVICE_NAME_LENGTH, serviceName
				.length());
		serviceName = serviceName.substring(0, min);
		try {
			log.debug("Advertising: " + serviceName);

			if (!serviceData.containsKey(serviceName)) {
				try {
					ServiceData data = new ServiceData(properties, serviceName,
							serviceClassName);
					serviceData.put(serviceName, data);
					log.info("Advertised: " + serviceName);
				} catch (Throwable t) {
					throw new ConnectionException(Connection.TPESYSTEM,
							"Could not create service factory for: "
									+ serviceName, t);
				}
			} else {
				throw new ConnectionException(Connection.TPEMATCH,
						"Service already registered");
			}
		} catch (Throwable t) {
			String message = "Could not advertise: " + serviceName;
			log.error(message, t);
			throw new ConnectionException(Connection.TPESYSTEM, message, t);
		}
	}

	/**
	 * Unadvertise the service by name.
	 * 
	 * @param serviceName
	 *            The name of the service to unadverise.
	 * @throws ConnectionException
	 *             If the service cannot be unadvertised.
	 */
	public void tpunadvertise(String serviceName) throws ConnectionException {
		serviceName = serviceName.substring(0, Math.min(
				Connection.XATMI_SERVICE_NAME_LENGTH, serviceName.length()));
		ServiceData data = serviceData.remove(serviceName);
		if (data != null) {
			data.close();
		}
	}

	/**
	 * Shutdown the server
	 * 
	 * @throws ConnectionException
	 *             If one of the services cannot disconnect
	 */
	public void shutdown() throws ConnectionException {
		log.debug("Close server called: " + serverName);
		String[] array = new String[serviceData.size()];
		array = serviceData.keySet().toArray(array);
		for (int i = 0; i < array.length; i++) {
			tpunadvertise(array[i]);
		}
		log.debug("Close server finished: " + serverName);
	}
}
