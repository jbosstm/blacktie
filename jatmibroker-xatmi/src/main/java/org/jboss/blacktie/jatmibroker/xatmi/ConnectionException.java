package org.jboss.blacktie.jatmibroker.xatmi;

/**
 * This is the exception that is raised when the connection to Blacktie is
 * suffering.
 */
public class ConnectionException extends Exception {
	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	private int tperrno;
	private long event;
	private Buffer received;

	/**
	 * Create a new exception giving it the error code.
	 * 
	 * @param tperrno
	 *            The error code
	 */
	public ConnectionException(int tperrno, String string, Throwable t) {
		super(string, t);
		this.tperrno = tperrno;
	}

	/**
	 * Create an exception without a root cause
	 * 
	 * @param tperrno
	 *            The error number
	 * @param string
	 *            The message
	 */
	public ConnectionException(int tperrno, String string) {
		super(string);
		this.tperrno = tperrno;
	}

	/**
	 * An exception for reporting events
	 * 
	 * @param tperrno
	 *            This will always be TPEEVENT
	 * @param event
	 *            The event may be any from Connection
	 * @param lastRCode
	 *            The rcode in case of TPFAIL
	 * @param string
	 *            The message
	 * @param received
	 *            A received buffer
	 */
	public ConnectionException(int tperrno, long event, int lastRCode,
			String string, Buffer received) {
		super(string);
		this.tperrno = tperrno;
		this.event = event;
		this.received = received;
	}

	/**
	 * Get the error code
	 * 
	 * @return The error code
	 */
	public int getTperrno() {
		return tperrno;
	}

	/**
	 * Get the event
	 * 
	 * @return The event
	 */
	public long getEvent() {
		return event;
	}

	/**
	 * Get a received buffer
	 * 
	 * @return The received buffer
	 */
	public Buffer getReceived() {
		return received;
	}
}
