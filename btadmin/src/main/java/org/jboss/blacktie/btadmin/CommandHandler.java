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

import java.util.Iterator;
import java.util.List;

import javax.management.MBeanServerConnection;
import javax.management.ObjectName;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.btadmin.commands.ListRunningServers;
import org.jboss.blacktie.btadmin.commands.Shutdown;

/**
 * Handle the command
 */
public class CommandHandler {
	private static Logger log = LogManager.getLogger(CommandHandler.class);
	private MBeanServerConnection beanServerConnection;
	private ObjectName blacktieAdmin;

	public CommandHandler(MBeanServerConnection beanServerConnection,
			ObjectName blacktieAdmin) {
		this.beanServerConnection = beanServerConnection;
		this.blacktieAdmin = blacktieAdmin;
	}

	public boolean handleCommand(String[] args) {
		boolean commandSuccessful = false;
		if (args.length < 1) {
			log.error("No command was provided");
		} else {
			String commandName = args[0];
			Command command = null;
			if (commandName.equals("shutdown")) {
				log.trace("Will execute the shutdown command");
				command = new Shutdown();
			} else if (commandName.equals("listRunningServers")) {
				log.trace("Will retrieve the list of running servers");
				command = new ListRunningServers();
			} else {
				log.error("Command was not known: " + commandName);
			}
			if (command != null) {
				// Create an new array for the commands arguments
				String[] commandArgs = new String[args.length - 1];
				if (commandArgs.length > 0) {
					log.trace("Copying arguments for the command");
					System.arraycopy(args, 1, commandArgs, 0,
							commandArgs.length);
				}

				String exampleUsage = command.getExampleUsage();
				char[] charArray = exampleUsage.toCharArray();
				int expectedArgsLength = 0;
				int optionalArgs = 0;
				for (int i = 0; i < exampleUsage.length(); i++) {
					if (charArray[i] == ' ') {
						expectedArgsLength++;
					} else if (charArray[i] == '[') {
						optionalArgs++;
					}
				}
				if (charArray.length > 0) {
					expectedArgsLength++;
				}
				if (commandArgs.length != expectedArgsLength) {
					log.trace("Arguments incompatible, expected "
							+ expectedArgsLength + ", received: "
							+ commandArgs.length);
					log
							.error(("Expected Usage: " + commandName + " " + exampleUsage)
									.trim());
				} else {
					try {
						// Try to initialize the arguments
						command.initializeArgs(commandArgs);
						log.trace("Arguments initialized");
						try {
							// Try to invoke the command
							command.invoke(beanServerConnection, blacktieAdmin);
							commandSuccessful = true;
							log.trace("Command invoked");
						} catch (Exception e) {
							log.error("Could not invoke the command: "
									+ e.getMessage(), e);
						}
					} catch (IncompatibleArgsException e) {
						log.error("Arguments invalid: " + e.getMessage());
						log.trace("Arguments invalid: " + e.getMessage(), e);
					}
				}
			}
		}
		return commandSuccessful;
	}

	/**
	 * Utility function to output the list
	 * 
	 * @param operationName
	 * @param list
	 */
	public static void output(String operationName, List list) {
		StringBuffer buffer = new StringBuffer();
		buffer.append("Output from: " + operationName + "\n");
		int i = 0;
		Iterator iterator = list.iterator();
		while (iterator.hasNext()) {
			buffer
					.append("Element: " + i + " Value: " + iterator.next()
							+ "\n");
			i++;
		}
		log.info(buffer);
	}
}
