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

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.JAtmiBrokerException;
import org.jboss.blacktie.jatmibroker.core.Transport;
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

public class ConnectionImpl implements Runnable, Transport {

	private static final Logger log = LogManager
			.getLogger(ConnectionImpl.class);
	private org.omg.CORBA.Object serverObject;

	private Thread callbackThread;

	private Map<String, SenderImpl> senders = new HashMap<String, SenderImpl>();
	private Map<java.lang.Integer, ReceiverImpl> temporaryQueues = new HashMap<java.lang.Integer, ReceiverImpl>();
	private OrbManagement orbManagement;

	ConnectionImpl(OrbManagement orbManagement, String serverName,
			String password) throws InvalidName, NotFound, CannotProceed,
			org.omg.CosNaming.NamingContextPackage.InvalidName,
			AdapterInactive, AlreadyBound {
		this.orbManagement = orbManagement;

		callbackThread = new Thread(this);
		callbackThread.setDaemon(true);
		callbackThread.start();
	}

	public void close() {
		Iterator<SenderImpl> iterator = senders.values().iterator();
		while (iterator.hasNext()) {
			iterator.next().close();
		}
		Iterator<ReceiverImpl> receivers = temporaryQueues.values().iterator();
		while (iterator.hasNext()) {
			receivers.next().close();
		}
		orbManagement.close();
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

	public Sender getSender(String serviceName) throws JAtmiBrokerException {
		SenderImpl sender = senders.get(serviceName);
		if (sender == null) {
			try {
				org.omg.CORBA.Object serviceFactoryObject = orbManagement
						.getNamingContext().resolve(
								orbManagement.getNamingContextExt().to_name(
										serviceName));
				sender = new SenderImpl(serviceFactoryObject, serviceName);
				senders.put(serviceName, sender);
			} catch (Throwable t) {
				throw new JAtmiBrokerException(
						"Could not load service manager proxy for: "
								+ serviceName, t);
			}
		}
		return sender;
	}

	public Sender createSender(String callback_ior) {
		org.omg.CORBA.Object serviceFactoryObject = orbManagement.getOrb()
				.string_to_object(callback_ior);
		SenderImpl sender = new SenderImpl(serviceFactoryObject, callback_ior);
		return sender;
	}

	public Receiver createReceiver(String serviceName)
			throws JAtmiBrokerException {
		log.debug("createClientCallback create client callback ");
		ReceiverImpl receiver = new ReceiverImpl(orbManagement, serviceName);
		return receiver;
	}

	public Receiver getReceiver(int id) throws JAtmiBrokerException {
		ReceiverImpl receiver = temporaryQueues.get(id);
		if (receiver == null) {
			try {
				log.debug("createClientCallback create client callback ");
				receiver = new ReceiverImpl(orbManagement.getOrb(),
						orbManagement.getRootPoa(), "TODO");
				temporaryQueues.put(id, receiver);
			} catch (Throwable t) {
				throw new JAtmiBrokerException(
						"Could not create a temporary queue", t);
			}
		}
		return receiver;
	}

	public OrbManagement getOrbManagement() {
		return orbManagement;
	}
}
