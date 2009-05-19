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

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Properties;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.JAtmiBrokerException;
import org.jboss.blacktie.jatmibroker.core.Connection;
import org.jboss.blacktie.jatmibroker.core.OrbManagement;
import org.jboss.blacktie.jatmibroker.core.Receiver;
import org.jboss.blacktie.jatmibroker.core.Sender;
import org.omg.CORBA.ORBPackage.InvalidName;
import org.omg.CosNaming.NamingContextPackage.AlreadyBound;
import org.omg.CosNaming.NamingContextPackage.CannotProceed;
import org.omg.CosNaming.NamingContextPackage.NotFound;
import org.omg.CosTransactions.TransactionFactory;
import org.omg.CosTransactions.TransactionFactoryHelper;
import org.omg.PortableServer.POAManagerPackage.AdapterInactive;

public class ConnectionImpl implements Runnable, Connection {

	private static final Logger log = LogManager
			.getLogger(ConnectionImpl.class);
	private org.omg.CORBA.Object serverObject;

	private Thread callbackThread;

	private Map<String, Sender> proxies = new HashMap<String, Sender>();
	private String serverName;
	private Map<java.lang.Integer, ReceiverImpl> temporaryQueues = new HashMap<java.lang.Integer, ReceiverImpl>();
	static OrbManagement orbManagement;

	public synchronized static Connection createConnection(
			Properties properties, String userName, String userPassword)
			throws JAtmiBrokerException {
		ConnectionImpl instance = null;
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

			instance = new ConnectionImpl(args, domainName, serverName);
		} catch (Throwable t) {
			throw new JAtmiBrokerException("Could not connect to server", t);
		}
		return instance;
	}

	protected ConnectionImpl(String[] args, String domainName, String serverName)
			throws InvalidName, NotFound, CannotProceed,
			org.omg.CosNaming.NamingContextPackage.InvalidName,
			AdapterInactive, AlreadyBound {
		this.serverName = serverName;
		orbManagement = new OrbManagement(args, domainName, false);
		log.debug("about to resolve '" + serverName + "'");

		serverObject = orbManagement.getNamingContext().resolve(
				orbManagement.getNamingContextExt().to_name(serverName));
		log.debug("Server Object is " + serverObject);
		log.debug("Server class is " + serverObject.getClass().getName());

		callbackThread = new Thread(this);
		callbackThread.setDaemon(true);
		callbackThread.start();
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
		Iterator<Sender> iterator = proxies.values().iterator();
		while (iterator.hasNext()) {
			iterator.next().close();
		}
		orbManagement.close();
	}

	public Sender getSender(String serviceName) throws JAtmiBrokerException {
		Sender proxy = proxies.get(serviceName);
		if (proxy == null) {
			try {
				org.omg.CORBA.Object serviceFactoryObject = orbManagement
						.getNamingContext().resolve(
								orbManagement.getNamingContextExt().to_name(
										serviceName));
				proxy = new SenderImpl(serviceFactoryObject, serviceName);
				proxies.put(serviceName, proxy);
			} catch (Throwable t) {
				throw new JAtmiBrokerException(
						"Could not load service manager proxy for: "
								+ serviceName, t);
			}
		}
		return proxy;
	}

	public Sender createSender(String callback_ior) {
		org.omg.CORBA.Object serviceFactoryObject = orbManagement.getOrb()
				.string_to_object(callback_ior);
		SenderImpl instance = new SenderImpl(serviceFactoryObject, callback_ior);
		return instance;
	}

	public Receiver createReceiver(String serviceName)
			throws JAtmiBrokerException {
		log.debug("createClientCallback create client callback ");
		ReceiverImpl receiver = new ReceiverImpl(orbManagement, serviceName);
		return receiver;
	}

	public Receiver getReceiver(int id) throws JAtmiBrokerException {
		ReceiverImpl proxy = temporaryQueues.get(id);
		if (proxy == null) {
			try {
				log.debug("createClientCallback create client callback ");
				proxy = new ReceiverImpl(orbManagement.getOrb(), orbManagement
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
