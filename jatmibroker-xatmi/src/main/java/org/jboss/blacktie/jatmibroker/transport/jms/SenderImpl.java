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

import javax.jms.BytesMessage;
import javax.jms.Destination;
import javax.jms.JMSException;
import javax.jms.MessageProducer;
import javax.jms.Queue;
import javax.jms.Session;
import javax.jms.Topic;
import javax.naming.NamingException;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.transport.JtsTransactionImple;
import org.jboss.blacktie.jatmibroker.transport.Sender;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;

public class SenderImpl implements Sender {
	private static final Logger log = LogManager.getLogger(SenderImpl.class);
	private MessageProducer sender;
	private Session session;
	private String name;
	boolean service;
	private boolean closed;
	private Destination destination;

	SenderImpl(Session session, Destination destination)
			throws NamingException, JMSException {
		this.session = session;
		sender = session.createProducer(destination);
		if (destination instanceof Queue) {
			this.name = ((Queue) destination).getQueueName();
		} else {
			this.name = ((Topic) destination).getTopicName();
		}
		this.destination = destination;
		service = true;
		log.debug("Sender Created: " + name);
	}

	public void send(Object replyTo, short rval, int rcode, byte[] data,
			int len, int correlationId, int flags) throws ConnectionException {
		if (closed) {
			throw new ConnectionException(Connection.TPEPROTO, "Sender closed");
		}
		try {
			if (name != null) {
				log.debug("Sender sending: " + name);
			}
			BytesMessage message = session.createBytesMessage();
			String ior = JtsTransactionImple.getTransactionIOR();

			message.setStringProperty("messagecontrol", ior);
			log.debug("Sender sending IOR: " + ior);
			if (replyTo != null) {
				message.setJMSReplyTo((Destination) replyTo);
				// TODOmessage.setStringProperty("reply-to", replyTo);
			}
			if (service) {
				message.setStringProperty("serviceName", name);
			}
			message.setStringProperty("messagecorrelationId", String
					.valueOf(correlationId));
			message.setStringProperty("messageflags", String.valueOf(flags));
			message.setStringProperty("messagerval", String.valueOf(rval));
			message.setStringProperty("messagercode", String.valueOf(rcode));

			byte[] toSend = new byte[len + 1];
			System.arraycopy(data, 0, toSend, 0, len);
			message.writeBytes(toSend, 0, toSend.length);
			sender.send(message);
		} catch (Throwable t) {
			throw new ConnectionException(-1, "Could not send the message", t);
		}
	}

	public void close() throws ConnectionException {
		try {
			log.debug("Sender closing: " + name);
			sender.close();
			sender = null;
			closed = true;
			log.debug("Sender closed: " + name);
		} catch (Throwable t) {
			throw new ConnectionException(-1, "Could not send the message", t);
		}
	}

	public Object getSendTo() {
		return destination;
	}
}
