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

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Properties;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.ResponseMonitor;
import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.core.transport.Message;
import org.jboss.blacktie.jatmibroker.core.transport.Receiver;
import org.jboss.blacktie.jatmibroker.core.transport.Transport;
import org.jboss.blacktie.jatmibroker.core.transport.TransportFactory;

/**
 * This is the connection to remote BlackTie services. It must be created using
 * the ConnectionFactory.
 * 
 * @see ConnectionFactory#getConnection()
 */
public class Connection {

	/**
	 * The logger to use.
	 */
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

	/**
	 * The next id to use for session connection descriptors.
	 */
	private static int nextId;

	/**
	 * The transports created for each service.
	 */
	private Map<String, Transport> transports = new HashMap<String, Transport>();

	/**
	 * Any local temporary queues created in this connection
	 */
	private Map<java.lang.Integer, Receiver> temporaryQueues = new HashMap<java.lang.Integer, Receiver>();

	/**
	 * The properties that this connection was created with.
	 */
	private Properties properties;

	/**
	 * The open sessions from this connection/
	 */
	private Map<Integer, Session> sessions = new HashMap<Integer, Session>();

	/**
	 * The list of sessionids that have received a message.
	 */
	private List<Integer> tpGetAnySessions = new ArrayList<Integer>();

	/**
	 * The monitor of responses.
	 */
	private ResponseMonitor responseMonitor;

	/**
	 * The connection factory that created this connection.
	 */
	private ConnectionFactory connectionFactory;

	/**
	 * The service sessions for this connection.
	 */
	private Session serviceSession;

