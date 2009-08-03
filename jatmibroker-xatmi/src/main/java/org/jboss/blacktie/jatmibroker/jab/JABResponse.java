/*
 * JBoss, Home of Professional Open Source
 * Copyright 2008, Red Hat, Inc., and others contributors as indicated
 * by the @authors tag. All rights reserved.
 * See the copyright.txt in the distribution for a
 * full listing of individual contributors.
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License, v. 2.1.
 * This program is distributed in the hope that it will be useful, but WITHOUT A
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 * You should have received a copy of the GNU Lesser General Public License,
 * v.2.1 along with this distribution; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */
package org.jboss.blacktie.jatmibroker.jab;

import org.jboss.blacktie.jatmibroker.xatmi.Response;

/**
 * The JABResponse is obtained from the JABRemoteService
 * 
 * @see JABRemoteService
 */
public class JABResponse implements Message {

	/**
	 * The response obtained.
	 */
	private Response response;

	/**
	 * The class should be created from the JABRemoteService
	 */
	JABResponse() {
	}

	/**
	 * Set the raw response object
	 * 
	 * @param response
	 *            The response to set
	 */
	void setResponse(Response response) {
		this.response = response;
	}

	/**
	 * Get the content of the buffer
	 * 
	 * @return The content of the buffer
	 * @throws JABException
	 *             In case the content is malformed
	 */
	public byte[] getData() throws JABException {
		if (response == null) {
			throw new JABException(
					"Cannot read the content of the buffer, no response to read");
		}
		return response.getBuffer().getData();
	}

	/**
	 * This method should not be called by programmers.
	 * 
	 * @param string
	 *            No parameter is acceptable
	 * @throws JABException
	 *             In case the programmer calls this.
	 */
	public void setData(byte[] data) throws JABException {
		throw new JABException("Cannot set data on a received buffer");
	}

	/**
	 * Clear the content of the response
	 */
	void clear() {
		response = null;
	}

	/**
	 * This is the rcode that tpreturn was invoked with
	 * 
	 * @return The return code
	 */
	int getRCode() {
		if (response != null) {
			return response.getRcode();
		} else {
			return -1;
		}
	}
}
