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

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Properties;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.transport.Message;
import org.jboss.blacktie.jatmibroker.transport.Receiver;
import org.jboss.blacktie.jatmibroker.transport.Transport;
import org.jboss.blacktie.jatmibroker.transport.TransportFactory;

/**
 * This is the connection to remote Blacktie services.
 */
public class Connection {

	private static final Logger log = LogManager.getLogger(Connection.class);

	// AVAILABLE FLAGS
	public static int TPNOBLOCK = 0x00000001;
	public static int TPSIGRSTRT = 0x00000002;
	public static int TPNOREPLY = 0x00000004;
	public static int TPNOTRAN = 0x00000008;
	public static int TPTRAN = 0x00000010;
	public static int TPNOTIME = 0x00000020;
	public static int TPGETANY = 0x00000080;
	public static int TPNOCHANGE = 0x00000100;
	public static int TPCONV = 0x00000400;
	public static int TPSENDONLY = 0x00000800;
	public static int TPRECVONLY = 0x00001000;

	// ERROR CONDITIONS
	public static int TPEBADDESC = 2;
	public static int TPEBLOCK = 3;
	public static int TPEINVAL = 4;
	public static int TPELIMIT = 5;
	public static int TPENOENT = 6;
	public static int TPEOS = 7;
	public static int TPEPROTO = 9;
	public static int TPESVCERR = 10;
	public static int TPESVCFAIL = 11;
	public static int TPESYSTEM = 12;
	public static int TPETIME = 13;
	public static int TPETRAN = 14;
	public static int TPGOTSIG = 15;
	public static int TPEITYPE = 17;
	public static int TPEOTYPE = 18;
	public static int TPEEVENT = 22;
	public static int TPEMATCH = 23;

	public static int XATMI_SERVICE_NAME_LENGTH = 15;

	private static int nextId;

	private Map<String, Transport> transports = new HashMap<String, Transport>();

	/**
	 * Any local temporary queues created in this connection
	 */
	private Map<java.lang.Integer, Receiver> temporaryQueues = new HashMap<java.lang.Integer, Receiver>();

	private Properties properties;

	private Map<Integer, Session> sessions = new HashMap<Integer, Session>();

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
	public int tpacall(String svc, Buffer buffer, int len, int flags)
			throws ConnectionException {
		svc = svc.substring(0, Math.min(Connection.XATMI_SERVICE_NAME_LENGTH,
				svc.length()));
		int correlationId = nextId++;
		Transport transport = getTransport(svc);
		Receiver endpoint = transport.createReceiver();
		temporaryQueues.put(correlationId, endpoint);
		// TODO HANDLE TRANSACTION
		transport.getSender(svc).send(endpoint.getReplyTo(), (short) 0, 0,
				buffer.getData(), len, correlationId, flags);
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
	public void tpcancel(int cd, int flags) throws ConnectionException {
		Receiver endpoint = temporaryQueues.get(cd);
		endpoint.close();
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
		Response toReturn = receive(cd, flags);
		Session session = sessions.remove(cd);
		if (session != null) {
			session.close();
		}
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
	public Session tpconnect(String svc, Buffer buffer, int len, int flags)
			throws ConnectionException {
		svc = svc.substring(0, Math.min(Connection.XATMI_SERVICE_NAME_LENGTH,
				svc.length()));
		// Initiate the session
		int cd = tpacall(svc, buffer, len, flags);
		Receiver endpoint = temporaryQueues.get(cd);
		// Return a handle to allow the connection to send/receive data on
		Session session = new Session(getTransport(svc), cd, endpoint);
		sessions.put(cd, session);
		return session;
	}

	/**
	 * Close any resources associated with this connection
	 * 
	 * @throws ConnectionException
	 */
	public void close() throws ConnectionException {
		Iterator<Receiver> receivers = temporaryQueues.values().iterator();
		while (receivers.hasNext()) {
			Receiver receiver = receivers.next();
			receiver.close();
		}
		temporaryQueues.clear();
		Iterator<Transport> transports = this.transports.values().iterator();
		while (transports.hasNext()) {
			Transport transport = transports.next();
			transport.close();
		}
		this.transports.clear();
	}

	private Transport getTransport(String serviceName)
			throws ConnectionException {
		Transport toReturn = transports.get(serviceName);
		if (toReturn == null) {
			try {
				toReturn = TransportFactory.loadTransportFactory(serviceName,
						properties).createTransport();
			} catch (ConfigurationException e) {
				throw new ConnectionException(-1, "Could not load properties",
						e);
			}
			transports.put(serviceName, toReturn);
		}
		return toReturn;
	}

	private Response receive(int cd, int flags) throws ConnectionException {
		Receiver endpoint = temporaryQueues.remove(cd);
		Message m = endpoint.receive(flags);
		// TODO WE SHOULD BE SENDING THE TYPE, SUBTYPE AND CONNECTION ID?
		Buffer received = new Buffer(null, null);
		received.setData(m.data);
		return new Response(m.rval, m.rcode, received, m.len, m.flags);
	}
}
