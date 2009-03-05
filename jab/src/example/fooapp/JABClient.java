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
import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.jab.JABRemoteService;
import org.jboss.blacktie.jatmibroker.jab.JABSession;
import org.jboss.blacktie.jatmibroker.jab.JABSessionAttributes;
import org.jboss.blacktie.jatmibroker.jab.JABTransaction;

public class JABClient {
	private static final Logger log = LogManager.getLogger(JABClient.class);

	public static void main(String[] args) throws Exception {
		if (args.length != 6) {
			log.error("fooapp foo TransactionManagerService.OTS -ORBInitRef NameService=corbaloc::localhost:3528/NameService message");
			return;
		}
		String domainName = args[0];
		String serverName = args[1];
		String transactionManagerService = args[2];
		String[] orbArgs = new String[3];
		orbArgs[0] = serverName;
		orbArgs[1] = args[3];
		orbArgs[2] = args[4];
		String message = args[5];
		JABSessionAttributes aJabSessionAttributes = new JABSessionAttributes(domainName, serverName, transactionManagerService, orbArgs);
		JABSession aJabSession = new JABSession(aJabSessionAttributes);
		JABTransaction transaction = new JABTransaction(aJabSession, 5000);
		JABRemoteService aJabService = new JABRemoteService(aJabSession, "BAR");
		aJabService.setString("STRING", message);
		log.info("Calling call with input: " + message);
		aJabService.call(transaction);
		log.info("Called call with output: " + aJabService.getResponseString());
		transaction.commit();
		aJabSession.endSession();
	}
}
