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
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Properties;

import javax.management.InstanceNotFoundException;
import javax.management.MBeanException;
import javax.management.MBeanServerConnection;
import javax.management.ObjectName;
import javax.management.ReflectionException;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.btadmin.Command;
import org.jboss.blacktie.btadmin.CommandFailedException;
import org.jboss.blacktie.btadmin.IncompatibleArgsException;
import org.jboss.blacktie.jatmibroker.core.conf.Server;

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
	 * The ID of the server, will be 0 (all) if not provided
	 */
	private int id = 0;

	/**
	 * Does the command require the admin connection.
	 */
	public boolean requiresAdminConnection() {
		return true;
	}

	/**
	 * Show the usage of the command
	 */
	public String getExampleUsage() {
		return "[<serverName> [<serverId>]]";
	}

	public void initializeArgs(String[] args) throws IncompatibleArgsException {
		if (args.length > 0) {
			serverName = args[0];
			if (args.length == 2) {
				try {
					id = Integer.parseInt(args[1]);
					log.trace("Successfully parsed: " + args[1]);
				} catch (NumberFormatException nfe) {
					throw new IncompatibleArgsException(
							"The third argument was expected to be the (integer) instance id to shutdown");
				}
			}
		}
	}

	public void invoke(MBeanServerConnection beanServerConnection,
			ObjectName blacktieAdmin, Properties configuration)
			throws InstanceNotFoundException, MBeanException,
			ReflectionException, IOException, CommandFailedException {
		List<String> serversToStop = new ArrayList<String>();
		if (serverName == null) {
			List<Server> serverLaunchers = (List<Server>) configuration
					.get("blacktie.domain.serverLaunchers");
			Iterator<Server> iterator = serverLaunchers.iterator();
			while (iterator.hasNext()) {
				Server next = iterator.next();
				if (next.getLocalMachine().size() > 0) {
					serversToStop.add(next.getName());
				}
			}
		} else {
			serversToStop.add(serverName);
		}
		Iterator<String> iterator = serversToStop.iterator();
		while (iterator.hasNext()) {
			String next = iterator.next();
			Boolean result = (Boolean) beanServerConnection.invoke(
					blacktieAdmin, "shutdown", new Object[] { next, id },
					new String[] { "java.lang.String", "int" });
			if (result) {
				log.info("Server shutdown successfully: " + next);
			} else {
				log
						.error("Server could not be shutdown (may already be stopped)");
				throw new CommandFailedException(-1);
			}
		}
	}
}
