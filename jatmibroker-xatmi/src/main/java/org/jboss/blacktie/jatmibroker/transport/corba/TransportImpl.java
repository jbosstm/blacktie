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
package org.jboss.blacktie.jatmibroker.transport.corba;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.transport.OrbManagement;
import org.jboss.blacktie.jatmibroker.transport.Receiver;
import org.jboss.blacktie.jatmibroker.transport.Sender;
import org.jboss.blacktie.jatmibroker.transport.Transport;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.omg.CORBA.ORBPackage.InvalidName;
import org.omg.CosNaming.NamingContextPackage.AlreadyBound;
import org.omg.CosNaming.NamingContextPackage.CannotProceed;
import org.omg.CosNaming.NamingContextPackage.NotFound;
import org.omg.PortableServer.POAManagerPackage.AdapterInactive;

public class TransportImpl implements Runnable, Transport {

	private static final Logger log = LogManager.getLogger(TransportImpl.class);
	private org.omg.CORBA.Object serverObject;

	private Thread callbackThread;

	private Map<String, SenderImpl> senders = new HashMap<String, SenderImpl>();
	private OrbManagement orbManagement;

	TransportImpl(OrbManagement orbManagement) throws InvalidName, NotFound,
			CannotProceed, org.omg.CosNaming.NamingContextPackage.InvalidName,
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
		orbManagement.close();
	}

	public void run() {
		orbManagement.getOrb().run();
	}

	public Sender getSender(String serviceName) throws ConnectionException {
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
				throw new ConnectionException(-1,
						"Could not load service manager proxy for: "
								+ serviceName, t);
			}
		}
		return sender;
	}

	public Sender createSender(Object destination) {
		String callback_ior = (String) destination;
		org.omg.CORBA.Object serviceFactoryObject = orbManagement.getOrb()
				.string_to_object(callback_ior);
		SenderImpl sender = new SenderImpl(serviceFactoryObject, callback_ior);
		return sender;
	}

	public Receiver getReceiver(String serviceName) throws ConnectionException {
		log.debug("createClientCallback create client callback");
		return new ReceiverImpl(orbManagement, serviceName);
	}

	public Receiver createReceiver() throws ConnectionException {
		log.debug("createClientCallback create client callback");
		return new ReceiverImpl(orbManagement);
	}

	public OrbManagement getOrbManagement() {
		return orbManagement;
	}
}
