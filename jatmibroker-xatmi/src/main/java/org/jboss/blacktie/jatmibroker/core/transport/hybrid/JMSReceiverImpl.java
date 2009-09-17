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

import javax.jms.BytesMessage;
import javax.jms.Destination;
import javax.jms.JMSException;
import javax.jms.MessageConsumer;
import javax.jms.Queue;
import javax.jms.Session;
import javax.jms.TemporaryQueue;
import javax.naming.NamingException;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.transport.Message;
import org.jboss.blacktie.jatmibroker.core.transport.Receiver;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;

public class JMSReceiverImpl implements Receiver {
	private static final Logger log = LogManager
			.getLogger(JMSReceiverImpl.class);
	private Queue destination;
	private MessageConsumer receiver;
	private boolean isTemporary;
	private int timeout = 0;

	JMSReceiverImpl(Session session, Properties properties) throws JMSException {
		destination = session.createTemporaryQueue();
		receiver = session.createConsumer(destination);
		isTemporary = true;
		timeout = Integer.parseInt(properties.getProperty("DestinationTimeout",
				"10")) * 1000;
		log.debug("Creating a consumer on: " + destination.getQueueName());
	}

	JMSReceiverImpl(Session session, Destination destination,
			Properties properties) throws JMSException, NamingException {
		this.destination = (Queue) destination;
		receiver = session.createConsumer(destination);
		timeout = Integer.parseInt(properties.getProperty("DestinationTimeout",
				"2")) * 1000;
		log.debug("Creating a consumer on: " + this.destination.getQueueName());
	}

	public Object getReplyTo() throws ConnectionException {
		if (isTemporary) {
			return destination;
		} else {
			return null;
		}
	}

	public Message receive(long flagsIn) throws ConnectionException {
		try {
			log.debug("Receiving from: " + destination.getQueueName());
			javax.jms.Message message = receiver.receive(timeout);
			if (message != null) {
				log.debug("Received from: " + destination.getQueueName());
				log.debug("getJMSExpiration is " + message.getJMSExpiration());
				String controlIOR = message.getStringProperty("messagecontrol");
				BytesMessage bytesMessage = ((BytesMessage) message);
				// TODO String replyTo = message.getStringProperty("reply-to");
				String replyTo = message.getStringProperty("messagereplyto");
				int len = (int) bytesMessage.getBodyLength();
				String serviceName = message.getStringProperty("servicename");
				int flags = new Integer(message
						.getStringProperty("messageflags"));
				int cd = new Integer(message
						.getStringProperty("messagecorrelationId"));

				String type = message.getStringProperty("messagetype");
				String subtype = message.getStringProperty("messagesubtype");
				log.debug("type: " + type + " subtype: " + subtype);

				org.jboss.blacktie.jatmibroker.core.transport.Message toProcess = new org.jboss.blacktie.jatmibroker.core.transport.Message();
				toProcess.type = type;
				toProcess.subtype = subtype;
				toProcess.replyTo = replyTo;
				toProcess.serviceName = serviceName;
				toProcess.flags = flags;
				toProcess.cd = cd;
				toProcess.len = len - 1;
				if (toProcess.len == 0 && toProcess.type == "") {
					toProcess.data = null;
				} else {
					toProcess.data = new byte[toProcess.len];
					bytesMessage.readBytes(toProcess.data);
				}
				toProcess.control = controlIOR;
				return toProcess;
			}
			throw new ConnectionException(Connection.TPETIME,
					"Did not receive a message");
		} catch (JMSException t) {
			throw new ConnectionException(-1, "Couldn't receive the message", t);
		}
	}

	public void close() throws ConnectionException {
		try {
			receiver.close();
			if (isTemporary) {
				log.debug("Deleting: " + destination.getQueueName());
				((TemporaryQueue) destination).delete();
				log.debug("Deleted: " + destination.getQueueName());
			}
		} catch (Throwable t) {
			throw new ConnectionException(-1, "Could not delete the queue", t);
		}
	}
}
