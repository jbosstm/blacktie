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
package org.jboss.blacktie.btadmin.commands;

import java.io.IOException;

import javax.management.InstanceNotFoundException;
import javax.management.MBeanException;
import javax.management.MBeanServerConnection;
import javax.management.ObjectName;
import javax.management.ReflectionException;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.btadmin.Command;
import org.jboss.blacktie.btadmin.IncompatibleArgsException;

/**
 * The shutdown command will shutdown the server specified
 */
public class Shutdown implements Command {
	/**
	 * The logger to use for output
	 */
	private static Logger log = LogManager.getLogger(Shutdown.class);

	/**
	 * The name of the server.
	 */
	private String serverName;

	/**
	 * The ID of the server
	 */
	private int id;

	/**
	 * Show the usage of the command
	 */
	public String getExampleUsage() {
		return "<serverName> <serverId>";
	}

	public void initializeArgs(String[] args) throws IncompatibleArgsException {
		serverName = args[0];
		try {
			id = Integer.parseInt(args[1]);
			log.trace("Successfully parsed: " + args[1]);
		} catch (NumberFormatException nfe) {
			throw new IncompatibleArgsException(
					"The third argument was expected to be the (integer) instance id to shutdown");
		}
	}

	public boolean invoke(MBeanServerConnection beanServerConnection,
			ObjectName blacktieAdmin) throws InstanceNotFoundException,
			MBeanException, ReflectionException, IOException {
		Boolean result = (Boolean) beanServerConnection.invoke(blacktieAdmin,
				"shutdown", new Object[] { serverName, id }, new String[] {
						"java.lang.String", "int" });
		if (result) {
			log.info("Server shutdown successfully");
		} else {
			log.error("Server could not be shutdown (may already be stopped)");
		}
		return result;
	}
}
