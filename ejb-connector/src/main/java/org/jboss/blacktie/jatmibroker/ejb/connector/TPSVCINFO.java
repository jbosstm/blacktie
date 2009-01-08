package org.jboss.blacktie.jatmibroker.ejb.connector;

import java.io.Serializable;

import org.jboss.blacktie.jatmibroker.ejb.connector.buffers.Buffer;

/**
 * This is the inbound service data struct
 */
public class TPSVCINFO implements Serializable {
	/**
	 * The service name
	 */
	private String name;

	/**
	 * The service data
	 */
	private Buffer data;

	/**
	 * The length of the data
	 */
	private long len;

	/**
	 * The flags the service was called with
	 */
	private long flags;

	/**
	 * The connection descriptor
	 */
	private int cd;

	/**
	 * Create a new tpsvcinfo wrapper class
	 * 
	 * @param name
	 *            The name of the service
	 * @param data
	 *            The data sent by the client
	 * @param len
	 *            The length of the said data
	 * @param flags
	 *            The flags that the client issued
	 * @param cd
	 *            The connection descriptor used
	 */
	public TPSVCINFO(String name, Buffer data, long flags, int cd) {
		this.name = name;
		this.data = data;
		this.len = data.getSize();
		this.flags = flags;
		this.cd = cd;
	}

	/**
	 * Get the services name
	 * 
	 * @return The name
	 */
	public String getName() {
		return name;
	}

	/**
	 * Get the data
	 * 
	 * @return The data
	 */
	public Buffer getData() {
		return data;
	}

	/**
	 * Get the length of the request
	 * 
	 * @return The length of the request
	 */
	public long getLen() {
		return len;
	}

	/**
	 * Get the flags that were issued
	 * 
	 * @return The flags
	 */
	public long getFlags() {
		return flags;
	}

	/**
	 * Get the connection descriptor
	 * 
	 * @return The connection descriptor
	 */
	public int getCd() {
		return cd;
	}
}
