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
package org.jboss.blacktie.jatmibroker.transport.jms;

import javax.jms.Connection;
import javax.jms.ConnectionFactory;
import javax.jms.Destination;
import javax.jms.JMSException;
import javax.jms.Session;
import javax.jms.TemporaryQueue;
import javax.naming.Context;

import org.jboss.blacktie.jatmibroker.transport.Receiver;
import org.jboss.blacktie.jatmibroker.transport.Sender;
import org.jboss.blacktie.jatmibroker.transport.Transport;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;

public class TransportImpl implements Transport {

	private Context context;
	private Connection connection;
	private Session session;

	TransportImpl(Context context, ConnectionFactory connectionFactory, Properties properties) throws JMSException {
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
	}

	public void close() throws ConnectionException {
		try {
			session.close();
		} catch (Throwable t) {
			throw new ConnectionException(-1, "Could not close the session",
					t);
		} finally {
			try {
				connection.close();
			} catch (Throwable t) {
				throw new ConnectionException(-1, "Could not close the connection",
						t);
			}
		}
	}

	public Sender getSender(String serviceName) throws ConnectionException {
		try {
			Destination destination = (Destination) context.lookup("/queue/"
					+ serviceName);
			return new SenderImpl(session, destination);
		} catch (Throwable t) {
			throw new ConnectionException(-1,
					"Could not create a service sender: " + t.getMessage(), t);
		}
	}

	public Sender createSender(Object callback_ior) throws ConnectionException {
		try {
			TemporaryQueue destination = (TemporaryQueue) callback_ior;
			return new SenderImpl(session, destination);
		} catch (Throwable t) {
			throw new ConnectionException(-1,
					"Could not create a temporary sender", t);
		}
	}

	public Receiver getReceiver(String serviceName) throws ConnectionException {
		try {
			Destination destination = (Destination) context.lookup("/queue/"
					+ serviceName);
			return new ReceiverImpl(session, destination);
		} catch (Throwable t) {
			throw new ConnectionException(-1,
					"Could not create the receiver on: " + serviceName, t);
		}
	}

	public Receiver createReceiver() throws ConnectionException {
		try {
			return new ReceiverImpl(session);
		} catch (Throwable t) {
			throw new ConnectionException(-1,
					"Could not create the temporary receiver", t);
		}
	}
}
