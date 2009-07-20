package org.jboss.blacktie.jatmibroker.jab.factory;

import org.jboss.blacktie.jatmibroker.jab.JABException;
import org.jboss.blacktie.jatmibroker.xatmi.Response;

public class JABConnection {

	public JABConnection() throws JABException {

	}

	public JABResponse call(String serviceName, JABBuffer toSend,
			JABTransaction transaction) throws JABException {
		return new JABResponse(new Response((short) -1, -1, null, -1, -1));
	}

	public JABTransaction beginTransaction(int timeout) throws JABException {
		return new JABTransaction(timeout);
	}

	public void close() {

	}
}
