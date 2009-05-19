package org.jboss.blacktie.jatmibroker.xatmi.ejb;

import java.util.Properties;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.AtmiBrokerServerXML;
import org.jboss.blacktie.jatmibroker.core.AtmiBroker_ServerImpl;
import org.jboss.blacktie.jatmibroker.core.JAtmiBrokerException;
import org.jboss.blacktie.jatmibroker.xatmi.connector.ConnectorException;

/**
 * All blacktie services should extend this class so that they can be advertised
 */
public abstract class AbstractBlacktieService implements BlacktieService {
	private static final Logger log = LogManager
			.getLogger(AbstractBlacktieService.class);
	private AtmiBroker_ServerImpl server;
	private String serviceName;

	/**
	 * The default number of servants
	 */
	private static final int DEFAULT_POOL_SIZE = 5;

	/**
	 * Must have a no-arg constructor
	 * 
	 * @throws ConnectorException
	 */
	public AbstractBlacktieService() throws ConnectorException {
		Properties properties = new Properties();
		try {
			AtmiBrokerServerXML server = new AtmiBrokerServerXML(properties);
			String configDir = System.getProperty("blacktie.config.dir");
			server.getProperties(configDir);

			this.server = new AtmiBroker_ServerImpl(properties);
		} catch (Throwable t) {
			throw new ConnectorException(-1, "Could not load properties", t);
		}
	}

	/**
	 * Create a blacktie service with the specified name
	 * 
	 * @param serviceName
	 *            The name of the service
	 * @throws ConnectorException
	 *             If the service cannot be advertised
	 */
	public void tpadvertise(String serviceName, Class service)
			throws ConnectorException {
		try {
			log.debug("Advertising: " + serviceName);
			server.createService(serviceName, DEFAULT_POOL_SIZE, service,
					new AtmiBrokerCallbackConverterImpl());
			this.serviceName = serviceName;
			log.info("Advertised: " + serviceName);
		} catch (Throwable t) {
			String message = "Could not advertise: " + serviceName;
			log.error(message, t);
			throw new ConnectorException(-1, message, t);
		}
	}

	public void tpunadvertise() throws ConnectorException {
		try {
			log.debug("Unadvertising: " + serviceName);
			server.unbind(serviceName);
			log.info("Unadvertised: " + serviceName);
		} catch (JAtmiBrokerException e) {
			throw new ConnectorException(-1, e);
		}
	}
}
