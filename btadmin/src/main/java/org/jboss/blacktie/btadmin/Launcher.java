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

/**
 * Launcher for the btadmin tool.
 * 
 * @author tomjenkinson
 */
public class Launcher {
	private static Logger log = LogManager.getLogger(Launcher.class);

	public static void main(String[] args) {
		String url = "service:jmx:rmi:///jndi/rmi://localhost:1090/jmxconnector";
		String mbeanName = "jboss.blacktie:service=Admin";
		boolean commandSuccessful = false;
		try {
			// Initialize the connection to the mbean server
			JMXServiceURL u = new JMXServiceURL(url);
			JMXConnector c = JMXConnectorFactory.connect(u);
			MBeanServerConnection beanServerConnection = c
					.getMBeanServerConnection();
			ObjectName blacktieAdmin = new ObjectName(mbeanName);

			CommandHandler commandHandler = new CommandHandler(
					beanServerConnection, blacktieAdmin);
			commandSuccessful = commandHandler.handleCommand(args);
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

		// Check whether we need to exit the launcher
		if (commandSuccessful) {
			log.trace("Command was successfull");
		} else {
			log.trace("Command failed");
			System.exit(-1);
		}
	}
}
