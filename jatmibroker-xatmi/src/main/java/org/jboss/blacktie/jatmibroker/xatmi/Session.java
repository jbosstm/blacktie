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
package org.jboss.blacktie.jatmibroker.xatmi;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.core.transport.Message;
import org.jboss.blacktie.jatmibroker.core.transport.Receiver;
import org.jboss.blacktie.jatmibroker.core.transport.Sender;
import org.jboss.blacktie.jatmibroker.core.transport.Transport;

/**
 * This is the session to send data on.
 */
public class Session {
	private static final Logger log = LogManager.getLogger(Session.class);

	/**
	 * The transport to manage data on
	 */
	private Transport transport;
	/**
	 * The descriptor
	 */
	private int cd;

	/**
	 * The sessions sender
	 */
	private Sender sender;

	/**
	 * The sessions receiver
	 */
	private Receiver receiver;

	/**
	 * Create a new session
	 * 
	 * @param transport
	 * @param cd
	 * @param receiver2
	 * @throws ConfigurationException
	 */
	Session(Transport transport, int cd, Receiver receiver)
			throws ConnectionException {
		this.transport = transport;
		this.cd = cd;
		this.receiver = receiver;
	}

	/**
	 * Create a new session
	 * 
	 * @param transport
	 * @param cd
	 * @param receiver2
	 * @throws ConfigurationException
	 */
	Session(Transport transport, int cd, Sender sender)
			throws ConnectionException {
		this.transport = transport;
		this.cd = cd;
		this.sender = sender;
	}

	/**
	 * Close the session
	 * 
	 * @throws ConnectionException
	 */
	void close() throws ConnectionException {
		log.debug("Closing session");
		if (sender != null) {
			log.debug("Sender closing");
			sender.close();
			sender = null;
		}
		if (receiver != null) {
			log.debug("Receiver closing");
			receiver.close();
			receiver = null;
		}
		log.debug("Closed session");
	}

	/**
	 * Send a buffer to a remote server in a conversation
	 * 
	 * @param cd
	 *            The connection descriptor
	 * @param idata
	 *            The outbound data
	 * @param flags
	 *            The flags to use
	 */
	public void tpsend(Buffer buffer, int len, int flags)
			throws ConnectionException {
		log.debug("tpsend invoked");
		// Can only send in certain circumstances
		if (sender != null) {
			log.debug("Sender not null, sending");
			sender.send(getReceiver().getReplyTo(), (short) 0, 0, buffer
					.getData(), len, cd, flags);
		} else {
			throw new ConnectionException(-1, "Session in receive mode", null);
		}
	}

	/**
	 * Received the next response in a conversation
	 * 
	 * @param cd
	 *            The connection descriptor to use
	 * @param flags
	 *            The flags to use
	 * @return The next response
	 */
	public Buffer tprecv(int flags) throws ConnectionException {
		Message m = getReceiver().receive(flags);
		log.debug("Receiving");
		// Prepare the outbound channel
		if (m.replyTo == null
				|| (sender != null && !m.replyTo.equals(sender.getSendTo()))) {
			log.trace("Send to location has altered");
			sender.close();
			sender = null;
		}
		if (sender == null && m.replyTo != null) {
			log.trace("Will require a new sender");
			sender = transport.createSender(m.replyTo);
		} else {
			log.debug("Not setting the sender");
		}

		log.debug("Initializing a new buffer");
		// TODO WE SHOULD BE SENDING THE TYPE, SUBTYPE AND CONNECTION ID?
		Buffer received = new Buffer(null, null);
		received.setData(m.data);
		log.debug("Prepared and ready to launch");
		return received;
	}

	/**
	 * Close the conversation with the remote service.
	 * 
	 * @param cd
	 *            The connection descriptor to use
	 */
	public void tpdiscon() throws ConnectionException {
		throw new ConnectionException(-1, "Waiting for implementation", null);
	}

	/**
	 * Return the connection descriptor
	 * 
	 * @return
	 */
	public int getCd() {
		return cd;
	}

	private Receiver getReceiver() throws ConnectionException {
		if (receiver == null) {
			receiver = transport.createReceiver();
			log.debug("Created a new receiver");
		}
		return receiver;
	}
}
