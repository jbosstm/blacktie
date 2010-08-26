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

import javax.jms.Destination;
import javax.jms.JMSException;
import javax.jms.Session;
import javax.naming.NameNotFoundException;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.ResponseMonitor;
import org.jboss.blacktie.jatmibroker.core.transport.EventListener;
import org.jboss.blacktie.jatmibroker.core.transport.JMSManagement;
import org.jboss.blacktie.jatmibroker.core.transport.OrbManagement;
import org.jboss.blacktie.jatmibroker.core.transport.Receiver;
import org.jboss.blacktie.jatmibroker.core.transport.Sender;
import org.jboss.blacktie.jatmibroker.core.transport.Transport;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;

public class TransportImpl implements Transport {

	private static final Logger log = LogManager.getLogger(TransportImpl.class);
	private OrbManagement orbManagement;
	private JMSManagement jmsManagement;
	private Properties properties;
	private TransportFactoryImpl transportFactoryImpl;
	private Session session;
	private boolean closed;

	TransportImpl(OrbManagement orbManagement, JMSManagement jmsManagement,
			Properties properties, TransportFactoryImpl transportFactoryImpl)
			throws JMSException {
		log.debug("Creating transport");
		this.orbManagement = orbManagement;
		this.jmsManagement = jmsManagement;
		this.properties = properties;
		this.transportFactoryImpl = transportFactoryImpl;

		this.session = jmsManagement.createSession();
		log.debug("Created transport");
	}

	public void close() throws ConnectionException {
		log.debug("Close called: " + this);
		if (!closed) {
			try {
				session.close();
			} catch (JMSException e) {
				throw new ConnectionException(
						org.jboss.blacktie.jatmibroker.xatmi.Connection.TPESYSTEM,
						"Could not close the session", e);
			}
			transportFactoryImpl.removeTransport(this);
			closed = true;
		}
		log.debug("Closed: " + this);
	}

	public Sender getSender(String serviceName, boolean conversational)
			throws ConnectionException {
		if (closed) {
			throw new ConnectionException(Connection.TPEPROTO, "Already closed");
		}
		log.debug("Get sender: " + serviceName);
		try {
			Destination destination = jmsManagement.lookup(serviceName,
					conversational);
			log.trace("Resolved destination");
			return new JMSSenderImpl(orbManagement, session, destination);
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

	public Sender createSender(Object destination) throws ConnectionException {
		if (closed) {
			throw new ConnectionException(Connection.TPEPROTO, "Already closed");
		}
		String callback_ior = (String) destination;
		log.debug("Creating a sender for: " + callback_ior);
		org.omg.CORBA.Object serviceFactoryObject = orbManagement.getOrb()
				.string_to_object(callback_ior);
		CorbaSenderImpl sender = new CorbaSenderImpl(serviceFactoryObject,
				callback_ior);
		log.debug("Created sender");
		return sender;
	}

	public Receiver getReceiver(String serviceName, boolean conversational)
			throws ConnectionException {
		if (closed) {
			throw new ConnectionException(Connection.TPEPROTO, "Already closed");
		}
		log.debug("Creating a receiver: " + serviceName);
		try {
			Destination destination = jmsManagement.lookup(serviceName,
					conversational);
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
		if (closed) {
			throw new ConnectionException(Connection.TPEPROTO, "Already closed");
		}
		log.debug("Creating a receiver");
		return new CorbaReceiverImpl(orbManagement, properties, cd,
				responseMonitor);
	}

	public Receiver createReceiver(EventListener eventListener)
			throws ConnectionException {
		if (closed) {
			throw new ConnectionException(Connection.TPEPROTO, "Already closed");
		}
		log.debug("Creating a receiver with event listener");
		return new CorbaReceiverImpl(eventListener, orbManagement, properties);
	}
}
