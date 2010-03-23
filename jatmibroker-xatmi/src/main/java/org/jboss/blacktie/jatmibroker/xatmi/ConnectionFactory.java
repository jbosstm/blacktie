package org.jboss.blacktie.jatmibroker.xatmi;

import java.util.Properties;

import org.jboss.blacktie.jatmibroker.core.conf.AtmiBrokerClientXML;

/**
 * This is a factory that will create connections to remote Blacktie services.
 * 
 * @see Connection
 * @see ConnectionException
 */
public class ConnectionFactory {
	/**
	 * The properties inside the connection factory.
	 */
	private Properties properties = null;

	/**
	 * Get the default connection factory
	 * 
	 * @return The connection factory
	 * @throws ConnectionException
	 */
	public static synchronized ConnectionFactory getConnectionFactory()
			throws ConnectionException {
		return new ConnectionFactory();
	}

	/**
	 * Create the connection factory
	 * 
	 * @throws ConnectionException
	 *             In case the configuration could not be loaded
	 */
	private ConnectionFactory() throws ConnectionException {
		try {
			AtmiBrokerClientXML xml = new AtmiBrokerClientXML();
			properties = xml.getProperties();
		} catch (Exception e) {
			throw new ConnectionException(-1, "Could not load properties", e);
		}

	}

	private ConnectionFactory(Properties prop) {
		properties = prop;
	}

	/**
	 * Get the connection.
	 * 
	 * @return The connection
	 * @throws ConnectionException
	 */
	public Connection getConnection() throws ConnectionException {
		return new Connection(properties);
	}
}
