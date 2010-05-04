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
	private static int pad = 0;

	JMSReceiverImpl(Session session, Properties properties) throws JMSException {
		destination = session.createTemporaryQueue();
		receiver = session.createConsumer(destination);
		isTemporary = true;
		timeout = Integer.parseInt(properties.getProperty("DestinationTimeout",
				"10")) * 1000;
		log.debug("Creating a consumer on: " + this.destination.getQueueName()
				+ " with timeout: " + timeout);
	}

	JMSReceiverImpl(Session session, Destination destination,
			Properties properties) throws JMSException, NamingException {
		this.destination = (Queue) destination;
		receiver = session.createConsumer(destination);
		timeout = Integer.parseInt(properties.getProperty("DestinationTimeout",
				"2")) * 1000;
		log.debug("Creating a consumer on: " + this.destination.getQueueName()
				+ " with timeout: " + timeout);
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
			javax.jms.Message message = receiver.receive(determineTimeout(flagsIn));
			if (message != null) {
				BytesMessage bytesMessage = ((BytesMessage) message);
				log.debug("Received from: " + destination.getQueueName());
				log.debug("getJMSExpiration is " + message.getJMSExpiration());
				org.jboss.blacktie.jatmibroker.core.transport.Message toProcess = convertFromBytesMessage(bytesMessage);
				return toProcess;
			}
			throw new ConnectionException(Connection.TPETIME,
					"Did not receive a message");
		} catch (JMSException t) {
			throw new ConnectionException(-1, "Couldn't receive the message", t);
		}
	}

	public void close() throws ConnectionException {
		log.debug("close");
		try {
			log.debug("closing consumer");
			receiver.close();
			log.debug("consumer closed");
			if (isTemporary) {
				log.debug("Deleting: " + destination.getQueueName());
				((TemporaryQueue) destination).delete();
				log.debug("Deleted: " + destination.getQueueName());
			}
		} catch (Throwable t) {
			log.debug("consumer could not be closed");
			throw new ConnectionException(-1, "Could not delete the queue", t);
		}
	}

	public static org.jboss.blacktie.jatmibroker.core.transport.Message convertFromBytesMessage(
			BytesMessage message) throws JMSException {
		String controlIOR = message.getStringProperty("messagecontrol");
		String replyTo = message.getStringProperty("messagereplyto");
		int len = (int) message.getBodyLength();
		String serviceName = message.getStringProperty("servicename");
		int flags = new Integer(message.getStringProperty("messageflags"));
		int cd = new Integer(message.getStringProperty("messagecorrelationId"));

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
		toProcess.len = len - pad;
		if (toProcess.type == "") {
			toProcess.data = null;
		} else {
			toProcess.data = new byte[toProcess.len];
			message.readBytes(toProcess.data);
		}
		toProcess.control = controlIOR;
		return toProcess;
	}

	public int determineTimeout(long flags) throws ConnectionException {
		return timeout;
	}
}
