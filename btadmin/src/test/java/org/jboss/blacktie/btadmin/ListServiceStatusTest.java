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

public class ListServiceStatusTest extends TestCase {

	private CommandHandler commandHandler;

	public void setUp() throws Exception {
		this.commandHandler = new CommandHandler();
	}

	public void tearDown() throws Exception {
	}

	public void testListServiceStatusWithoutServerName() throws IOException,
			MalformedObjectNameException, NullPointerException,
			InstantiationException, IllegalAccessException,
			ClassNotFoundException {
		String command = "listServiceStatus";
		if (commandHandler.handleCommand(command.split(" ")) == 0) {
			fail("Command was successful");
		}
	}

	public void testListServiceStatusWithoutServiceName() throws IOException,
			MalformedObjectNameException, NullPointerException,
			InstantiationException, IllegalAccessException,
			ClassNotFoundException {
		String command = "listServiceStatus default";
		if (commandHandler.handleCommand(command.split(" ")) == 0) {
			fail("Command was successful");
		}
	}

	public void testListServiceStatusWithAdditionalParameters()
			throws IOException, MalformedObjectNameException,
			NullPointerException, InstantiationException,
			IllegalAccessException, ClassNotFoundException {
		String command = "listServiceStatus default 1 BAR";
		if (commandHandler.handleCommand(command.split(" ")) == 0) {
			fail("Command was successful");
		}
	}

	public void testListServiceStatusWithNonRunningServer() throws IOException,
			MalformedObjectNameException, NullPointerException,
			InstantiationException, IllegalAccessException,
			ClassNotFoundException {
		String command = "listServiceStatus foo BAR";
		if (commandHandler.handleCommand(command.split(" ")) == 0) {
			fail("Command was not successful");
		}
	}

	public void testListServiceStatusWithRunningServer() throws IOException,
			MalformedObjectNameException, NullPointerException,
			InstantiationException, IllegalAccessException,
			ClassNotFoundException {
		if (commandHandler.handleCommand("startup default".split(" ")) != 0) {
			fail("Could not start the server");
		}
		String command = "advertise default BAR";
		if (commandHandler.handleCommand(command.split(" ")) != 0) {
			fail("Command failed");
		}
		command = "listServiceStatus default BAR";
		if (commandHandler.handleCommand(command.split(" ")) != 0) {
			fail("Command was not successful");
		}

		if (commandHandler.handleCommand("shutdown default".split(" ")) != 0) {
			fail("Could not stop the server");
		}
		// TODO SHUTDOWN SHOULD RETURN WHEN THERE ARE NO MORE CONSUMERS?
		try {
			Thread.currentThread().sleep(5000);
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
}
