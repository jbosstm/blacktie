package org.jboss.blacktie.jatmibroker;

import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.blacktie.jatmibroker.xatmi.server.BlackTieServer;

public class BlackTieServerLauncher {

	/**
	 * @param args
	 * @throws ConnectionException
	 * @throws ConfigurationException
	 * @throws InterruptedException
	 */
	public static void main(String[] args) throws ConfigurationException,
			ConnectionException, InterruptedException {
		BlackTieServer server = new BlackTieServer("javaser");
		server.tpadvertise("JAVASERV", BarService.class.getName());
		server.block();
	}

}
