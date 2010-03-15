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
package org.jboss.blacktie.jatmibroker.core.server;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Properties;
import java.util.StringTokenizer;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.conf.AtmiBrokerServerXML;
import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.core.transport.OrbManagement;
import org.jboss.blacktie.jatmibroker.core.transport.Receiver;
import org.jboss.blacktie.jatmibroker.core.transport.Transport;
import org.jboss.blacktie.jatmibroker.core.transport.TransportFactory;
import org.jboss.blacktie.jatmibroker.xatmi.BlacktieService;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;

public class AtmiBrokerServer {
	private static final Logger log = LogManager
			.getLogger(AtmiBrokerServer.class);
	private Map<String, ServiceData> serviceData = new HashMap<String, ServiceData>();
	private OrbManagement orbManagement;
	private Properties properties;
	private static final String DEFAULT_POOL_SIZE = "1";

	public AtmiBrokerServer(String serverName, String configurationDir)
			throws ConfigurationException, ConnectionException {
		AtmiBrokerServerXML server = new AtmiBrokerServerXML();
		properties = server.getProperties(configurationDir);

		try {
			orbManagement = new OrbManagement(properties, true);
		} catch (Throwable t) {
			throw new ConnectionException(-1, "Could not connect to orb", t);
		}

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

	public void close() throws ConnectionException {
		Iterator<String> names = serviceData.keySet().iterator();
		while (names.hasNext()) {
			ServiceData next = serviceData.get(names.next());
			next.close();
			names.remove();
		}
	}

	/**
	 * Create a blacktie service with the specified name
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
					ServiceData data = new ServiceData(serviceName,
							serviceClassName);
					serviceData.put(serviceName, data);
					log.info("Advertised: " + serviceName);
				} catch (Throwable t) {
					throw new ConnectionException(-1,
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
			throw new ConnectionException(-1, message, t);
		}
	}

	public void tpunadvertise(String serviceName) throws ConnectionException {
		serviceName = serviceName.substring(0, Math.min(
				Connection.XATMI_SERVICE_NAME_LENGTH, serviceName.length()));
		ServiceData data = serviceData.remove(serviceName);
		if (data != null) {
			try {
				data.close();
			} catch (Throwable t) {
				log.error("Could not unadvertise: " + serviceName, t);
			}
		}
	}

	private class ServiceData {
		private Receiver receiver;
		private List<ServiceDispatcher> dispatchers = new ArrayList<ServiceDispatcher>();
		private Transport connection;
		private String serviceName;

		ServiceData(String serviceName, String serviceClassName)
				throws ConnectionException, InstantiationException,
				IllegalAccessException, ClassNotFoundException,
				ConfigurationException {
			this.serviceName = serviceName;

			String sizeS = properties.getProperty("blacktie." + serviceName
					+ ".size", DEFAULT_POOL_SIZE);
			int size = Integer.parseInt(sizeS);

			connection = TransportFactory.loadTransportFactory(serviceName,
					properties).createTransport();
			this.receiver = connection.getReceiver(serviceName);

			Class callback = Class.forName(serviceClassName);
			for (int i = 0; i < size; i++) {
				dispatchers.add(new ServiceDispatcher(serviceName,
						(BlacktieService) callback.newInstance(), receiver));
			}
		}

		public void close() throws ConnectionException {
			log.debug("Unadvertising: " + serviceName);

			// Clean up the consumers
			Iterator<ServiceDispatcher> iterator = dispatchers.iterator();
			while (iterator.hasNext()) {
				iterator.next().startClose();
			}

			// Disconnect the receiver
			receiver.close();
			// Disconnect the transport
			connection.close();

			// Clean up the consumers
			iterator = dispatchers.iterator();
			while (iterator.hasNext()) {
				iterator.next().close();
			}
			dispatchers.clear();
			log.info("Unadvertised: " + serviceName);
		}
	}
}
