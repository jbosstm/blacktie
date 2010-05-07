package org.jboss.blacktie.jatmibroker.xatmi;

import java.io.Serializable;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

/**
 * This is the inbound service data struct
 */
public class TPSVCINFO implements Serializable {
	/**
	 * The logger to use.
	 */
	private static final Logger log = LogManager.getLogger(TPSVCINFO.class);

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
	private int flags;

	/**
	 * The connection descriptor
	 */
	private Session session;

	private Connection connection;

	private int len;

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
	 * @param session
	 *            The connection descriptor used
	 * @param properties
	 *            The properties to use
	 * @param len
	 */
	TPSVCINFO(String name, Buffer buffer, int flags, Session session,
			Connection connection, int len) {
		this.name = name;
		this.buffer = buffer;
		this.flags = flags;
		this.session = session;
		this.connection = connection;
		this.len = len;
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
	 * Get the length of the buffer that was sent
	 * 
	 * @return The length of the buffer
	 */
	public int getLen() {
		return len;
	}

	/**
	 * Get the flags that were issued
	 * 
	 * @return The flags
	 */
	public int getFlags() {
		return flags;
	}

	/**
	 * Get the connection descriptor
	 * 
	 * @return The connection descriptor
	 * @throws ConnectionException
	 */
	public Session getSession() throws ConnectionException {
		if (session == null) {
			throw new ConnectionException(Connection.TPEPROTO,
					"Not a TPCONV session");
		}
		return session;
	}

	/**
	 * Allocate a new buffer
	 * 
	 * @param type
	 *            The type of the buffer
	 * @param subtype
	 *            The subtype of the buffer
	 * @return The new buffer
	 * @throws ConnectionException
	 *             If the buffer cannot be created or the subtype located
	 */
	public Buffer tpalloc(String type, String subtype)
			throws ConnectionException {
		return connection.tpalloc(type, subtype);
	}

}
