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

import java.util.Arrays;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Properties;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.core.transport.Message;
import org.jboss.blacktie.jatmibroker.core.transport.Receiver;
import org.jboss.blacktie.jatmibroker.core.transport.Transport;
import org.jboss.blacktie.jatmibroker.core.transport.TransportFactory;

/**
 * This is the connection to remote Blacktie services. It must be created using
 * the ConnectionFactory
 * 
 * @see ConnectionFactory
 */
public class Connection {

	private static final Logger log = LogManager.getLogger(Connection.class);

	// AVAILABLE FLAGS
	public static final int TPNOBLOCK = 0x00000001;
	public static final int TPSIGRSTRT = 0x00000002;
	public static final int TPNOREPLY = 0x00000004;
	public static final int TPNOTRAN = 0x00000008;
	public static final int TPTRAN = 0x00000010;
	public static final int TPNOTIME = 0x00000020;
	public static final int TPGETANY = 0x00000080;
	public static final int TPNOCHANGE = 0x00000100;
	public static final int TPCONV = 0x00000400;
	public static final int TPSENDONLY = 0x00000800;
	public static final int TPRECVONLY = 0x00001000;

	// ERROR CONDITIONS
	public static final int TPEBADDESC = 2;
	public static final int TPEBLOCK = 3;
	public static final int TPEINVAL = 4;
	public static final int TPELIMIT = 5;
	public static final int TPENOENT = 6;
	public static final int TPEOS = 7;
	public static final int TPEPROTO = 9;
	public static final int TPESVCERR = 10;
	public static final int TPESVCFAIL = 11;
	public static final int TPESYSTEM = 12;
	public static final int TPETIME = 13;
	public static final int TPETRAN = 14;
	public static final int TPGOTSIG = 15;
	public static final int TPEITYPE = 17;
	public static final int TPEOTYPE = 18;
	public static final int TPEEVENT = 22;
	public static final int TPEMATCH = 23;

	// SERVICE CONDITIONS
	public static final short TPFAIL = 0x00000001;
	public static final short TPSUCCESS = 0x00000002;

	// Events
	public static final long TPEV_DISCONIMM = 0x0001;
	public static final long TPEV_SVCERR = 0x0002;
	public static final long TPEV_SVCFAIL = 0x0004;
	public static final long TPEV_SVCSUCC = 0x0008;
	public static final long TPEV_SENDONLY = 0x0020;

	public static final int XATMI_SERVICE_NAME_LENGTH = 15;

	private static int nextId;

	private Map<String, Transport> transports = new HashMap<String, Transport>();

	/**
	 * Any local temporary queues created in this connection
	 */
	private Map<java.lang.Integer, Receiver> temporaryQueues = new HashMap<java.lang.Integer, Receiver>();

	private Properties properties;

	private Map<Integer, Session> sessions = new HashMap<Integer, Session>();

	private static boolean warnedTPSIGRSTRT;

	/**
	 * The connection
	 * 
	 * @param properties
	 * @param username
	 * @param password
	 * @throws ConnectionException
	 */
	Connection(Properties properties) {
		this.properties = properties;
	}

	/**
	 * Allocate a new buffer
	 * 
	 * @param type
	 *            The type of the buffer
	 * @param subType
	 *            The subtype of the buffer
	 * @return The new buffer
	 * @throws ConnectionException
	 */
	public Buffer tpalloc(String type, String subtype)
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

	/**
	 * Synchronous call
	 * 
	 * @param svc
	 *            The name of the service to call
	 * @param idata
	 *            The inbound data
	 * @param flags
	 *            The flags to use
	 * @return The returned buffer
	 */
	public Response tpcall(String svc, Buffer buffer, int len, int flags)
			throws ConnectionException {
		int cd = tpacall(svc, buffer, len, flags);
		return receive(cd, flags);
	}

