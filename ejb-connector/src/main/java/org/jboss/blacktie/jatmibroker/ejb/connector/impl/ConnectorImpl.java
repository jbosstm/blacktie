package org.jboss.blacktie.jatmibroker.ejb.connector.impl;

import java.util.Properties;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.AtmiBrokerServerImpl;
import org.jboss.blacktie.jatmibroker.core.AtmiBroker_ServerImpl;
import org.jboss.blacktie.jatmibroker.core.JAtmiBrokerException;
import org.jboss.blacktie.jatmibroker.core.proxy.AtmiBrokerServer;
import org.jboss.blacktie.jatmibroker.ejb.connector.Connector;
import org.jboss.blacktie.jatmibroker.ejb.connector.ConnectorException;
import org.jboss.blacktie.jatmibroker.ejb.connector.Response;
import org.jboss.blacktie.jatmibroker.ejb.connector.buffers.Buffer;
import org.omg.CORBA.IntHolder;

/**
 * Handles the connector to the server
 */
public class ConnectorImpl implements Connector {
	private static final Logger log = LogManager.getLogger(ConnectorImpl.class);

	/**
	 * The default number of servants
	 */
	private static final int servantCacheSize = 5;

	/**
	 * A reference to the server if we advertise items
	 */
	private AtmiBroker_ServerImpl server;

	/**
	 * A reference to the proxy to issue calls on
	 */
	private AtmiBrokerServer proxy;

	/**
	 * The properties to connect with
	 */
	private Properties properties;

	/**
	 * The username to connect with
	 */
	private String username;

	/**
	 * The password to connect with
	 */
	private String password;

	/**
	 * The connector itself
	 * 
	 * @param properties
	 * @param username
	 * @param password
	 * @throws ConnectorException
	 */
	public ConnectorImpl(Properties properties, String username, String password) throws ConnectorException {
		this.properties = properties;
		this.username = username;
		this.password = password;
	}

	public Response tpcall(String svc, Buffer idata, int flags) throws ConnectorException {

		AtmiBroker.octetSeqHolder odata = new AtmiBroker.octetSeqHolder();
		org.omg.CORBA.IntHolder olen = new org.omg.CORBA.IntHolder();
		try {
			// TODO HANDLE TRANSACTION
			getProxy().getServiceManagerProxy(svc).send_data(null, false, idata.getData(), idata.getSize(), flags, 0, null);
			IntHolder event = new IntHolder();
			getProxy().dequeue_data(odata, olen, flags, event);
		} catch (JAtmiBrokerException e) {
			throw new ConnectorException(-1, e);
		}

		// TODO WE SHOULD BE SENDING THE TYPE, SUBTYPE AND CONNECTION ID?
		Buffer buffer = new Buffer("unknown", "unknown", odata.value.length);
		buffer.setData(odata.value);
		return new Response(buffer);
	}

	private synchronized AtmiBrokerServer getProxy() throws ConnectorException {
		if (proxy == null) {
			try {
				proxy = AtmiBrokerServerImpl.getProxy(properties, username, password);
			} catch (JAtmiBrokerException e) {
				throw new ConnectorException(-1, e);
			}
		}
		return proxy;
	}

	/**
	 * Create a blacktie service with the specified name
	 * 
	 * @param serviceName
	 *            The name of the service
	 * @throws ConnectorException
	 *             If the service cannot be advertised
	 */
	public void tpadvertise(String serviceName, Class service) throws ConnectorException {
		try {
			AtmiBroker_ServerImpl server = getServer();
			log.info("Advertising: " + serviceName);
			server.createAtmiBroker_ServiceManagerImpl(serviceName);
			server.createAtmiBroker_ServiceFactoryImpl(serviceName, servantCacheSize, service, new AtmiBrokerCallbackConverterImpl());
			log.info("Advertised: " + serviceName);
		} catch (Throwable t) {
			String message = "Could not advertise: " + serviceName;
			log.error(message);
			throw new ConnectorException(-1, message, t);
		}
	}

	private synchronized AtmiBroker_ServerImpl getServer() throws JAtmiBrokerException {
		if (server == null) {
			server = new AtmiBroker_ServerImpl(properties);
			server.bind();
		}
		return server;
	}

	public void tpunadvertise(String serviceName) throws ConnectorException {
		try {
			log.debug("ejbRemove() on obj " + this);
			getServer().unbind(serviceName);
		} catch (JAtmiBrokerException e) {
			throw new ConnectorException(-1, e);
		}
	}
}
