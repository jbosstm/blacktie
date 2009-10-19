package org.jboss.blacktie.jatmibroker.xatmi;

import java.io.Serializable;
import java.util.Properties;

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
	private long flags;

	/**
	 * The connection descriptor
	 */
	private Session session;

	private Properties properties;

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
	 */
	TPSVCINFO(String name, Buffer buffer, long flags, Session session,
			Properties properties) {
		this.name = name;
		this.buffer = buffer;
		this.flags = flags;
		this.session = session;
		this.properties = properties;
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
		if (type == null) {
			throw new ConnectionException(Connection.TPEINVAL,
					"No type provided");
		} else if (type.equals("X_OCTET")) {
			log.debug("Initializing a new X_OCTET");
			return new X_OCTET();
		} else if (type.equals("X_C_TYPE")) {
			log.debug("Initializing a new X_C_TYPE");
			return new X_C_TYPE(subtype, properties);
		} else {
			log.debug("Initializing a new X_COMMON");
			return new X_COMMON(subtype, properties);
		}
	}

}
