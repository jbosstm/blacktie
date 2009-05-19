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
package org.jboss.blacktie.jatmibroker.core;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Properties;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.conf.AtmiBrokerServerXML;
import org.jboss.blacktie.jatmibroker.core.corba.ReceiverImpl;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectorException;
import org.omg.CORBA.Object;
import org.omg.CORBA.Policy;
import org.omg.CosNaming.NameComponent;
import org.omg.PortableServer.POA;
import org.omg.PortableServer.ThreadPolicyValue;

import AtmiBroker.EnvVariableInfo;
import AtmiBroker.ServerInfo;
import AtmiBroker.ServerPOA;
import AtmiBroker.ServiceInfo;

public class AtmiBroker_ServerImpl extends ServerPOA {
	private static final Logger log = LogManager
			.getLogger(AtmiBroker_ServerImpl.class);
	private POA poa;
	private String serverName;
	private byte[] activate_object;
	private Map<String, ServiceQueue> serviceFactoryList = new HashMap<String, ServiceQueue>();
	private boolean bound;
	private OrbManagement orbManagement;
	private static final int DEFAULT_POOL_SIZE = 5;

	public AtmiBroker_ServerImpl() throws JAtmiBrokerException {
		Properties properties = new Properties();
		AtmiBrokerServerXML server = new AtmiBrokerServerXML(properties);
		String configDir = System.getProperty("blacktie.config.dir");
		try {
			server.getProperties(configDir);
		} catch (Exception e) {
			throw new JAtmiBrokerException("Could not load properties", e);
		}

		String domainName = properties.getProperty("blacktie.domain.name");
		String serverName = properties.getProperty("blacktie.server.name");
		int numberOfOrbArgs = Integer.parseInt(properties
				.getProperty("blacktie.orb.args"));
		List<String> orbArgs = new ArrayList<String>(numberOfOrbArgs);
		for (int i = 1; i <= numberOfOrbArgs; i++) {
			orbArgs.add(properties.getProperty("blacktie.orb.arg." + i));
		}
		String[] args = orbArgs.toArray(new String[] {});
		try {
			orbManagement = new OrbManagement(args, domainName, true);
		} catch (Throwable t) {
			throw new JAtmiBrokerException("Could not connect to orb", t);
		}
		this.serverName = serverName;
		Policy[] policiesArray = new Policy[1];
		List<Policy> policies = new ArrayList<Policy>(1);
		policies.add(orbManagement.getRootPoa().create_thread_policy(
				ThreadPolicyValue.ORB_CTRL_MODEL));
		policies.toArray(policiesArray);
		try {
			this.poa = orbManagement.getRootPoa().create_POA(serverName,
					orbManagement.getRootPoa().the_POAManager(), policiesArray);
		} catch (Throwable t) {
			throw new JAtmiBrokerException(
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
			throw new JAtmiBrokerException("Could not bind server", t);
		}
	}

	public void close() throws JAtmiBrokerException {
		Iterator<ServiceQueue> iterator = serviceFactoryList.values()
				.iterator();
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
				throw new JAtmiBrokerException("Could not unbind server", t);
			}
		}
	}

	/**
	 * Create a blacktie service with the specified name
	 * 
	 * @param serviceName
	 *            The name of the service
	 * @throws ConnectorException
	 *             If the service cannot be advertised
	 */
	public void tpadvertise(String serviceName, Class service)
			throws ConnectorException {
		try {
			log.debug("Advertising: " + serviceName);

			if (!serviceFactoryList.containsKey(serviceName)) {
				try {
					ServiceQueue atmiBroker_ServiceFactoryImpl = new ServiceQueue(
							orbManagement, serviceName, DEFAULT_POOL_SIZE,
							service);
					serviceFactoryList.put(serviceName,
							atmiBroker_ServiceFactoryImpl);
				} catch (Throwable t) {
					throw new JAtmiBrokerException(
							"Could not create service factory for: "
									+ serviceName, t);
				}
			}
			log.info("Advertised: " + serviceName);
		} catch (Throwable t) {
			String message = "Could not advertise: " + serviceName;
			log.error(message, t);
			throw new ConnectorException(-1, message, t);
		}
	}

	public void tpunadvertise(String serviceName) throws ConnectorException {
		log.debug("Unadvertising: " + serviceName);
		ServiceQueue atmiBroker_ServiceFactoryImpl = serviceFactoryList
				.remove(serviceName);
		if (atmiBroker_ServiceFactoryImpl != null) {
			atmiBroker_ServiceFactoryImpl.close();
		}
		log.info("Unadvertised: " + serviceName);
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

	private class ServiceQueue {
		private ReceiverImpl endpointQueue;
		private List<Runnable> servantCache = new ArrayList<Runnable>();

		ServiceQueue(OrbManagement orbManagement, String serviceName,
				int servantCacheSize, Class atmiBrokerCallback)
				throws JAtmiBrokerException, InstantiationException,
				IllegalAccessException {
			this.endpointQueue = new ReceiverImpl(orbManagement, serviceName);

			for (int i = 0; i < servantCacheSize; i++) {
				servantCache.add(new AtmiBroker_ServiceImpl(orbManagement,
						serviceName, atmiBrokerCallback, endpointQueue));
			}
		}

		public void close() {
			endpointQueue.close();
			servantCache.clear();
		}
	}
}
