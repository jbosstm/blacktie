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
package org.jboss.blacktie.jatmibroker.jab.factory;

import org.jboss.blacktie.jatmibroker.jab.JABException;
import org.jboss.blacktie.jatmibroker.xatmi.Buffer;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.blacktie.jatmibroker.xatmi.X_OCTET;

/**
 * The JABBuffer allows the programmer to invoked XATMI services which accept
 * X_OCTET parameters.
 * 
 * @see JABResponse
 */
public class JABBuffer {

	/**
	 * The underlying buffer object
	 */
	private Buffer value;

	/**
	 * Is the buffer for sending or has it been received
	 */
	private boolean readOnly;

	/**
	 * Create a new JABBuffer which is suitable for sending to a remote XATMI
	 * service
	 */
	public JABBuffer() {

	}

	/**
	 * Used as an extension point by the JABResponse class
	 * 
	 * @param buffer
	 *            The buffer that was received from the remote service
	 */
	JABBuffer(Buffer buffer) {
		readOnly = true;
		this.value = buffer;
	}

	/**
	 * Set the value of the outbound buffer to a char[]
	 * 
	 * @param value
	 *            The content to send to the remote service
	 * @throws JABException
	 *             In case the buffer is in read only mode or the value was
	 *             malformed.
	 */
	public void setValue(byte[] value) throws JABException {
		if (!readOnly) {
			try {
				this.value = new X_OCTET();
			} catch (ConnectionException e) {
				throw new JABException("Could not create an X_OCTET buffer", e);
			}
			this.value.setData(value);
		} else {
			throw new JABException(
					"The buffer was received from a remote service and may not be modified");
		}
	}

	/**
	 * Retrieve the content of the buffer
	 * 
	 * @return The message from the remote service
	 * @throws JABException
	 *             In case the content returned from the server was not of the
	 *             expected format
	 */
	public byte[] getValue() throws JABException {
		if (value != null) {
			return value.getData();
		} else {
			return null;
		}
	}
}
