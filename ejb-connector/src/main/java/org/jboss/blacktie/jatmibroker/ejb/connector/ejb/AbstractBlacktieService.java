package org.jboss.blacktie.jatmibroker.ejb.connector.ejb;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.AtmiBroker_ServerImpl;
import org.jboss.blacktie.jatmibroker.ejb.connector.BlacktieService;

/**
 * All blacktie services should extend this class so that they can be advertised
 */
public abstract class AbstractBlacktieService implements BlacktieService {

	/**
	 * Must have a no-arg constructor
	 */
	public AbstractBlacktieService() {

	}
}
