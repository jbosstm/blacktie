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

public class ShutdownTest extends TestCase {
	private CommandHandler commandHandler;

	public void setUp() throws Exception {
		this.commandHandler = new CommandHandler();
	}

	public void testShutdownWithoutArgs() throws IOException,
			MalformedObjectNameException, NullPointerException {
		String command = "shutdown";
		if (commandHandler.handleCommand(command.split(" "))) {
			fail("Command was successful");
		}
	}

	public void testShutdownWithoutId() throws IOException,
			MalformedObjectNameException, NullPointerException {
		String command = "shutdown default";
		if (commandHandler.handleCommand(command.split(" "))) {
			fail("Command was successful");
		}
	}

	public void testShutdownWithNonIntId() throws IOException,
			MalformedObjectNameException, NullPointerException {
		String command = "shutdown default one";
		if (commandHandler.handleCommand(command.split(" "))) {
			fail("Command was successful");
		}
	}

	public void testShutdownWithoutServerName() throws IOException,
			MalformedObjectNameException, NullPointerException {
		String command = "shutdown 1";
		if (commandHandler.handleCommand(command.split(" "))) {
			fail("Command was successful");
		}
	}

	public void testShutdown() throws IOException,
			MalformedObjectNameException, NullPointerException {
		String command = "shutdown default 1";
		if (!commandHandler.handleCommand(command.split(" "))) {
			fail("Command was not successful");
		}
	}
}
