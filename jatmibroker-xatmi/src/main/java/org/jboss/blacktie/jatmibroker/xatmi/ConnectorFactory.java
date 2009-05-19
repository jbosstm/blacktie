package org.jboss.blacktie.jatmibroker.xatmi;

import java.util.Properties;

import org.jboss.blacktie.jatmibroker.conf.AtmiBrokerClientXML;

/**
 * This is a factory that will create connectors to remote Blacktie services.
 */
public class ConnectorFactory {
	private Properties properties = null;

	/**
	 * Get the default connector factory
	 * 
	 * @return The connector factory
	 * @throws ConnectorException
	 */
	public static synchronized ConnectorFactory getConnectorFactory(
			String configurationDirectory) throws ConnectorException {
		return new ConnectorFactory(configurationDirectory);
	}

	/**
	 * Create the connector factory
	 * 
	 * @throws ConnectorException
	 */
	private ConnectorFactory(String configurationDirectory)
			throws ConnectorException {
		try {
			AtmiBrokerClientXML xml = new AtmiBrokerClientXML();
			properties = xml.getProperties(configurationDirectory);
		} catch (Exception e) {
			throw new ConnectorException(-1, "Could not load properties", e);
		}

	}

	/**
	 * Get the connector
	 * 
	 * @return The connector
	 * @throws ConnectorException
	 */
	public Connector getConnector(String username, String password)
			throws ConnectorException {
		return new Connector(properties, username, password);
	}
}
