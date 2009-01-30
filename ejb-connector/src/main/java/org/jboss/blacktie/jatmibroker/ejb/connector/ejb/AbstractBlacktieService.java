package org.jboss.blacktie.jatmibroker.ejb.connector.ejb;

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
