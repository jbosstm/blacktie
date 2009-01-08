package org.jboss.blacktie.jatmibroker.ejb.connector.ejb;

import java.rmi.RemoteException;

import javax.ejb.CreateException;
import javax.ejb.EJBHome;

import org.jboss.blacktie.jatmibroker.ejb.connector.BlacktieService;

/**
 * Required for EJB
 */
public interface BlacktieServiceHome extends EJBHome {

	/**
	 * Create a new Blacktie service
	 * 
	 * @return A reference to a blacktie service
	 * @throws CreateException
	 *             If the EJB could not be created
	 * @throws RemoteException
	 *             If there was a communication exception
	 */
	public BlacktieService create() throws CreateException, RemoteException;
}