	/**
	 * Asynchronous call
	 * 
	 * @param svc
	 *            The name of the service to call
	 * @param idata
	 *            The inbound data
	 * @param flags
	 *            The flags to use
	 * @return The connection descriptor
	 */
	public int tpacall(String svc, Buffer toSend, int len, int flags)
			throws ConnectionException {
		log.info("tpacall");
		int toCheck = flags & ~(TPNOTRAN | TPNOREPLY | TPNOBLOCK | TPNOTIME | TPSIGRSTRT);
		if (toCheck != 0) {
			log.trace("invalid flags remain: " + toCheck);
			throw new ConnectionException(Connection.TPEINVAL, "Invalid flags remain: " + toCheck);
		}

		boolean hasTPSIGSTRT = (flags & TPSIGRSTRT) == TPSIGRSTRT;
		if (hasTPSIGSTRT && !warnedTPSIGRSTRT) {
			log.error("TPSIGRSTRT NOT SUPPORTED FOR SENDS OR RECEIVES");
			warnedTPSIGRSTRT = true;
		}
		svc = svc.substring(0, Math.min(Connection.XATMI_SERVICE_NAME_LENGTH,
				svc.length()));
		int correlationId = 0;
		synchronized (this) {
			correlationId = ++nextId;
		}
		Transport transport = getTransport(svc);
		Receiver endpoint = transport.createReceiver();
		temporaryQueues.put(correlationId, endpoint);
		// TODO HANDLE TRANSACTION
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

		String timeToLive = properties.getProperty("TimeToLive");
		int ttl = 0;

		if (timeToLive != null) {
			ttl = Integer.parseInt(timeToLive) * 1000;
		}
		transport.getSender(svc).send(endpoint.getReplyTo(), (short) 0, 0,
				data, len, correlationId, flags, ttl, type, subtype);
		if ((flags & Connection.TPNOREPLY) == Connection.TPNOREPLY) {
			correlationId = 0;
		}
		log.debug("Returning cd: " + correlationId);
		return correlationId;
	}

	/**
	 * Cancel the outstanding asynchronous call.
	 * 
	 * @param cd
	 *            The connection descriptor
	 * @param flags
	 *            The flags to use
	 */
	public int tpcancel(int cd) throws ConnectionException {
		log.debug("tpcancel: " + cd);
		int toReturn = -1;
		Receiver endpoint = temporaryQueues.remove(cd);
		if (endpoint != null) {
			log.debug("closing endpoint");
			endpoint.close();
			log.debug("endpoint closed");
			toReturn = 0;
		} else {
			log.debug("No endpoint available");
			throw new ConnectionException(Connection.TPEBADDESC, "cd " + cd
					+ " does not exist");
		}
		log.debug("tpcancel returning: " + toReturn);
		return toReturn;
	}

	/**
	 * Get the reply from the server
	 * 
	 * @param cd
	 *            The connection descriptor to use
	 * @param flags
	 *            The flags to use
	 * @return The response from the server
	 */
	public Response tpgetrply(int cd, int flags) throws ConnectionException {
		log.debug("tpgetrply: " + cd);
		int toCheck = flags & ~(TPGETANY | TPNOCHANGE | TPNOBLOCK | TPNOTIME | TPSIGRSTRT);
		if (toCheck != 0) {
			log.trace("invalid flags remain: " + toCheck);
			throw new ConnectionException(Connection.TPEINVAL, "Invalid flags remain: " + toCheck);
		}
		
		boolean hasTPSIGSTRT = (flags & TPSIGRSTRT) == TPSIGRSTRT;
		if (hasTPSIGSTRT && !warnedTPSIGRSTRT) {
			log.error("TPSIGRSTRT NOT SUPPORTED FOR SENDS OR RECEIVES");
			warnedTPSIGRSTRT = true;
		}

		Response toReturn = receive(cd, flags);
		Session session = sessions.remove(cd);
		if (session != null) {
			log.debug("closing session");
			session.close();
			log.debug("closed session");
		}
		log.debug("tpgetrply returning: " + toReturn);
		return toReturn;
	}

