package org.jboss.blacktie.jatmibroker.xatmi;

import java.rmi.RemoteException;

/**
 * All extending classes must override the tpservice template method.
 */
public interface BlacktieService {
	/**
	 * This is a template method that should be implemented by classes to
	 * provide the service behaviour
	 * 
	 * @param svcinfo
	 *            The inbound parameters
	 * @return A response to the client
	 */
	public Response tpservice(TPSVCINFO svcinfo) throws RemoteException;
}
