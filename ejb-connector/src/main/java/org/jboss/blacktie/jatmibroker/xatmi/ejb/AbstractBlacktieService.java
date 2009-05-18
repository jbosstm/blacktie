package org.jboss.blacktie.jatmibroker.xatmi.ejb;


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
