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
package org.jboss.blacktie.jatmibroker.core.transport.hybrid.stomp;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.util.HashMap;
import java.util.Map;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.transport.JtsTransactionImple;
import org.jboss.blacktie.jatmibroker.core.transport.Sender;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;

public class StompSenderImpl implements Sender {
	private static int counter = 0;
	private static final Logger log = LogManager
			.getLogger(StompSenderImpl.class);
	private boolean closed;
	private StompManagement management;
	private String destinationName;
	private Socket socket;
	private OutputStream outputStream;
	private InputStream inputStream;
	private Map<String, Sender> conversationalMap;
	private String serviceName;

	public StompSenderImpl(StompManagement management, String serviceName,
			boolean conversational, Map<String, Sender> conversationalMap)
			throws ConnectionException, IOException {
		this.management = management;
		if (conversational) {
			this.destinationName = "/queue/BTC_" + serviceName;
		} else {
			this.destinationName = "/queue/BTR_" + serviceName;
		}

		this.serviceName = serviceName;
		this.socket = management.connect();
		this.outputStream = socket.getOutputStream();
		this.inputStream = socket.getInputStream();
		this.conversationalMap = conversationalMap;
		log.debug("Sender Created: " + destinationName);
	}

	public void send(Object replyTo, short rval, int rcode, byte[] data,
			int len, int correlationId, int flags, int ttl, String type,
			String subtype) throws ConnectionException {
		if (closed) {
			throw new ConnectionException(Connection.TPEPROTO, "Sender closed");
		}

		if (data == null) {
			data = new byte[1];
			len = 1;
		}
		if (len < 1) {
			throw new ConnectionException(Connection.TPEINVAL,
					"Length of buffer must be greater than 0");
		}

		log.debug("Sender sending: " + destinationName);
		Message message = new Message();

		message.setCommand("SEND");

		Map<String, String> headers = new HashMap<String, String>();
		try {
			String ior = JtsTransactionImple.getTransactionIOR();
			if (ior != null) {
				headers.put("messagecontrol", ior);
				log.debug("Sender sending IOR: " + ior);
			}
		} catch (Exception e) {
			throw new ConnectionException(Connection.TPETRAN, e.getMessage());
		}
		if (replyTo != null) {
			log.debug("Reply to: " + replyTo);
			headers.put("messagereplyto", (String) replyTo);
		}
		headers.put("servicename", destinationName);
		headers.put("messagecorrelationId", String.valueOf(correlationId));
		headers.put("messageflags", String.valueOf(flags));
		headers.put("messagerval", String.valueOf(rval));
		headers.put("messagercode", String.valueOf(rcode));
		headers.put("messagetype", type == null ? "" : type);
		headers.put("messagesubtype", subtype == null ? "" : subtype);

		if (ttl > 0) {
			headers.put("expires", String.valueOf(ttl));
		}
		synchronized (StompSenderImpl.class) {
			headers.put("receipt", "send-J-" + counter);
			counter++;
		}
		headers.put("destination", destinationName);
		message.setHeaders(headers);

		byte[] toSend = new byte[len];
		if (data != null) {
			int min = Math.min(toSend.length, data.length);
			System.arraycopy(data, 0, toSend, 0, min);
			headers.put("content-length", String.valueOf(toSend.length));
		}
		message.setBody(toSend);

		Message ack;
		try {
			management.send(message, this.outputStream);
			ack = management.receive(this.inputStream);
		} catch (IOException e) {
			throw new ConnectionException(Connection.TPEOS, e.getMessage());
		}
		if (!ack.getCommand().equals("RECEIPT")) {
			log.error(new String(ack.getBody()));
			throw new ConnectionException(Connection.TPENOENT, new String(
					ack.getBody()));
		}
		log.debug("sent message");
	}

	public void close() throws ConnectionException {
		log.debug("Sender closing: " + destinationName);
		if (closed) {
			throw new ConnectionException(Connection.TPEPROTO,
					"Sender already closed");
		}
		closed = true;
		try {
			log.debug("closing socket: " + socket);
			socket.close();
			log.debug("closed socket: " + socket);
			conversationalMap.remove(serviceName);
		} catch (Throwable t) {
			throw new ConnectionException(Connection.TPESYSTEM,
					"Could not send the message", t);
		}
	}

	public Object getSendTo() {
		return destinationName;
	}
}
