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
import org.jboss.blacktie.jatmibroker.JAtmiBrokerException;
import org.jboss.blacktie.jatmibroker.transport.Message;
import org.jboss.blacktie.jatmibroker.transport.Receiver;
import org.jboss.blacktie.jatmibroker.transport.Transport;
import org.jboss.blacktie.jatmibroker.transport.TransportFactory;

/**
 * This is the connection to remote Blacktie services.
 */
public class Connection {

	private static final Logger log = LogManager.getLogger(Connection.class);

	/**
	 * A reference to the proxy to issue calls on
	 */
	private Transport transport;

	// AVAILABLE FLAGS
	int TPNOBLOCK = 0x00000001;
	int TPSIGRSTRT = 0x00000002;
	int TPNOREPLY = 0x00000004;
	int TPNOTRAN = 0x00000008;
	int TPTRAN = 0x00000010;
	int TPNOTIME = 0x00000020;
	int TPGETANY = 0x00000080;
	int TPNOCHANGE = 0x00000100;
	int TPCONV = 0x00000400;
	int TPSENDONLY = 0x00000800;
	int TPRECVONLY = 0x00001000;

	// ERROR CONDITIONS
	public int TPEBADDESC = 2;
	public int TPEBLOCK = 3;
	public int TPEINVAL = 4;
	public int TPELIMIT = 5;
	public int TPENOENT = 6;
	public int TPEOS = 7;
	public int TPEPROTO = 9;
	public int TPESVCERR = 10;
	public int TPESVCFAIL = 11;
	public int TPESYSTEM = 12;
	public int TPETIME = 13;
	public int TPETRAN = 14;
	public int TPGOTSIG = 15;
	public int TPEITYPE = 17;
	public int TPEOTYPE = 18;
	public int TPEEVENT = 22;
	public int TPEMATCH = 23;

	private static int nextId;

	/**
	 * Any local temporary queues created in this connection
	 */
	private Map<java.lang.Integer, Receiver> temporaryQueues = new HashMap<java.lang.Integer, Receiver>();

	/**
	 * The connection
	 * 
	 * @param properties
	 * @param username
	 * @param password
	 * @throws ConnectionException
	 */
	Connection(Properties properties, String username, String password)
			throws ConnectionException {
		try {

			transport = TransportFactory.loadTransportFactory(properties)
					.createTransport(username, password);
		} catch (JAtmiBrokerException e) {
			throw new ConnectionException(-1, "Could not load properties", e);
		}
	}

	/**
	 * Allocate a new typed buffer
	 * 
	 * @param type
	 *            The type of the buffer
	 * @param subtype
	 *            The subtype of the buffer
	 * @param length
	 *            The length of the buffer
	 * @return The new buffer
	 */
	public Buffer tpalloc(String type, String subtype, int length) {
		return new Buffer(type, subtype, length);
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
	public Response tpcall(String svc, Buffer buffer, int flags)
			throws ConnectionException {
		int cd = tpacall(svc, buffer, flags);
		return tpgetrply(cd, flags);
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
	int tpacall(String svc, Buffer buffer, int flags)
			throws ConnectionException {
		try {
			int correlationId = nextId++;
			Receiver endpoint = getReceiver(correlationId);
			// TODO HANDLE TRANSACTION
			transport.getSender(svc).send(endpoint.getReplyTo(), (short) 0, 0,
					buffer.getData(), buffer.getLen(), correlationId, flags);
			return correlationId;
		} catch (JAtmiBrokerException e) {
			throw new ConnectionException(-1, "Could not send the request", e);
		}
	}

	/**
	 * Cancel the outstanding asynchronous call.
	 * 
	 * @param cd
	 *            The connection descriptor
	 * @param flags
	 *            The flags to use
	 */
	void tpcancel(int cd, int flags) throws ConnectionException {
		Receiver endpoint = getReceiver(cd);
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
		Receiver endpoint = getReceiver(cd);
		Message m = endpoint.receive(flags);
		// TODO WE SHOULD BE SENDING THE TYPE, SUBTYPE AND CONNECTION ID?
		Buffer received = new Buffer("TODO", null, m.len);
		received.setData(m.data);
		return new Response(m.rval, m.rcode, received, m.flags);
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
	public Session tpconnect(String svc, Buffer buffer, int flags)
			throws ConnectionException {
		// Initialate the session
		int cd = tpacall(svc, buffer, flags);
		// Return a handle to allow the connection to send/receive data on
		return new Session(transport, cd, getReceiver(cd));
	}

	/**
	 * Close any resources associated with this connection
	 */
	public void close() {
		Iterator<Receiver> receivers = temporaryQueues.values().iterator();
		while (receivers.hasNext()) {
			Receiver receiver = receivers.next();
			receiver.close();
		}
		transport.close();
	}

	private Receiver getReceiver(int id) throws ConnectionException {
		Receiver receiver = temporaryQueues.get(id);
		if (receiver == null) {
			try {
				receiver = transport.createReceiver(id);
				temporaryQueues.put(id, receiver);
			} catch (Throwable t) {
				throw new ConnectionException(-1,
						"Could not create a temporary queue", t);
			}
		}
		return receiver;

	}
}
