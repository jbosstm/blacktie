package org.jboss.blacktie.jatmibroker.xatmi;

import java.io.Serializable;

/**
 * This is the inbound service data struct
 */
public class TPSVCINFO implements Serializable {
	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;

	/**
	 * The service name
	 */
	private String name;

	/**
	 * The service data
	 */
	private Buffer buffer;

	/**
	 * The flags the service was called with
	 */
	private long flags;

	/**
	 * The connection descriptor
	 */
	private Session session;

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
	public TPSVCINFO(String name, Buffer buffer, long flags, Session session) {
		this.name = name;
		this.buffer = buffer;
		this.flags = flags;
		this.session = session;
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
	public Buffer getBuffer() {
		return buffer;
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
	public Session getSession() {
		return session;
	}
}
