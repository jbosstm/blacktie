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
import org.jboss.blacktie.jatmibroker.core.administration.BTServerAdministration;
import org.jboss.blacktie.jatmibroker.core.proxy.AtmiBrokerServer;
import org.jboss.blacktie.jatmibroker.core.proxy.ServiceQueue;
import org.omg.CORBA.Object;
import org.omg.CORBA.ORBPackage.InvalidName;
import org.omg.CosNaming.NamingContextPackage.AlreadyBound;
import org.omg.CosNaming.NamingContextPackage.CannotProceed;
import org.omg.CosNaming.NamingContextPackage.NotFound;
import org.omg.CosTransactions.TransactionFactory;
import org.omg.CosTransactions.TransactionFactoryHelper;
import org.omg.PortableServer.POAManagerPackage.AdapterInactive;

import AtmiBroker.Server;
import AtmiBroker.ServerHelper;

public class AtmiBrokerServerProxy implements BTServerAdministration,
		AtmiBrokerServer, Runnable {

	private static final Logger log = LogManager
			.getLogger(AtmiBrokerServerProxy.class);
	private Server server;
	private org.omg.CORBA.Object serverObject;

	private Thread callbackThread;

	private Map<String, ServiceQueue> proxies = new HashMap<String, ServiceQueue>();
	private String serverName;
	private Map<java.lang.Integer, EndpointQueue> temporaryQueues = new HashMap<java.lang.Integer, EndpointQueue>();
	static OrbManagement orbManagement;

	public synchronized static AtmiBrokerServer getProxy(Properties properties,
			String userName, String userPassword) throws JAtmiBrokerException {
		AtmiBrokerServerProxy instance = null;
		try {
			String domainName = properties.getProperty("blacktie.domain.name");
			String serverName = properties.getProperty("blacktie.server.name");
			String property = properties.getProperty("blacktie.orb.args");
			int numberOfOrbArgs = Integer.parseInt(property);
			List<String> orbArgs = new ArrayList<String>(numberOfOrbArgs);
			for (int i = 1; i <= numberOfOrbArgs; i++) {
				orbArgs.add(properties.getProperty("blacktie.orb.arg." + i));
			}
			String[] args = orbArgs.toArray(new String[] {});

			instance = new AtmiBrokerServerProxy(args, domainName, serverName);
		} catch (Throwable t) {
			throw new JAtmiBrokerException("Could not connect to server", t);
		}
		return instance;
	}

	public synchronized static BTServerAdministration getAdministration(
			String[] args, String namingContextExt, String serverName)
			throws JAtmiBrokerException {
		try {
			return new AtmiBrokerServerProxy(args, namingContextExt, serverName);
		} catch (Throwable t) {
			throw new JAtmiBrokerException("Could not load administration", t);
		}
	}

	protected AtmiBrokerServerProxy(String[] args, String domainName,
			String serverName) throws InvalidName, NotFound, CannotProceed,
			org.omg.CosNaming.NamingContextPackage.InvalidName,
			AdapterInactive, AlreadyBound {
		this.serverName = serverName;
		orbManagement = new OrbManagement(args, domainName, false);
		server = null;
		log.debug("about to resolve '" + serverName + "'");

		serverObject = orbManagement.getNamingContext().resolve(
				orbManagement.getNamingContextExt().to_name(serverName));
		log.debug("Server Object is " + serverObject);
		log.debug("Server class is " + serverObject.getClass().getName());

		server = ServerHelper.narrow(serverObject);
		log.debug("Server is " + server);
		callbackThread = new Thread(this);
		callbackThread.setDaemon(true);
		callbackThread.start();
	}

	public short server_init() {
		return server.server_init();
	}

	public void server_done() {
		server.server_done();
	}

	public AtmiBroker.ServerInfo get_server_info() {
		return server.get_server_info();
	}

	public AtmiBroker.ServiceInfo[] get_all_service_info() {
		return server.get_all_service_info();
	}

	public AtmiBroker.EnvVariableInfo[] get_environment_variable_info() {
		return server.get_environment_variable_info();
	}

	public void set_server_descriptor(String xml_descriptor) {
		server.set_server_descriptor(xml_descriptor);
	}

	public void set_service_descriptor(String service_name,
			String xml_descriptor) {
		server.set_service_descriptor(service_name, xml_descriptor);
	}

	public void set_environment_descriptor(String xml_descriptor) {
		server.set_environment_descriptor(xml_descriptor);
	}

	public void stop_service(String service_name) {
		server.stop_service(service_name);
	}

	public void start_service(String service_name) {
		server.start_service(service_name);
	}

	public Object resolve(String name) throws NotFound, CannotProceed,
			org.omg.CosNaming.NamingContextPackage.InvalidName {
		return orbManagement.getNamingContext().resolve(
				orbManagement.getNamingContextExt().to_name(name));
	}

	public void run() {
		orbManagement.getOrb().run();
	}

	public TransactionFactory getTransactionFactory(
			String transactionManagerServiceName) throws NotFound,
			CannotProceed, org.omg.CosNaming.NamingContextPackage.InvalidName {
		org.omg.CORBA.Object aObject = orbManagement.getNamingContextExt()
				.resolve_str(transactionManagerServiceName);
		return TransactionFactoryHelper.narrow(aObject);
	}

	public void close() {
		Iterator<ServiceQueue> iterator = proxies.values().iterator();
		while (iterator.hasNext()) {
			iterator.next().close();
		}
		orbManagement.close();
	}

	public ServiceQueue getServiceQueue(String serviceName)
			throws JAtmiBrokerException {
		ServiceQueue proxy = proxies.get(serviceName);
		if (proxy == null) {
			try {
				proxy = AtmiBrokerServiceFactoryImpl
						.getProxy(this, serviceName);
				proxies.put(serviceName, proxy);
			} catch (Throwable t) {
				throw new JAtmiBrokerException(
						"Could not load service manager proxy for: "
								+ serviceName, t);
			}
		}
		return proxy;
	}

	public org.jboss.blacktie.jatmibroker.core.proxy.Queue getEndpointQueue(
			int id) throws JAtmiBrokerException {

		EndpointQueue proxy = temporaryQueues.get(id);
		if (proxy == null) {
			try {
				log.debug("createClientCallback create client callback ");
				proxy = new EndpointQueue(orbManagement.getOrb(), orbManagement
						.getRootPoa(), serverName);
				temporaryQueues.put(id, proxy);
			} catch (Throwable t) {
				throw new JAtmiBrokerException(
						"Could not create a temporary queue", t);
			}
		}

		return proxy;
	}

	public OrbManagement getOrbManagement() {
		return orbManagement;
	}
}
