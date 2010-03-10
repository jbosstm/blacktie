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
import java.net.MalformedURLException;

import javax.management.MBeanServerConnection;
import javax.management.MalformedObjectNameException;
import javax.management.ObjectName;
import javax.management.remote.JMXConnector;
import javax.management.remote.JMXConnectorFactory;
import javax.management.remote.JMXServiceURL;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.btadmin.commands.ListRunningServers;
import org.jboss.blacktie.btadmin.commands.Shutdown;

/**
 * Launcher for the btadmin tool.
 * 
 * @author tomjenkinson
 */
public class Launcher {
	private static Logger log = LogManager.getLogger(Launcher.class);

	public static void main(String[] args) {
		if (args.length < 1) {
			log.error("No command was provided");
		}
		String url = "service:jmx:rmi:///jndi/rmi://localhost:1090/jmxconnector";
		String mbeanName = "jboss.blacktie:service=Admin";
		try {
			// Initialize the connection to the mbean server
			JMXServiceURL u = new JMXServiceURL(url);
			JMXConnector c = JMXConnectorFactory.connect(u);
			MBeanServerConnection beanServerConnection = c
					.getMBeanServerConnection();
			ObjectName blacktieAdmin = new ObjectName(mbeanName);

			Command command = null;
			if (args[0].equals("shutdown")) {
				log.trace("Will execute the shutdown command");
				command = new Shutdown();
			} else if (args[0].equals("listRunningServers")) {
				log.trace("Will retrieve the list of running servers");
				command = new ListRunningServers();
			} else {
				log.error("Command was not known: " + args[0]);
			}
			if (command != null) {

				// Create an new array for the commands arguments
				String[] commandArgs = new String[args.length - 1];
				if (commandArgs.length > 0) {
					log.trace("Copying arguments for the command");
					System.arraycopy(args, 1, commandArgs, 0,
							commandArgs.length);
				}

				int expectedArgsLength = command.getExpectedArgsLength();
				if (commandArgs.length != expectedArgsLength) {
					log
							.error("Arguments incompatible, expected "
									+ expectedArgsLength + ", received: "
									+ args.length);
					log.error(("Expected Usage: " + args[0] + " " + command
							.getExampleUsage()).trim());
				} else {
					try {
						// Try to initialize the arguments
						command.initializeArgs(args);
						log.trace("Arguments initialized");
					} catch (IncompatibleArgsException e) {
						log.error("Arguments invalid: " + e.getMessage(), e);
					}
					try {
						// Try to invoke the command
						command.invoke(beanServerConnection, blacktieAdmin);
						log.trace("Command invoked");
					} catch (Exception e) {
						log.error("Could not invoke the command: "
								+ e.getMessage(), e);
					}
				}
			}
		} catch (MalformedURLException e) {
			log.error("JMX URL (" + url + ") was incorrect: " + e.getMessage(),
					e);
		} catch (IOException e) {
			log
					.error("Could not connect to mbean server: "
							+ e.getMessage(), e);
		} catch (MalformedObjectNameException e) {
			log.error("MBean name (" + mbeanName + ") was badly structured: "
					+ e.getMessage(), e);
		} catch (NullPointerException e) {
			log.error("MBean name (" + mbeanName + ") was raised an NPE: "
					+ e.getMessage(), e);
		}
	}
}
