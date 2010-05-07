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
package org.jboss.blacktie.btadmin;

import java.io.IOException;

import javax.management.MalformedObjectNameException;

import junit.framework.TestCase;

public class PauseDomainTest extends TestCase {

	private CommandHandler commandHandler;

	public void setUp() throws Exception {

		this.commandHandler = new CommandHandler();

		if (commandHandler.handleCommand("startup default".split(" ")) != 0) {
			fail("Could not start the server");
		}

		String command = "advertise default BAR";
		if (commandHandler.handleCommand(command.split(" ")) != 0) {
			fail("Command failed");
		}

		command = "resumeDomain";
		if (commandHandler.handleCommand(command.split(" ")) != 0) {
			fail("Command failed");
		}
	}

	public void tearDown() throws Exception {
		if (commandHandler.handleCommand("shutdown default".split(" ")) != 0) {
			fail("Could not stop the server");
		}

		// // TODO SHUTDOWN SHOULD RETURN WHEN THERE ARE NO MORE CONSUMERS?
		// try {
		// Thread.currentThread().sleep(5000);
		// } catch (InterruptedException e) {
		// // TODO Auto-generated catch block
		// e.printStackTrace();
		// }
	}

	public void testPauseDomain() throws IOException,
			MalformedObjectNameException, NullPointerException,
			InstantiationException, IllegalAccessException,
			ClassNotFoundException {
		String command = "pauseDomain";
		if (commandHandler.handleCommand(command.split(" ")) != 0) {
			fail("Command failed");
		}
	}

	public void testPausePauseDomain() throws IOException,
			MalformedObjectNameException, NullPointerException,
			InstantiationException, IllegalAccessException,
			ClassNotFoundException {
		String command = "pauseDomain";
		if (commandHandler.handleCommand(command.split(" ")) != 0) {
			fail("Command failed");
		}
		if (commandHandler.handleCommand(command.split(" ")) != 0) {
			fail("Command failed");
		}
	}

	public void testPauseDomainWithArg() throws IOException,
			MalformedObjectNameException, NullPointerException,
			InstantiationException, IllegalAccessException,
			ClassNotFoundException {
		String command = "pauseDomain fooapp";
		if (commandHandler.handleCommand(command.split(" ")) == 0) {
			fail("Command successful");
		}
	}
}
