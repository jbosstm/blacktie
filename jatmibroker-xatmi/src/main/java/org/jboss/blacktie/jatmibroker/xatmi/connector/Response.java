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
package org.jboss.blacktie.jatmibroker.xatmi.connector;

import java.io.Serializable;

import org.jboss.blacktie.jatmibroker.xatmi.connector.buffers.Buffer;

/**
 * This class encapsulates the response from the remote service and the return
 * code
 */
public class Response implements Serializable {

	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;

	/**
	 * The return value
	 */
	short rval;

	/**
	 * The return code
	 */
	int rcode;

	/**
	 * The flags to return
	 */
	int flags;

	/**
	 * The response from the server
	 */
	private Buffer response;

	public Response(short rval, int rcode, Buffer response, int flags) {
		this.rval = rval;
		this.rcode = rcode;
		this.response = response;
		this.flags = flags;
	}

	/**
	 * Get the return value
	 * 
	 * @return The return value
	 */
	public short getRval() {
		return rval;
	}

	/**
	 * Get the return code
	 * 
	 * @return The return code
	 */
	public int getRcode() {
		return rcode;
	}

	/**
	 * Get the response out of the wrapper
	 * 
	 * @return The response
	 */
	public Buffer getResponse() {
		return response;
	}

	public int getFlags() {
		return flags;
	}
}
