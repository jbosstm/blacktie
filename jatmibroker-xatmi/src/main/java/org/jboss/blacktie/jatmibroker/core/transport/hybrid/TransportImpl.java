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
package org.jboss.blacktie.jatmibroker.core.transport.hybrid;

import java.util.Properties;

import javax.jms.Connection;
import javax.jms.ConnectionFactory;
import javax.jms.Destination;
import javax.jms.JMSException;
import javax.jms.Session;
import javax.naming.Context;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.transport.EventListener;
import org.jboss.blacktie.jatmibroker.core.transport.OrbManagement;
import org.jboss.blacktie.jatmibroker.core.transport.Receiver;
import org.jboss.blacktie.jatmibroker.core.transport.Sender;
import org.jboss.blacktie.jatmibroker.core.transport.Transport;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.omg.CORBA.ORBPackage.InvalidName;
import org.omg.CosNaming.NamingContextPackage.AlreadyBound;
import org.omg.CosNaming.NamingContextPackage.CannotProceed;
import org.omg.CosNaming.NamingContextPackage.NotFound;
import org.omg.PortableServer.POAManagerPackage.AdapterInactive;

public class TransportImpl implements Runnable, Transport {

	private static final Logger log = LogManager.getLogger(TransportImpl.class);
	private Thread callbackThread;

	private OrbManagement orbManagement;
	private Context context;
	private Connection connection;
	private Session session;
	private Properties properties;

	TransportImpl(OrbManagement orbManagement, Context context,
			ConnectionFactory factory, Properties properties)
			throws InvalidName, NotFound, CannotProceed,
			org.omg.CosNaming.NamingContextPackage.InvalidName,
			AdapterInactive, AlreadyBound, JMSException {
		log.debug("Creating transport");
		this.orbManagement = orbManagement;

		callbackThread = new Thread(this);
		callbackThread.setDaemon(true);
		callbackThread.start();

		String username = (String) properties.get("StompConnectUsr");
		String password = (String) properties.get("StompConnectPwd");
		if (username != null) {
			connection = factory.createConnection(username, password);
		} else {
			connection = factory.createConnection();
		}
		connection.start();

		this.context = context;
		this.session = connection
				.createSession(false, Session.AUTO_ACKNOWLEDGE);

		this.properties = properties;
		log.debug("Created transport");
	}

	public void close() throws ConnectionException {
		log.debug("Close called");
		orbManagement.close();
		try {
			session.close();
		} catch (Throwable t) {
			throw new ConnectionException(-1, "Could not close the session", t);
		} finally {
			try {
				connection.close();
			} catch (Throwable t) {
				throw new ConnectionException(-1,
						"Could not close the connection", t);
			}
		}
		log.debug("Closed");
	}

	public void run() {
		log.debug("Running the orb");
		orbManagement.getOrb().run();
	}

	public Sender getSender(String serviceName) throws ConnectionException {
		log.debug("Get sender: " + serviceName);
		try {
			Destination destination = (Destination) context.lookup("/queue/"
					+ serviceName);
			log.trace("Resolved destination");
			return new JMSSenderImpl(session, destination);
		} catch (Throwable t) {
			throw new ConnectionException(-1,
					"Could not create a service sender: " + t.getMessage(), t);
		}
	}

	public Sender createSender(Object destination) {
		String callback_ior = (String) destination;
		log.debug("Creating a sender for: " + callback_ior);
		org.omg.CORBA.Object serviceFactoryObject = orbManagement.getOrb()
				.string_to_object(callback_ior);
		CorbaSenderImpl sender = new CorbaSenderImpl(serviceFactoryObject,
				callback_ior);
		log.debug("Created sender");
		return sender;
	}

	public Receiver getReceiver(String serviceName) throws ConnectionException {
		log.debug("Creating a receiver: " + serviceName);
		try {
			Destination destination = (Destination) context.lookup("/queue/"
					+ serviceName);
			log.debug("Resolved destination");
			return new JMSReceiverImpl(session, destination, properties);
		} catch (Throwable t) {
			throw new ConnectionException(-1,
					"Could not create the receiver on: " + serviceName, t);
		}

	}

	public Receiver createReceiver() throws ConnectionException {
		log.debug("Creating a receiver");
		return new CorbaReceiverImpl(null, orbManagement, properties);
	}

	public Receiver createReceiver(EventListener eventListener)
			throws ConnectionException {
		log.debug("Creating a receiver with event listener");
		return new CorbaReceiverImpl(eventListener, orbManagement, properties);
	}

	public OrbManagement getOrbManagement() {
		return orbManagement;
	}
}
