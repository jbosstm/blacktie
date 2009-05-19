package org.jboss.blacktie.jatmibroker.xatmi;

/**
 * This is the exception that is raised when the connection to Blacktie is
 * suffering.
 */
public class ConnectorException extends Exception {
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
	public ConnectorException(int tperrno, Throwable t) {
		this(tperrno, t.getMessage(), t);
	}

	public ConnectorException(int tperrno, String string, Throwable t) {
		super(string, t);
		setTperrno(tperrno);
	}

	/**
	 * Set the error code
	 * 
	 * @param tperrno
	 *            The error number
	 */
	private void setTperrno(int tperrno) {
		this.tperrno = tperrno;
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
