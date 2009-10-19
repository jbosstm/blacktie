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

import org.jboss.blacktie.jatmibroker.xatmi.Buffer;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.blacktie.jatmibroker.xatmi.X_OCTET;

/**
 * The JABRequest class wraps the output parameter to the service.
 * 
 * @see JABRemoteService
 */
public class JABRequest implements Message {

	/**
	 * The buffer to send.
	 */
	private Buffer request;

	/**
	 * The request should be created from the JABRemoteService getRequest
	 * method.
	 */
	JABRequest() {
	}

	/**
	 * Get the current content of the buffer.
	 * 
	 * @return The content of the buffer
	 * @throws JABException
	 *             if the data is malformed
	 */
	public byte[] getData() {
		return request.getData();
	}

	/**
	 * Set the content of the buffer
	 * 
	 * @param string
	 *            The content of the buffer to set
	 * @throws JABException
	 *             In case the content is malformed
	 */
	public void setData(byte[] data) throws JABException {
		try {
			request = new X_OCTET();
		} catch (ConnectionException e) {
			throw new JABException("Could not create an X_OCTET buffer", e);
		}
		request.setData(data);
	}

	/**
	 * An internal method to access the actual buffer.
	 * 
	 * @return The buffer
	 */
	Buffer getRequest() {
		return request;
	}

	/**
	 * Clear the real buffer
	 */
	void clear() {
		request = null;
	}

}