	/**
	 * The connection
	 * 
	 * @param connectionFactory
	 *            The connection factory that created this connection.
	 * @param properties
	 *            The properties that this connection was created with.
	 */
	Connection(ConnectionFactory connectionFactory, Properties properties) {
		this.connectionFactory = connectionFactory;
		this.properties = properties;
		responseMonitor = new ResponseMonitorImpl();
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
	 *             If the buffer was unknown or invalid.
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
	 * Synchronous call.
	 * 
	 * @param svc
	 *            The name of the service to call
	 * @param buffer
	 *            The inbound data
	 * @param len
	 *            The length of the data
	 * @param flags
	 *            The flags to use
	 * @return The returned buffer
	 * @throws ConnectionException
	 *             If the service cannot be contacted.
	 */
	public Response tpcall(String svc, Buffer buffer, int len, int flags)
			throws ConnectionException {
		log.debug("tpcall");
		int tpacallFlags = flags;
		tpacallFlags &= ~TPNOCHANGE;
		int cd = tpacall(svc, buffer, len, tpacallFlags);
		return receive(cd, flags);
	}

	/**
	 * Asynchronous call
	 * 
	 * @param svc
	 *            The name of the service to call
	 * @param toSend
	 *            The inbound data
	 * @param len
	 *            The length of the data
	 * @param flags
	 *            The flags to use
	 * @return The connection descriptor
	 * @throws ConnectionException
	 *             If the service cannot be contacted.
	 */
	public int tpacall(String svc, Buffer toSend, int len, int flags)
			throws ConnectionException {
		log.debug("tpacall");
		int toCheck = flags
				& ~(TPNOTRAN | TPNOREPLY | TPNOBLOCK | TPNOTIME | TPSIGRSTRT);
		if (toCheck != 0) {
			log.trace("invalid flags remain: " + toCheck);
			throw new ConnectionException(Connection.TPEINVAL,
					"Invalid flags remain: " + toCheck);
		}

		svc = svc.substring(0, Math.min(Connection.XATMI_SERVICE_NAME_LENGTH,
				svc.length()));
		int correlationId = 0;
		synchronized (this) {
			correlationId = ++nextId;
		}
		Transport transport = getTransport(svc);
		Receiver endpoint = transport.createReceiver(correlationId,
				responseMonitor);
		temporaryQueues.put(correlationId, endpoint);
		log.trace("Added a queue for: " + correlationId);
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
	 * @throws ConnectionException
	 *             If the client cannot be cleaned up.
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
	 * Get the reply for an asynchronous call.
	 * 
	 * @param cd
	 *            The connection descriptor to use
	 * @param flags
	 *            The flags to use
	 * @return The response from the server
	 * @throws ConnectionException
	 *             If the service cannot be contacted.
	 */
	public Response tpgetrply(int cd, int flags) throws ConnectionException {
		log.debug("tpgetrply: " + cd);
		int toCheck = flags
				& ~(TPGETANY | TPNOCHANGE | TPNOBLOCK | TPNOTIME | TPSIGRSTRT);
		if (toCheck != 0) {
			log.trace("invalid flags remain: " + toCheck);
			throw new ConnectionException(Connection.TPEINVAL,
					"Invalid flags remain: " + toCheck);
		}

		synchronized (tpGetAnySessions) {
			if ((flags & Connection.TPGETANY) == Connection.TPGETANY) {
				int timeout = 0;
				if ((flags & Connection.TPNOTIME) != Connection.TPNOTIME) {
					timeout = Integer.parseInt(properties
							.getProperty("RequestTimeout"))
							* 1000
							+ Integer.parseInt(properties
									.getProperty("TimeToLive")) * 1000;
				}
				if (tpGetAnySessions.size() == 0) {
					try {
						tpGetAnySessions.wait(timeout);
					} catch (InterruptedException e) {
						throw new ConnectionException(Connection.TPESYSTEM,
								"Could not wait", e);
					}
				}
				if (tpGetAnySessions.size() == 0) {
					throw new ConnectionException(Connection.TPETIME,
							"No message arrived");
				} else {
					cd = tpGetAnySessions.remove(0);
				}
			}
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
	 * Handle the initiation of a conversation with the server.
	 * 
	 * @param svc
	 *            The name of the service
	 * @param toSend
	 *            The outbound buffer
	 * @param len
	 *            The length of the data
	 * @param flags
	 *            The flags to use
	 * @return The connection descriptor
	 * @throws ConnectionException
	 *             If the service cannot be contacted.
	 */
	public Session tpconnect(String svc, Buffer toSend, int len, int flags)
			throws ConnectionException {
		log.debug("tpconnect: " + svc);

		svc = svc.substring(0, Math.min(Connection.XATMI_SERVICE_NAME_LENGTH,
				svc.length()));
		// Initiate the session
		svc = svc.substring(0, Math.min(Connection.XATMI_SERVICE_NAME_LENGTH,
				svc.length()));
		int correlationId = 0;
		synchronized (this) {
			correlationId = nextId++;
		}
		Transport transport = getTransport(svc);
		Session session = new Session(this, transport, correlationId);

		Receiver receiver = session.getReceiver();
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
		transport.getSender(svc).send(receiver.getReplyTo(), (short) 0, 0,
				data, len, correlationId, flags | TPCONV, ttl, type, subtype);

		byte[] response = null;
		try {
			log.debug("tpconnect receiving data");
			X_OCTET odata = (X_OCTET) session.tprecv(0);
			response = odata.getByteArray();
			log.debug("tpconnect received data");
		} catch (ResponseException e) {
			response = ((X_OCTET) e.getReceived()).getByteArray();
			log.debug("Caught an exception with data", e);
		} catch (ConnectionException e) {
			throw new ConnectionException(e.getTperrno(), "Could not connect");
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
		sessions.put(correlationId, session);
		temporaryQueues.put(correlationId, session.getReceiver());

		// Return a handle to allow the connection to send/receive data on
		return session;
	}

	/**
	 * Close any resources associated with this connection
	 * 
	 * @throws ConnectionException
	 *             If an open session cannot be cancelled or disconnected.
	 */
	public void close() throws ConnectionException {
		log.debug("Close connection called");

		// MUST close the session first to remove the temporary queue
		Iterator<Session> sessions = this.sessions.values().iterator();
		while (sessions.hasNext()) {
			Session session = sessions.next();
			log.debug("closing session");
			session.tpdiscon();
			log.debug("Closed open session");
		}
		this.sessions.clear();
		log.trace("Sessions cleared");

		Iterator<Receiver> receivers = temporaryQueues.values().iterator();
		while (receivers.hasNext()) {
			Receiver receiver = receivers.next();
			log.debug("closing receiver");
			tpcancel(receiver.getCd());
			log.warn("Closed open receiver");
		}
		temporaryQueues.clear();
		log.trace("Temporary queues cleared");

		if (serviceSession != null) {
			log.debug("closing service session");
			serviceSession.close();
			log.debug("Closed open service session");
		}

		Iterator<Transport> transports = this.transports.values().iterator();
		while (transports.hasNext()) {
			Transport transport = transports.next();
			log.debug("closing transport");
			transport.close();
			log.debug("closed transport");
		}
		this.transports.clear();
		this.connectionFactory.removeConnection(this);
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

	/**
	 * Retrieve a response.
	 * 
	 * @param cd
	 *            The connection descriptor
	 * @param flags
	 *            The flags to use
	 * @return The response
	 * @throws ConnectionException
	 *             If the response cannot be retrieved.
	 */
	private Response receive(int cd, int flags) throws ConnectionException {
		log.debug("receive: " + cd);
		Receiver endpoint = temporaryQueues.remove(cd);
		if (endpoint == null) {
			throw new ConnectionException(Connection.TPEBADDESC,
					"Session does not exist: " + cd);
		}
		Message message = endpoint.receive(flags);
		// TODO WE SHOULD BE SENDING THE CONNECTION ID?
		Buffer buffer = null;
		if (message.type != null && !message.type.equals("")) {
			buffer = tpalloc(message.type, message.subtype);
			buffer.deserialize(message.data);
		}
		if (message.rval == Connection.TPFAIL) {
			if (message.rcode == Connection.TPESVCERR) {
				throw new ResponseException(Connection.TPESVCERR, 0L,
						message.rcode,
						"Got an error back from the remote service", buffer);
			}
			throw new ResponseException(Connection.TPESVCFAIL, 0L,
					message.rcode, "Got a fail back from the remote service",
					buffer);
		} else {
			Response response = new Response(cd, message.rval, message.rcode,
					buffer, message.len, message.flags);

			log.debug("received returned a response? "
					+ (response == null ? "false" : "true"));
			return response;
		}
	}

	/**
	 * Used by the service side to create a session for handling the client
	 * request.
	 * 
	 * @param name
	 *            The name of the service.
	 * @param cd
	 *            The connection descriptor
	 * @param replyTo
	 *            The client to respond to
	 * @return The session to use for the service invocation
	 * @throws ConnectionException
	 *             In case the transport cannot be established.
	 */
	Session createServiceSession(String name, int cd, Object replyTo)
			throws ConnectionException {
		log.trace("Creating the service session");
		if (serviceSession != null) {
			throw new ConnectionException(Connection.TPEPROTO,
					"Second service session creation attempt, was: "
							+ serviceSession.getCd() + " new: " + cd);
		}
		Transport transport = getTransport(name);
		serviceSession = new Session(this, transport, cd, replyTo);
		log.trace("Created the service session");
		return serviceSession;
	}

	/**
	 * Does this connection have any open sessions? Used to determine if a
	 * service has unanswered requests prior to a tpreturn.
	 * 
	 * @return True, if there are open conversations or asynchronous XATMI calls
	 *         open.
	 */
	boolean hasOpenSessions() {
		return sessions.size() > 0 || temporaryQueues.size() > 0;
	}

	/**
	 * Detach the open session, called during {@link Session#close()}
	 * 
	 * @param session
	 *            The session that is closing.
	 */
	void removeSession(Session session) {
		temporaryQueues.remove(session.getCd());
		// May be a no-op
		Session remove = sessions.remove(session.getCd());
		if (remove == null) {
			log.debug("Session did not exist: " + session.getCd());
		}

		if (session.equals(serviceSession)) {
			serviceSession = null;
		}
	}

	/**
	 * This class allows the session to notify the connection when a response is
	 * delivered or consumed.
	 */
	private class ResponseMonitorImpl implements ResponseMonitor {
		/**
		 * Handle the response update.
		 */
		public void responseReceived(int sessionId, boolean remove) {
			synchronized (tpGetAnySessions) {
				if (!remove) {
					log.trace("tpgetanyCallback adding: " + sessionId);
					tpGetAnySessions.add(sessionId);
					tpGetAnySessions.notify();
				} else {
					log.trace("tpgetanyCallback removing: " + sessionId);
					for (int i = 0; i < tpGetAnySessions.size(); i++) {
						if (sessionId == tpGetAnySessions.get(i)) {
							tpGetAnySessions.remove(i);
							log.trace("tpgetanyCallback removed: " + sessionId);
							break;
						}
					}
				}
			}
		}
	}
}
