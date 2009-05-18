package org.jboss.blacktie.jatmibroker.xatmi.connector;

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
	public Connector getConnector() throws ConnectorException;
}
