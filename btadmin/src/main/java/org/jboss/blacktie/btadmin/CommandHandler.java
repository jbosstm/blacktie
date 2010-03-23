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
import java.util.Iterator;
import java.util.List;
import java.util.Properties;

import javax.management.MBeanServerConnection;
import javax.management.MalformedObjectNameException;
import javax.management.ObjectName;
import javax.management.remote.JMXConnector;
import javax.management.remote.JMXConnectorFactory;
import javax.management.remote.JMXServiceURL;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.core.conf.XMLEnvHandler;
import org.jboss.blacktie.jatmibroker.core.conf.XMLParser;

/**
 * Handle the command
 */
public class CommandHandler {
	private static Logger log = LogManager.getLogger(CommandHandler.class);
	private MBeanServerConnection beanServerConnection;
	private ObjectName blacktieAdmin;
	private Properties prop = new Properties();
	private String url;

	public CommandHandler() throws ConfigurationException,
			MalformedObjectNameException, NullPointerException {
		// Obtain the JMXURL from the Environment.xml
		XMLEnvHandler handler = new XMLEnvHandler(prop);
		XMLParser xmlenv;
		xmlenv = new XMLParser(handler, "Environment.xsd");
		xmlenv.parse("Environment.xml");
		url = (String) prop.get("JMXURL");
		if (url == null) {
			throw new ConfigurationException(
					"No JMX url configuration in Environment.xml");
		}
		this.blacktieAdmin = new ObjectName("jboss.blacktie:service=Admin");
	}

	public int handleCommand(String[] args) throws InstantiationException,
			IllegalAccessException, ClassNotFoundException,
			NullPointerException, IOException {
		int exitStatus = -1;
		if (args.length < 1 || args[0] == null || args[0].trim().length() == 0) {
			log.error("No command was provided");
		} else {
			Command command = loadCommand(args[0]);

			// Create an new array for the commands arguments
			String[] commandArgs = new String[args.length - 1];
			if (commandArgs.length > 0) {
				log.trace("Copying arguments for the command");
				System.arraycopy(args, 1, commandArgs, 0, commandArgs.length);
			}

			String exampleUsage = command.getExampleUsage();
			char[] charArray = exampleUsage.toCharArray();
			int expectedArgsLength = 0;
			int optionalArgs = 0;
			// Note this does assume that each word is a parameter
			for (int i = 0; i < exampleUsage.length(); i++) {
				if (charArray[i] == ' ') {
					expectedArgsLength++;
				} else if (charArray[i] == '[') {
					optionalArgs++;
				}
			}
			// Add the last parameter
			if (charArray.length > 0) {
				expectedArgsLength++;
			}
			// Check if the number of parameters is in an expected range
			if (commandArgs.length > expectedArgsLength
					|| commandArgs.length < expectedArgsLength - optionalArgs) {
				if (optionalArgs == 0) {
					log.trace("Arguments incompatible, expected "
							+ expectedArgsLength + ", received: "
							+ commandArgs.length);
				} else {
					log.trace("Arguments incompatible, expected at least "
							+ optionalArgs + " and no more than "
							+ expectedArgsLength + ", received: "
							+ commandArgs.length);
				}
				log.error(("Expected Usage: " + args[0] + " " + exampleUsage)
						.trim());
			} else {
				if (command.requiresAdminConnection()) {
					initializeAdminConnection();
				}
				try {
					// Try to initialize the arguments
					command.initializeArgs(commandArgs);
					log.trace("Arguments initialized");
					try {
						// Try to invoke the command
						command.invoke(beanServerConnection, blacktieAdmin,
								prop);
						exitStatus = 0;
						log.trace("Command invoked");
					} catch (CommandFailedException e) {
						exitStatus = e.getExitCode();
					} catch (Exception e) {
						log.error("Could not invoke the command: "
								+ e.getMessage(), e);
					}
				} catch (IncompatibleArgsException e) {
					String usage = "Expected Usage: " + args[0] + " "
							+ exampleUsage;
					log.error("Arguments invalid: " + e.getMessage());
					log.error(usage.trim());
					log.trace("Arguments invalid: " + e.getMessage(), e);
				}
			}
		}
		return exitStatus;
	}

	public static Command loadCommand(String commandName)
			throws InstantiationException, IllegalAccessException,
			ClassNotFoundException {
		String firstLetter = commandName.substring(0, 1);
		String remainder = commandName.substring(1);
		String capitalized = firstLetter.toUpperCase() + remainder;
		String className = "org.jboss.blacktie.btadmin.commands." + capitalized;
		log.trace("Will execute the " + className + " command");
		Command command = (Command) Class.forName(className).newInstance();
		log.debug("Command was known");
		return command;
	}

	public void initializeAdminConnection() throws IOException {
		if (beanServerConnection == null) {
			// Initialize the connection to the mbean server
			JMXServiceURL u = new JMXServiceURL(url);
			JMXConnector c = JMXConnectorFactory.connect(u);
			this.beanServerConnection = c.getMBeanServerConnection();
		}
	}

	/**
	 * Utility function to output the list
	 * 
	 * @param operationName
	 * @param list
	 */
	public static String convertList(String operationName, List list) {
		StringBuffer buffer = new StringBuffer();
		buffer.append("Output from: " + operationName);
		int i = 0;
		Iterator iterator = list.iterator();
		while (iterator.hasNext()) {
			buffer.append("\nElement: " + i + " Value: " + iterator.next());
			i++;
		}
		return buffer.toString();
	}
}
