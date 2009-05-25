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
package org.jboss.blacktie.jatmibroker.server;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Properties;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.conf.AtmiBrokerServerXML;
import org.jboss.blacktie.jatmibroker.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.transport.OrbManagement;
import org.jboss.blacktie.jatmibroker.transport.Receiver;
import org.jboss.blacktie.jatmibroker.transport.Transport;
import org.jboss.blacktie.jatmibroker.transport.TransportFactory;
import org.jboss.blacktie.jatmibroker.xatmi.BlacktieService;
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
	private Transport connection;
	private static final int DEFAULT_POOL_SIZE = 5;

	public AtmiBrokerServer(String serverName, String configurationDir)
			throws ConfigurationException, ConnectionException {
		this.serverName = serverName;

		Properties properties = null;
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

		connection = TransportFactory.loadTransportFactory(properties)
				.createTransport();
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
		try {
			log.debug("Advertising: " + serviceName);

			if (!serviceData.containsKey(serviceName)) {
				try {
					ServiceData data = new ServiceData(connection, serviceName,
							DEFAULT_POOL_SIZE, serviceClassName);
					serviceData.put(serviceName, data);
				} catch (Throwable t) {
					throw new ConnectionException(-1,
							"Could not create service factory for: "
									+ serviceName, t);
				}
			}
			log.info("Advertised: " + serviceName);
		} catch (Throwable t) {
			String message = "Could not advertise: " + serviceName;
			log.error(message, t);
			throw new ConnectionException(-1, message, t);
		}
	}

	public void tpunadvertise(String serviceName) throws ConnectionException {
		log.debug("Unadvertising: " + serviceName);
		ServiceData data = serviceData.remove(serviceName);
		if (data != null) {
			try {
				data.close();
				log.info("Unadvertised: " + serviceName);
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
		private List<Runnable> dispatchers = new ArrayList<Runnable>();

		ServiceData(Transport connection, String serviceName, int poolSize,
				String serviceClassName) throws ConnectionException,
				InstantiationException, IllegalAccessException,
				ClassNotFoundException {
			this.receiver = connection.createReceiver(serviceName);

			Class callback = Class.forName(serviceClassName);
			for (int i = 0; i < poolSize; i++) {
				dispatchers.add(new ServiceDispatcher(connection, serviceName,
						(BlacktieService) callback.newInstance(), receiver));
			}
		}

		public void close() throws ConnectionException {
			receiver.close();
			dispatchers.clear();
		}
	}
}
