package org.jboss.blacktie.jatmibroker.jab.factory;

import org.jboss.blacktie.jatmibroker.jab.JABException;

public class JABTransaction {

	private int timeout;

	JABTransaction(int timeout) throws JABException {
		this.timeout = timeout;
	}

	public void commit() throws JABException {

	}

	public void rollback() throws JABException {

	}
}