	/**
	 * Handle the initiation of a conversation with the server
	 * 
	 * @param svc
	 *            The name of the service
	 * @param idata
	 *            The outbound buffer
	 * @param flags
	 *            The flags to use
	 * @return The connection descriptor
	 */
	public Session tpconnect(String svc, Buffer toSend, int len, int flags)
			throws ConnectionException {
		log.debug("tpconnect: " + svc);

		svc = svc.substring(0, Math.min(Connection.XATMI_SERVICE_NAME_LENGTH,
				svc.length()));
		// Initiate the session
		boolean hasTPSIGSTRT = (flags & TPSIGRSTRT) == TPSIGRSTRT;
		if (hasTPSIGSTRT && !warnedTPSIGRSTRT) {
			log.error("TPSIGRSTRT NOT SUPPORTED FOR SENDS OR RECEIVES");
			warnedTPSIGRSTRT = true;
		}
		svc = svc.substring(0, Math.min(Connection.XATMI_SERVICE_NAME_LENGTH,
				svc.length()));
		int correlationId = 0;
		synchronized (this) {
			correlationId = nextId++;
		}
		Transport transport = getTransport(svc);
		Session session = new Session(properties, transport, correlationId);

		Receiver endpoint = session.getReceiver();
		// TODO HANDLE TRANSACTION
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

		String timeToLive = properties.getProperty("TimeToLive");
		int ttl = 0;

		if (timeToLive != null) {
			ttl = Integer.parseInt(timeToLive) * 1000;
		}
		log.debug("tpconnect sending data");
		transport.getSender(svc).send(endpoint.getReplyTo(), (short) 0, 0,
				data, len, correlationId, flags | TPCONV, ttl, type, subtype);

		byte[] response = null;
		try {
			log.debug("tpconnect receiving data");
			X_OCTET odata = (X_OCTET) session.tprecv(0);
			response = odata.getByteArray();
			log.debug("tpconnect received data");
		} catch (ConnectionException e) {
			if (e.getReceived() != null) {
				response = ((X_OCTET) e.getReceived()).getByteArray();
				log.debug("Caught an exception with data", e);
			} else {
				throw new ConnectionException(e.getTperrno(),
						"Could not connect");
			}
		}
		byte[] ack = new byte[4];
		byte[] bytes = "ACK".getBytes();
		System.arraycopy(bytes, 0, ack, 0, 3);
		boolean connected = response == null ? false : Arrays.equals(ack,
				response);
		if (!connected) {
			log.error("Could not connect");
			session.close();
			throw new ConnectionException(Connection.TPETIME,
					"Could not connect");
		}
		session.setCreatorState(flags);
		temporaryQueues.put(correlationId, endpoint);
		sessions.put(correlationId, session);

		// Return a handle to allow the connection to send/receive data on
		return session;
	}

	/**
	 * Close any resources associated with this connection
	 * 
	 * @throws ConnectionException
	 */
	public void close() throws ConnectionException {
		log.debug("Close connection called");
		Iterator<Receiver> receivers = temporaryQueues.values().iterator();
		while (receivers.hasNext()) {
			Receiver receiver = receivers.next();
			log.debug("closing receiver");
			receiver.close();
			log.debug("closed receiver");
		}
		temporaryQueues.clear();
		Iterator<Transport> transports = this.transports.values().iterator();
		while (transports.hasNext()) {
			Transport transport = transports.next();
			log.debug("closing transport");
			transport.close();
			log.debug("closed transport");
		}
		this.transports.clear();
		log.debug("Close connection finished");
	}

	private Transport getTransport(String serviceName)
			throws ConnectionException {
		Transport toReturn = transports.get(serviceName);
		if (toReturn == null) {
			try {
				toReturn = TransportFactory.loadTransportFactory(serviceName,
						properties).createTransport();
			} catch (ConfigurationException e) {
				throw new ConnectionException(Connection.TPENOENT,
						"Could not load transport for: " + serviceName, e);
			}
			transports.put(serviceName, toReturn);
		}
		return toReturn;
	}

	private Response receive(int cd, int flags) throws ConnectionException {
		log.debug("receive: " + cd);
		Receiver endpoint = temporaryQueues.remove(cd);
		if (endpoint == null) {
			throw new ConnectionException(Connection.TPEBADDESC,
					"Session does not exist");
		}
		Message message = endpoint.receive(flags);
		// TODO WE SHOULD BE SENDING THE CONNECTION ID?
		Buffer buffer = null;
		if (message.type != null && !message.type.equals("")) {
			if (message.type.equals("X_OCTET")) {
				log.debug("Initializing a new X_OCTET");
				buffer = new X_OCTET(message.data);
			} else if (message.type.equals("X_C_TYPE")) {
				log.debug("Initializing a new X_C_TYPE");
				buffer = new X_C_TYPE(message.subtype, properties, message.data);
			} else {
				log.debug("Initializing a new X_COMMON");
				buffer = new X_COMMON(message.subtype, properties, message.data);
			}
		}
		if (message.rval == Connection.TPFAIL) {
			if (message.rcode == Connection.TPESVCERR) {
				throw new ConnectionException(Connection.TPESVCERR, 0L,
						message.rcode,
						"Got an error back from the remote service", buffer);
			}
			throw new ConnectionException(Connection.TPESVCFAIL, 0L,
					message.rcode, "Got a fail back from the remote service",
					buffer);
		} else {
			Response response = new Response(message.rval, message.rcode,
					buffer, message.len, message.flags);

			log.debug("received returned a response? "
					+ (response == null ? "false" : "true"));
			return response;
		}
	}
}
