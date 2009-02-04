/*
 * JBoss, Home of Professional Open Source
 * Copyright 2008, Red Hat Middleware LLC, and others contributors as indicated
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
package org.jboss.blacktie.jatmibroker.ejb.connector.buffers;

import java.io.Serializable;

/**
 * The typed buffer is the base class for all buffers.
 */
public class Buffer implements Serializable {
	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;

	/**
	 * The data
	 */
	private byte[] data;

	/**
	 * The size of the buffer.
	 */
	private int size;

	/**
	 * The subtype.
	 */
	private String subtype;

	/**
	 * The type
	 */
	private String type;

	/**
	 * Create a new typed buffer with a size
	 * 
	 * @param size
	 *            The size to reserve
	 */
	public Buffer(String type, String subtype, int size) {
		setSize(size);
		data = new byte[size];
	}

	/**
	 * Get the size
	 * 
	 * @return The size
	 */
	public int getSize() {
		return size;
	}

	/**
	 * Set the size
	 * 
	 * @param size
	 *            The size
	 */
	public void setSize(int size) {
		this.size = size;
	}

	/**
	 * Get the subtype
	 * 
	 * @return The subtype
	 */
	public String getSubtype() {
		return subtype;
	}

	/**
	 * Get the type
	 * 
	 * @return The type
	 */
	public String getType() {
		return type;
	}

	/**
	 * Get the data to be sent.
	 * 
	 * @return The data
	 */
	public byte[] getData() {
		return data;
	}

	/**
	 * Set the data
	 * 
	 * @param data
	 *            The data
	 */
	public void setData(byte[] data) {
		this.data = data;
	}
}