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
import org.jboss.blacktie.jatmibroker.transport.OrbManagement;
import org.jboss.blacktie.jatmibroker.transport.Receiver;
import org.jboss.blacktie.jatmibroker.transport.Transport;
import org.jboss.blacktie.jatmibroker.transport.TransportFactory;
import org.jboss.blacktie.jatmibroker.xatmi.BlacktieService;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.omg.CORBA.Object;
import org.omg.CORBA.Policy;
import org.omg.CosNaming.NameComponent;
import org.omg.PortableServer.POA;
import org.omg.PortableServer.ThreadPolicyValue;

import AtmiBroker.EnvVariableInfo;
import AtmiBroker.ServerInfo;
import AtmiBroker.ServerPOA;
import AtmiBroker.ServiceInfo;

public class AtmiBrokerServer extends ServerPOA {
	private static final Logger log = LogManager
			.getLogger(AtmiBrokerServer.class);
	private POA poa;
	private String serverName;
	private byte[] activate_object;
	private Map<String, ServiceData> serviceData = new HashMap<String, ServiceData>();
	private boolean bound;
	private OrbManagement orbManagement;
	private Properties properties;
	private static final String DEFAULT_POOL_SIZE = "1";

	public AtmiBrokerServer(String serverName, String configurationDir)
			throws ConfigurationException, ConnectionException {
		this.serverName = serverName;

		AtmiBrokerServerXML server = new AtmiBrokerServerXML(serverName);
		properties = server.getProperties(configurationDir);

		try {
			orbManagement = new OrbManagement(properties, true);
		} catch (Throwable t) {
			throw new ConnectionException(-1, "Could not connect to orb", t);
		}

		Policy[] policiesArray = new Policy[1];
		List<Policy> policies = new ArrayList<Policy>(1);
		policies.add(orbManagement.getRootPoa().create_thread_policy(
				ThreadPolicyValue.ORB_CTRL_MODEL));
		policies.toArray(policiesArray);
		try {
			this.poa = orbManagement.getRootPoa().create_POA(serverName,
					orbManagement.getRootPoa().the_POAManager(), policiesArray);
		} catch (Throwable t) {
			throw new ConnectionException(-1,
					"Server appears to be already running", t);
		}

		try {
			activate_object = poa.activate_object(this);
			NameComponent[] name = orbManagement.getNamingContextExt().to_name(
					serverName);
			Object servant_to_reference = poa.servant_to_reference(this);
			orbManagement.getNamingContext().bind(name, servant_to_reference);
			bound = true;
		} catch (Throwable t) {
			throw new ConnectionException(-1, "Could not bind server", t);
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
		Iterator<ServiceData> iterator = serviceData.values().iterator();
		while (iterator.hasNext()) {
			iterator.next().close();
			iterator.remove();
		}

		if (bound) {
			try {
				NameComponent[] name = orbManagement.getNamingContextExt()
						.to_name(serverName);
				orbManagement.getNamingContext().unbind(name);
				poa.deactivate_object(activate_object);
				bound = false;
			} catch (Throwable t) {
				throw new ConnectionException(-1, "Could not unbind server", t);
			}
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

	public ServiceInfo[] get_all_service_info() {
		log.error("NO-OP get_all_service_info");
		// TODO Auto-generated method stub
		return null;
	}

	public EnvVariableInfo[] get_environment_variable_info() {
		log.error("NO-OP get_environment_variable_info");
		// TODO Auto-generated method stub
		return null;
	}

	public int get_queue_log(String queue_name) {
		log.error("NO-OP get_queue_log");
		// TODO Auto-generated method stub
		return 0;
	}

	public ServerInfo get_server_info() {
		log.error("NO-OP get_server_info");
		// TODO Auto-generated method stub
		return null;
	}

	public ServiceInfo get_service_info(String service_name) {
		log.error("NO-OP get_service_info");
		// TODO Auto-generated method stub
		return null;
	}

	public void server_done() {
		log.error("NO-OP server_done");
		// TODO Auto-generated method stub

	}

	public short server_init() {
		log.error("NO-OP server_init");
		// TODO Auto-generated method stub
		return 0;
	}

	public void set_environment_descriptor(String xml_descriptor) {
		log.error("NO-OP set_environment_descriptor");
		// TODO Auto-generated method stub

	}

	public void set_server_descriptor(String xml_descriptor) {
		log.error("NO-OP set_server_descriptor");
		// TODO Auto-generated method stub

	}

	public void set_service_descriptor(String serviceName, String xml_descriptor) {
		log.error("NO-OP set_service_descriptor");
		// TODO Auto-generated method stub

	}

	public void start_service(String service_name) {
		log.error("NO-OP start_service");
		// TODO Auto-generated method stub

	}

	public void stop_service(String service_name) {
		log.error("NO-OP stop_service");
		// TODO Auto-generated method stub

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
				dispatchers.add(new ServiceDispatcher(connection, serviceName,
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
