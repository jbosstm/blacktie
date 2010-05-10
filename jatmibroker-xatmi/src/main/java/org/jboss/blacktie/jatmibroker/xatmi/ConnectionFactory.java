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

	/**
	 * The connection factory will allocate a connection per thread.
	 */
	private static ThreadLocal<Connection> connections = new ThreadLocal<Connection>();

	/**
	 * Get the default connection factory
	 * 
	 * @return The connection factory
	 * @throws ConfigurationException
	 *             If the configuration cannot be parsed.
	 */
	public static synchronized ConnectionFactory getConnectionFactory()
			throws ConfigurationException {
		return new ConnectionFactory();
	}

	/**
	 * Create the connection factory
	 * 
	 * @throws ConfigurationException
	 *             In case the configuration could not be loaded
	 */
	private ConnectionFactory() throws ConfigurationException {
		AtmiBrokerEnvXML xml = new AtmiBrokerEnvXML();
		properties.putAll(xml.getProperties());
	}

	/**
	 * Get the connection for this thread.
	 * 
	 * @return The connection for this thread.
	 */
	public Connection getConnection() {
		Connection connection = connections.get();
		if (connection == null) {
			connection = new Connection(this, properties);
			connections.set(connection);
		}
		return connection;
	}

	/**
	 * Remove the connection from the factory after closure.
	 * 
	 * @param connection
	 *            The connection to remove.
	 */
	void removeConnection(Connection connection) {
		connections.set(null);
	}
}
