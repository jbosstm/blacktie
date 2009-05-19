package org.jboss.blacktie.jatmibroker.xatmi.impl;

import java.util.Properties;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.JAtmiBrokerException;
import org.jboss.blacktie.jatmibroker.core.Connection;
import org.jboss.blacktie.jatmibroker.core.Message;
import org.jboss.blacktie.jatmibroker.core.Receiver;
import org.jboss.blacktie.jatmibroker.core.corba.ConnectionFactoryImpl;
import org.jboss.blacktie.jatmibroker.xatmi.Connector;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectorException;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.buffers.Buffer;

/**
 * Handles the connector to the server
 */
public class ConnectorImpl implements Connector {
	private static final Logger log = LogManager.getLogger(ConnectorImpl.class);

	/**
	 * A reference to the proxy to issue calls on
	 */
	private Connection connection;

	/**
	 * The connector itself
	 * 
	 * @param properties
	 * @param username
	 * @param password
	 * @throws ConnectorException
	 */
	public ConnectorImpl(Properties properties, String username, String password)
			throws ConnectorException {
		try {
			connection = new ConnectionFactoryImpl(properties)
					.createConnection(username, password);
		} catch (JAtmiBrokerException e) {
			throw new ConnectorException(-1, e);
		}
	}

	public Response tpcall(String svc, Buffer idata, int flags)
			throws ConnectorException {

		try {
			// TODO HANDLE TRANSACTION
			Receiver endpoint = connection.getReceiver(0);
			connection.getSender(svc).send(endpoint.getReplyTo(), (short) 0, 0,
					idata.getData(), idata.getSize(), 0, flags);
			Message receive = endpoint.receive(flags);
			// TODO WE SHOULD BE SENDING THE TYPE, SUBTYPE AND CONNECTION ID?
			Buffer buffer = new Buffer("unknown", "unknown", receive.len);
			buffer.setData(receive.data);
			return new Response(receive.rval, receive.rcode, buffer,
					receive.flags);
		} catch (JAtmiBrokerException e) {
			throw new ConnectorException(-1, e);
		}
	}

	public void close() {
		connection.close();
	}
}
