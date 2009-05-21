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

import org.jboss.blacktie.jatmibroker.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.transport.Message;
import org.jboss.blacktie.jatmibroker.transport.Receiver;
import org.jboss.blacktie.jatmibroker.transport.Sender;
import org.jboss.blacktie.jatmibroker.transport.Transport;

/**
 * This is the session to send data on.
 */
public class Session {
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
	Session(Transport transport, int cd, Sender sender, Receiver receiver)
			throws ConnectionException {
		this.transport = transport;
		this.cd = cd;
		this.sender = sender;
		this.receiver = receiver;
	}

	/**
	 * Close the session
	 * 
	 * @throws ConnectionException
	 */
	void close() throws ConnectionException {
		if (sender != null) {
			sender.close();
		}
		receiver.close();
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
		// Can only send in certain circumstances
		if (sender != null) {
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

		// Prepare the outbound channel
		if (m.replyTo == null
				|| (sender != null && !sender.getName().equals(m.replyTo))) {
			sender.close();
		}
		if (m.replyTo != null) {
			sender = transport.createSender(m.replyTo);
		}

		// TODO WE SHOULD BE SENDING THE TYPE, SUBTYPE AND CONNECTION ID?
		Buffer received = new Buffer(null, null);
		received.setData(m.data);
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
		}
		return receiver;
	}
}
