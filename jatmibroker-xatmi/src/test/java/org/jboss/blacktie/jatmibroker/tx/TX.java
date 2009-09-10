package org.jboss.blacktie.jatmibroker.tx;

public class TX {
	public static final int TX_OK = -1;
	public static final int TX_ROLLBACK_ONLY = -1;
	public static final int TX_ROLLBACK = -1;

	public static int tx_begin() {
		return -1;
	}

	public static int tx_open() {
		return -1;
	}

	public static int tx_info(TXINFO txinfo) {
		txinfo.transaction_state = -1;
		return -1;
	}

	public static int tx_commit() {
		return -1;
	}
}
