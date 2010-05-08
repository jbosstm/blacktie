package org.jboss.blacktie.jatmibroker.xatmi;

import java.util.Properties;

import org.jboss.blacktie.jatmibroker.core.conf.AtmiBrokerEnvXML;
import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;

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
	private Properties properties = new Properties();

	private static ThreadLocal<Connection> connections = new ThreadLocal<Connection>();

	/**
	 * Get the default connection factory
	 * 
	 * @return The connection factory
	 * @throws ConfigurationException
	 * @throws ConnectionException
	 */
	public static synchronized ConnectionFactory getConnectionFactory()
			throws ConfigurationException {
		return new ConnectionFactory();
	}

	/**
	 * Create the connection factory
	 * 
	 * @throws ConnectionException
	 *             In case the configuration could not be loaded
	 */
	private ConnectionFactory() throws ConfigurationException {
		AtmiBrokerEnvXML xml = new AtmiBrokerEnvXML();
		properties.putAll(xml.getProperties());

	}

	/**
	 * Get the connection.
	 * 
	 * @return The connection
	 */
	public Connection getConnection() {
		Connection connection = connections.get();
		if (connection == null) {
			connection = new Connection(this, properties);
			connections.set(connection);
		}
		return connection;
	}

	void removeConnection(Connection connection) {
		connections.set(null);
	}
}
