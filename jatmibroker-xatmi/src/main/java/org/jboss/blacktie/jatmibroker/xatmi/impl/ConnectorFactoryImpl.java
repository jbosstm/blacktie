package org.jboss.blacktie.jatmibroker.xatmi.impl;

import java.util.HashMap;
import java.util.Map;
import java.util.Properties;

import org.jboss.blacktie.jatmibroker.core.conf.AtmiBrokerServerXML;
import org.jboss.blacktie.jatmibroker.xatmi.Connector;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectorException;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectorFactory;

/**
 * The connector factory allows us to create connections to the remote servers.
 */
public class ConnectorFactoryImpl implements ConnectorFactory {
	private static Map<Properties, ConnectorFactoryImpl> connectorFactories = new HashMap<Properties, ConnectorFactoryImpl>();

	/**
	 * The properties to connect with
	 */
	private Properties properties;

	/**
	 * Get the connector factory
	 * 
	 * @param properties
	 *            The properties to use
	 * @return The connector factory
	 */
	public static synchronized ConnectorFactory getConnectorFactory(
			Properties properties) {
		ConnectorFactoryImpl connectorFactoryImpl = connectorFactories
				.get(properties);
		if (connectorFactoryImpl == null) {
			connectorFactoryImpl = new ConnectorFactoryImpl(properties);
			connectorFactories.put(properties, connectorFactoryImpl);
		}
		return connectorFactoryImpl;
	}

	/**
	 * Get the default connector factory
	 * 
	 * @return The connector factory
	 * @throws ConnectorException
	 */
	public static synchronized ConnectorFactory getConnectorFactory()
			throws ConnectorException {
		Properties properties = new Properties();
		/*
		 * try { InputStream resourceAsStream =
		 * Thread.currentThread().getContextClassLoader
		 * ().getResourceAsStream("blacktie.properties");
		 * properties.load(resourceAsStream); } catch (Throwable t) { throw new
		 * ConnectorException(-1, "Could not load properties", t); }
		 */

		try {
			AtmiBrokerServerXML server = new AtmiBrokerServerXML(properties);
			String configDir = System.getProperty("blacktie.config.dir");
			server.getProperties(configDir);
		} catch (Exception e) {
			throw new ConnectorException(-1, "Could not load properties", e);
		}

		/*
		 * ConnectorFactoryImpl connectorFactoryImpl =
		 * connectorFactories.get(properties); if (connectorFactoryImpl == null)
		 * { connectorFactoryImpl = new ConnectorFactoryImpl(properties); }
		 * 
		 * return connectorFactoryImpl;
		 */
		return getConnectorFactory(properties);
	}

	/**
	 * Create the connector factory
	 */
	protected ConnectorFactoryImpl(Properties properties) {
		this.properties = properties;
	}

	/**
	 * Create a connector
	 */
	public Connector getConnector() throws ConnectorException {
		return new ConnectorImpl(properties, "", "");
	}
}
