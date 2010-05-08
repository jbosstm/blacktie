package org.jboss.blacktie.jatmibroker.xatmi;

/**
 * All XATMI services must implement the tpservice method.
 */
public interface Service {
	/**
	 * This is a method that should be implemented by classes to provide the
	 * service behavior
	 * 
	 * @param svcinfo
	 *            The inbound parameters
	 * @return A response to the client
	 */
	public Response tpservice(TPSVCINFO svcinfo);
}
