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

import java.util.Properties;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.core.transport.EventListener;
import org.jboss.blacktie.jatmibroker.core.transport.Message;
import org.jboss.blacktie.jatmibroker.core.transport.Receiver;
import org.jboss.blacktie.jatmibroker.core.transport.Sender;
import org.jboss.blacktie.jatmibroker.core.transport.Transport;
import org.jboss.blacktie.jatmibroker.jab.JABException;
import org.jboss.blacktie.jatmibroker.jab.JABTransaction;

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
	 * The event listener allows us to hear events on tpsend
	 */
	private EventListener eventListener;

	/**
	 * The last event received by the session, this is either discon, SUCC,
	 * FAIL, ERR
	 */
	private long lastEvent = -1;

	/**
	 * The last rcode
	 */
	private int lastRCode = 0;

	/**
	 * Is the session in read mode.
	 */
	private boolean canSend = true;

	/**
	 * Is the session in write mode, so to speak.
	 */
	private boolean canRecv = true;

	private Properties properties;

	/**
	 * Create a new session
	 * 
	 * @param transport
	 * @param cd
	 * @param c
	 * @param b
	 * @throws ConfigurationException
	 */
	Session(Properties properties, Transport transport, int cd)
			throws ConnectionException {
		this.properties = properties;
		this.transport = transport;
		this.cd = cd;
		this.eventListener = new EventListenerImpl(this);
		this.receiver = transport.createReceiver(eventListener);

		this.canSend = false;
		this.canRecv = true;
	}

	/**
	 * Create a new session
	 * 
	 * @param transport
	 * @param cd
	 * @param receiver2
	 * @throws ConfigurationException
	 */
	Session(Properties properties, Transport transport, int cd, Sender sender)
			throws ConnectionException {
		this.properties = properties;
		this.transport = transport;
		this.cd = cd;
		this.sender = sender;
		this.eventListener = new EventListenerImpl(this);
		this.receiver = transport.createReceiver(eventListener);

		this.canRecv = false;
		this.canSend = true;
	}

	void setCreatorState(long sentFlags) {
		// Sort out session state
		if ((sentFlags & Connection.TPSENDONLY) == Connection.TPSENDONLY) {
			canSend = true;
			canRecv = false;
		} else if ((sentFlags & Connection.TPRECVONLY) == Connection.TPRECVONLY) {
			canSend = false;
			canRecv = true;
		}
	}

	void setCreatedState(long receivedFlags) {
		// Sort out session state
		if ((receivedFlags & Connection.TPSENDONLY) == Connection.TPSENDONLY) {
			canSend = false;
			canRecv = true;
		} else if ((receivedFlags & Connection.TPRECVONLY) == Connection.TPRECVONLY) {
			canSend = true;
			canRecv = false;
		}
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
	public int tpsend(Buffer toSend, int len, int flags)
			throws ConnectionException {
		int toReturn = -1;
		log.debug("tpsend invoked");
		if (this.lastEvent > -1) {
			throw new ConnectionException(Connection.TPEEVENT, lastEvent,
					lastRCode, "Event existed on descriptor: " + lastEvent,
					null);
		} else if (!canSend) {
			throw new ConnectionException(Connection.TPEPROTO,
					"Session can't send");
		}
		// Can only send in certain circumstances
		if (sender != null) {
			log.debug("Sender not null, sending");
			String type = null;
			String subtype = null;
			byte[] data = null;
			if (toSend != null) {
				data = toSend.serialize();
				type = toSend.getType();
				subtype = toSend.getSubtype();
				if (!type.equals("X_OCTET")) {
					len = data.length;
				}
			}

			sender.send(receiver.getReplyTo(), (short) 0, 0, data, len, cd,
					flags, 0, type, subtype);

			// Sort out session state
			if ((flags & Connection.TPSENDONLY) == Connection.TPSENDONLY) {
				canSend = true;
				canRecv = false;
			} else if ((flags & Connection.TPRECVONLY) == Connection.TPRECVONLY) {
				canSend = false;
				canRecv = true;
			}

			toReturn = 0;
		} else {
			throw new ConnectionException(-1, "Session in receive mode", null);
		}
		return toReturn;
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
		log.debug("Receiving");
		if (!canRecv) {
			throw new ConnectionException(Connection.TPEPROTO,
					"Session can't receive");
		}
		Message m = receiver.receive(flags);
		// Prepare the outbound channel
		if (m.replyTo == null
				|| (sender != null && !m.replyTo.equals(sender.getSendTo()))) {
			log.trace("Send to location has altered");
			sender.close();
			sender = null;
		}
		if (sender == null && m.replyTo != null && !m.replyTo.equals("")) {
			log.trace("Will require a new sender");
			sender = transport.createSender(m.replyTo);
		} else {
			log.debug("Not setting the sender");
		}

		X_OCTET received = null;
		if (m.type != null) {
			received = (X_OCTET) tpalloc("X_OCTET", null);
			received.setByteArray(m.data);
		}
		log.debug("Prepared and ready to launch");

		// Sort out session state
		if ((m.flags & Connection.TPSENDONLY) == Connection.TPSENDONLY) {
			canSend = false;
			canRecv = true;
		} else if ((m.flags & Connection.TPRECVONLY) == Connection.TPRECVONLY) {
			canSend = true;
			canRecv = false;
		}

		// Check we didn't just get an event while waiting
		if (this.lastEvent > -1) {
			throw new ConnectionException(Connection.TPEEVENT, lastEvent,
					lastRCode, "Event existed on descriptor: " + lastEvent,
					received);
		} else if ((m.flags & Connection.TPRECVONLY) == Connection.TPRECVONLY) {
			throw new ConnectionException(Connection.TPEEVENT,
					Connection.TPEV_SENDONLY, 0, "Reporting send only event",
					received);
		} else if (m.rval == Connection.TPSUCCESS) {
			throw new ConnectionException(Connection.TPEEVENT,
					Connection.TPEV_SVCSUCC, 0,
					"Service completed successfully event", received);
		}
		return received;
	}

	/**
	 * Close the conversation with the remote service.
	 * 
	 * @param cd
	 *            The connection descriptor to use
	 */
	public void tpdiscon() throws ConnectionException {
		if (JABTransaction.current() != null) {
			try {
				JABTransaction.current().rollback_only();
			} catch (JABException e) {
				throw new ConnectionException(Connection.TPESYSTEM,
						"Could not mark transaction for rollback only");
			}
		}
		try {
			sender.send("", EventListener.DISCON_CODE, 0, null, 0, cd, 0, 0,
					null, null);
		} catch (org.omg.CORBA.OBJECT_NOT_EXIST one) {
			log.warn("The disconnect called failed to notify the remote end");
			log.debug("The disconnect called failed to notify the remote end",
					one);
		}

	}

	/**
	 * Return the connection descriptor
	 * 
	 * @return
	 */
	public int getCd() {
		return cd;
	}

	private void setLastEvent(long lastEvent) {
		this.lastEvent = lastEvent;
	}

	public void setLastRCode(int rcode) {
		this.lastRCode = rcode;
	}

	private class EventListenerImpl implements EventListener {

		private Session session;

		public EventListenerImpl(Session session) {
			this.session = session;
		}

		public void setLastEvent(long lastEvent) {
			session.setLastEvent(lastEvent);
		}

		public void setLastRCode(int rcode) {
			session.setLastRCode(rcode);

		}
	}

	Receiver getReceiver() {
		return receiver;
	}

	/**
	 * Allocate a new buffer
	 * 
	 * @param type
	 *            The type of the buffer
	 * @param subtype
	 *            The subtype of the buffer
	 * @return The new buffer
	 * @throws ConnectionException
	 *             If the buffer cannot be created or the subtype located
	 */
	private Buffer tpalloc(String type, String subtype)
			throws ConnectionException {
		if (type == null) {
			throw new ConnectionException(Connection.TPEINVAL,
					"No type provided");
		} else if (type.equals("X_OCTET")) {
			log.debug("Initializing a new X_OCTET");
			return new X_OCTET();
		} else if (type.equals("X_C_TYPE")) {
			log.debug("Initializing a new X_C_TYPE");
			return new X_C_TYPE(subtype, properties);
		} else {
			log.debug("Initializing a new X_COMMON");
			return new X_COMMON(subtype, properties);
		}
	}
}