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

	public ConnectionException(int tpeproto, String string) {
		super(string);
	}

	/**
	 * Get the error code
	 * 
	 * @return The error code
	 */
	public int getTperrno() {
		return tperrno;
	}
}
