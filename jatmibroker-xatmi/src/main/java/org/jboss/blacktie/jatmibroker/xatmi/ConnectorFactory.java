package org.jboss.blacktie.jatmibroker.xatmi;

/**
 * This is a factory that will create connectors to remote Blacktie services.
 */
public interface ConnectorFactory {

	/**
	 * Get the connector
	 * 
	 * @return The connector
	 * @throws ConnectorException
	 */
	public Connector getConnector(String username, String password)
			throws ConnectorException;
}
