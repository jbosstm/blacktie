package org.jboss.narayana.blacktie.jatmibroker.core.transport.hybrid.stomp;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Properties;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.narayana.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.narayana.blacktie.jatmibroker.xatmi.ConnectionException;

/**
 * This class could be extended to support connection reconnection.
 */
public class StompManagement {
	private static final Logger log = LogManager
			.getLogger(StompManagement.class);

	private String username;

	private String password;

	private String host;

	private int port;

	public StompManagement(Properties properties) throws NumberFormatException {
		log.debug("Creating JMSManagement: " + this);

		username = (String) properties.get("StompConnectUsr");
		password = (String) properties.get("StompConnectPwd");
		host = (String) properties.get("StompConnectHost");
		port = Integer.parseInt((String) properties.get("StompConnectPort"));

		log.debug("Created JMSManagement: " + this);
	}

	public void close() {
		log.debug("close");
	}

	public Socket connect() throws IOException, ConnectionException {
		Socket socket = new Socket(host, port);
		InputStream inputStream = socket.getInputStream();
		OutputStream outputStream = socket.getOutputStream();

		Map<String, String> headers = new HashMap<String, String>();
		headers.put("login", username);
		headers.put("passcode", password);
		Message message = new Message();
		message.setCommand("CONNECT");
		message.setHeaders(headers);
		send(message, outputStream);
		Message received = receive(inputStream);
		if (received.getCommand().equals("ERROR")) {
			throw new ConnectionException(Connection.TPESYSTEM, new String(
					received.getBody()));
		}

		log.debug("Created socket: " + socket + " input: " + inputStream
				+ "output: " + outputStream);
		return socket;
	}

	public void send(Message message, OutputStream outputStream)
			throws IOException {
		log.trace("Writing on: " + outputStream);
		StringBuffer toSend = new StringBuffer(message.getCommand().toString());
		toSend.append("\n");

		Iterator<String> keys = message.getHeaders().keySet().iterator();
		while (keys.hasNext()) {
			String key = keys.next();
			String value = message.getHeaders().get(key);
			toSend.append(key);
			toSend.append(":");
			toSend.append(value);
			toSend.append("\n");
		}
		toSend.append("\n");
		outputStream.write(toSend.toString().getBytes());

		if (message.getBody() != null) {
			outputStream.write(message.getBody());
		}

		outputStream.write("\000\n\n".getBytes());
		log.trace("Wrote on: " + outputStream);
	}

	public Message receive(InputStream inputStream) throws IOException {
		log.trace("Reading from: " + inputStream);
		Message message = new Message();
		message.setCommand(readLine(inputStream));
		log.trace(message.getCommand());
		Map<String, String> headers = new HashMap<String, String>();
		String header;
		while ((header = readLine(inputStream)).length() > 0) {
			int sep = header.indexOf(':');
			String key = header.substring(0, sep);
			String value = header.substring(sep + 1, header.length());
			headers.put(key.trim(), value.trim());
			log.trace("Header: " + key + ":" + value);
		}
		if (!message.getCommand().equals("ERROR")) {
			message.setHeaders(headers);
			String contentLength = headers.get("content-length");
			if (contentLength != null) {
				byte[] body = new byte[Integer.valueOf(contentLength)];
				int offset = 0;
				while (offset != body.length) {
					offset = inputStream.read(body, offset, body.length
							- offset);
				}
				message.setBody(body);
			}
			readLine(inputStream);
			readLine(inputStream);
		} else {
			message.setBody(headers.get("message").getBytes());
			// Drain off the error message
			String read = null;
			do {
				read = readLine(inputStream);
				if (read != null)
					log.debug(read);
			} while (read != null);
			readLine(inputStream);
		}
		log.trace("Read from: " + inputStream);
		return message;
	}

	private String readLine(InputStream inputStream) throws IOException {
		String toReturn = null;
		char[] read = new char[0];
		char c = (char) inputStream.read();
		while (!(c == '\n') && c != '\000') {
			char[] tmp = new char[read.length + 1];
			System.arraycopy(read, 0, tmp, 0, read.length);
			tmp[read.length] = c;
			read = tmp;
			c = (char) inputStream.read();
		}
		if (c == '\000') {
			log.trace("returning null");
		} else {
			toReturn = new String(read);
			log.trace("returning: " + toReturn);
		}
		return toReturn;
	}
}
