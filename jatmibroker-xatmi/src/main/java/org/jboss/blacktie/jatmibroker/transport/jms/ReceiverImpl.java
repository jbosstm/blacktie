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
import javax.jms.MessageConsumer;
import javax.jms.Queue;
import javax.jms.Session;
import javax.jms.TemporaryQueue;
import javax.naming.NamingException;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.transport.Message;
import org.jboss.blacktie.jatmibroker.transport.Receiver;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;

public class ReceiverImpl implements Receiver {
	private static final Logger log = LogManager.getLogger(ReceiverImpl.class);
	private Queue destination;
	private MessageConsumer receiver;
	private boolean isTemporary;

	ReceiverImpl(Session session) throws JMSException {
		destination = session.createTemporaryQueue();
		receiver = session.createConsumer(destination);
		isTemporary = true;
		log.info("Creating a consumer on: " + destination.getQueueName());
	}

	ReceiverImpl(Session session, Destination destination) throws JMSException,
			NamingException {
		this.destination = (Queue) destination;
		receiver = session.createConsumer(destination);
		log.info("Creating a consumer on: " + this.destination.getQueueName());
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
			log.info("Receiving from: " + destination.getQueueName());
			javax.jms.Message message = receiver.receive();
			log.info("Received from: " + destination.getQueueName());
			BytesMessage bytesMessage = ((BytesMessage) message);
			// TODO String replyTo = message.getStringProperty("reply-to");
			Destination replyTo = message.getJMSReplyTo();
			int len = (int) bytesMessage.getBodyLength();
			String serviceName = message.getStringProperty("serviceName");
			int flags = new Integer(message.getStringProperty("messageflags"));
			int cd = new Integer(message
					.getStringProperty("messagecorrelationId"));
			byte[] bytes = new byte[len];
			bytesMessage.readBytes(bytes);

			org.jboss.blacktie.jatmibroker.transport.Message toProcess = new org.jboss.blacktie.jatmibroker.transport.Message();
			toProcess.replyTo = replyTo;
			toProcess.len = len;
			toProcess.serviceName = serviceName;
			toProcess.flags = flags;
			toProcess.cd = cd;
			toProcess.data = bytes;

			return toProcess;
		} catch (Throwable t) {
			throw new ConnectionException(-1, "Could not receive the message",
					t);
		}
	}

	public void close() throws ConnectionException {
		try {
			receiver.close();
			if (isTemporary) {
				log.info("Deleting: " + destination.getQueueName());
				((TemporaryQueue) destination).delete();
				log.info("Deleted: " + destination.getQueueName());
			}
		} catch (Throwable t) {
			throw new ConnectionException(-1, "Could not delete the queue", t);
		}
	}
}
