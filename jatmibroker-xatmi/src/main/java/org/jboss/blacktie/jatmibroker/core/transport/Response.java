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
package org.jboss.blacktie.jatmibroker.core.transport;

public final class Response implements java.io.Serializable {
	private static final long serialVersionUID = 1L;
	private short rval;
	private int rcode;
	private byte[] bytes;
	private int length;
	private int flags;
	public int event;

	public Response(short rval, int rcode, byte[] bytes, int length, int flags) {
		this.rval = rval;
		this.rcode = rcode;
		this.bytes = bytes;
		this.length = length;
		this.flags = flags;
	}

	public void setLength(int aLength) {
		length = aLength;
	}

	public void setEvent(int aEvent) {
		event = aEvent;
	}

	public int getEvent() {
		return event;
	}

	public String toString() {
		return String.valueOf(bytes);
	}

	public short getRval() {
		return rval;
	}

	public int getRcode() {
		return rcode;
	}

	public byte[] getBytes() {
		return bytes;
	}

	public int getLength() {
		return length;
	}

	public int getFlags() {
		return flags;
	}
}
