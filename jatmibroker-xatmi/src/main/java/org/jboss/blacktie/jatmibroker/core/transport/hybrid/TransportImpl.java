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
import javax.jms.Destination;
import javax.jms.JMSException;
import javax.jms.Session;
import javax.naming.Context;
import javax.naming.NameNotFoundException;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.ResponseMonitor;
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

public class TransportImpl implements Transport {

	private static final Logger log = LogManager.getLogger(TransportImpl.class);

	private OrbManagement orbManagement;
	private Context context;
	private Connection connection;
	private Session session;
	private Properties properties;
	private boolean closed;

	TransportImpl(OrbManagement orbManagement, Context context,
			Connection connection, Properties properties) throws InvalidName,
			NotFound, CannotProceed,
			org.omg.CosNaming.NamingContextPackage.InvalidName,
			AdapterInactive, AlreadyBound, JMSException {
		log.debug("Creating transport");
		this.orbManagement = orbManagement;
		this.connection = connection;

		/*
		 * String username = (String) properties.get("StompConnectUsr"); String
		 * password = (String) properties.get("StompConnectPwd"); if (username
		 * != null) { connection = factory.createConnection(username, password);
		 * } else { connection = factory.createConnection(); }
		 */

		this.connection.start();

		this.context = context;
		this.session = this.connection.createSession(false,
				Session.AUTO_ACKNOWLEDGE);

		this.properties = properties;
		log.debug("Created transport");
	}

	public void close() throws ConnectionException {
		log.debug("Close called");
		if (closed) {
			throw new ConnectionException(
					org.jboss.blacktie.jatmibroker.xatmi.Connection.TPEPROTO,
					"Transport already closed");
		}
		try {
			session.close();
		} catch (JMSException e) {
			throw new ConnectionException(
					org.jboss.blacktie.jatmibroker.xatmi.Connection.TPESYSTEM,
					"Could not close the session", e);
		} finally {
			try {
				connection.close();
			} catch (Throwable t) {
				throw new ConnectionException(
						org.jboss.blacktie.jatmibroker.xatmi.Connection.TPESYSTEM,
						"Could not close the connection", t);
			} finally {
				closed = true;
			}
		}
		log.debug("Closed");
	}

	public Sender getSender(String serviceName) throws ConnectionException {
		log.debug("Get sender: " + serviceName);
		try {
			Destination destination = (Destination) context.lookup("/queue/"
					+ serviceName);
			log.trace("Resolved destination");
			return new JMSSenderImpl(session, destination);
		} catch (NameNotFoundException e) {
			throw new ConnectionException(
					org.jboss.blacktie.jatmibroker.xatmi.Connection.TPENOENT,
					"Could not resolve destination: " + serviceName, e);
		} catch (Throwable t) {
			throw new ConnectionException(
					org.jboss.blacktie.jatmibroker.xatmi.Connection.TPESYSTEM,
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
		} catch (NameNotFoundException e) {
			throw new ConnectionException(
					org.jboss.blacktie.jatmibroker.xatmi.Connection.TPENOENT,
					"Could not resolve destination: " + serviceName, e);
		} catch (Throwable t) {
			throw new ConnectionException(
					org.jboss.blacktie.jatmibroker.xatmi.Connection.TPESYSTEM,
					"Could not create the receiver on: " + serviceName, t);
		}

	}

	public Receiver createReceiver(int cd, ResponseMonitor responseMonitor)
			throws ConnectionException {
		log.debug("Creating a receiver");
		return new CorbaReceiverImpl(null, orbManagement, properties, cd,
				responseMonitor);
	}

	public Receiver createReceiver(EventListener eventListener)
			throws ConnectionException {
		log.debug("Creating a receiver with event listener");
		return new CorbaReceiverImpl(eventListener, orbManagement, properties,
				-1, null);
	}

	public OrbManagement getOrbManagement() {
		return orbManagement;
	}
}
