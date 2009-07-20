package org.jboss.blacktie.jatmibroker.jab.factory;

import java.util.HashMap;
import java.util.Map;

import org.jboss.blacktie.jatmibroker.jab.JABException;

public class JABConnectionFactory {

	private static JABConnectionFactory instance;

	public static JABConnectionFactory getInstance() throws JABException {
		return instance;
	}

	private Map<String, JABConnection> connections = new HashMap<String, JABConnection>();

	private JABConnectionFactory() throws JABException {
	}

	public JABConnection getConnection(String connectionName)
			throws JABException {
		JABConnection toReturn = connections.get(connectionName);
		if (toReturn == null) {
			toReturn = new JABConnection();
			connections.put(connectionName, toReturn);
		}
		return toReturn;
	}

	public void closeConnection(String connectionName) {
		JABConnection connection = connections.get(connectionName);
		if (connection != null) {
			connection.close();
		}
	}
}
