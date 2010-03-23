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
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

import junit.framework.TestCase;

import org.jboss.blacktie.jatmibroker.jab.JABException;
import org.jboss.blacktie.jatmibroker.jab.JABRemoteService;
import org.jboss.blacktie.jatmibroker.jab.JABSession;
import org.jboss.blacktie.jatmibroker.jab.JABSessionAttributes;
import org.jboss.blacktie.jatmibroker.jab.JABTransaction;

public class JABClientTest extends TestCase {
	private static final Logger log = LogManager.getLogger(JABClientTest.class);

	private InputStreamReader isr = new InputStreamReader(System.in);
	private BufferedReader br = new BufferedReader(isr);

	public void test() throws Exception {
		log.info("JABClient");
		String message = prompt("Enter a message to send");
		JABSessionAttributes aJabSessionAttributes = new JABSessionAttributes();
		JABSession aJabSession = new JABSession(aJabSessionAttributes);
		JABTransaction transaction = new JABTransaction(aJabSession, 5000);
		JABRemoteService aJabService = new JABRemoteService("BAR", aJabSession,
				"X_OCTET", null);
		aJabService.getRequest().setByteArray("X_OCTET", message.getBytes());
		log.info("Calling call with input: " + message);
		aJabService.call(null);
		log
				.info("Called call with output: "
						+ new String(aJabService.getResponse().getByteArray(
								"X_OCTET")));
		transaction.commit();
		aJabSession.closeSession();
	}

	private String prompt(String prompt) throws IOException {
		System.out.println("Please press return after you: " + prompt + "...");
		return br.readLine();
	}
}
