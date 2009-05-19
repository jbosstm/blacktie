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
package org.jboss.blacktie.jatmibroker.jab;

import junit.framework.TestCase;

import org.jboss.blacktie.jatmibroker.core.RunServer;

public class JABClientTestCase extends TestCase {
	private RunServer runServer = new RunServer();

	public void setUp() throws InterruptedException {
		runServer.serverinit();
	}

	public void tearDown() {
		runServer.serverdone();
	}

	public void testJABService() throws Exception {
		JABSessionAttributes aJabSessionAttributes = new JABSessionAttributes();
		JABSession aJabSession = new JABSession(aJabSessionAttributes);
		JABTransaction transaction = new JABTransaction(aJabSession, 5000);
		JABRemoteService aJabService = new JABRemoteService(aJabSession, "BAR");
		aJabService.setString("STRING", "HOWS IT GOING DUDE????!!!!");
		aJabService.call(null);
		transaction.commit();
		aJabSession.endSession();
		String expectedString = "BAR SAYS HELLO";
		String responseString = aJabService.getResponseString();
		assertEquals(expectedString, responseString);
	}

}
