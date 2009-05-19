package org.jboss.blacktie.jatmibroker.xatmi.impl;

import java.util.Properties;

import org.jboss.blacktie.jatmibroker.conf.AtmiBrokerServerXML;
import org.jboss.blacktie.jatmibroker.xatmi.Connector;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectorException;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectorFactory;

/**
 * The connector factory allows us to create connections to the remote servers.
 */
public class ConnectorFactoryImpl implements ConnectorFactory {
	private Properties properties = null;

	/**
	 * Get the default connector factory
	 * 
	 * @return The connector factory
	 * @throws ConnectorException
	 */
	public static synchronized ConnectorFactory getConnectorFactory(
			String configurationDirectory) throws ConnectorException {
		return new ConnectorFactoryImpl(configurationDirectory);
	}

	/**
	 * Create the connector factory
	 * 
	 * @throws ConnectorException
	 */
	private ConnectorFactoryImpl(String configurationDirectory)
			throws ConnectorException {
		try {
			AtmiBrokerServerXML server = new AtmiBrokerServerXML();
			properties = server.getProperties(configurationDirectory);
		} catch (Exception e) {
			throw new ConnectorException(-1, "Could not load properties", e);
		}

	}

	/**
	 * Create a connector
	 */
	public Connector getConnector(String username, String password)
			throws ConnectorException {
		return new ConnectorImpl(properties, username, password);
	}
}
