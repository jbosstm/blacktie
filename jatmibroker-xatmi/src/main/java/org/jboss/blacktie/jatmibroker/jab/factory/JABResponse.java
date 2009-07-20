package org.jboss.blacktie.jatmibroker.jab.factory;

import org.jboss.blacktie.jatmibroker.xatmi.Response;

public class JABResponse extends JABBuffer {

	private Response response;

	JABResponse(Response response) {
		this.response = response;
	}

	public int getResponseCode() {
		return response.getRcode();
	}
}
