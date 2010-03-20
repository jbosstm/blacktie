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

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.Iterator;
import java.util.List;
import java.util.Properties;

import javax.management.MBeanServerConnection;
import javax.management.ObjectName;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.btadmin.Command;
import org.jboss.blacktie.btadmin.CommandFailedException;
import org.jboss.blacktie.btadmin.IncompatibleArgsException;
import org.jboss.blacktie.jatmibroker.core.conf.Machine;
import org.jboss.blacktie.jatmibroker.core.conf.Server;

/**
 * The shutdown command will shutdown the server specified
 */
public class Startup implements Command {
	/**
	 * The logger to use for output
	 */
	private static Logger log = LogManager.getLogger(Startup.class);

	/**
	 * The name of the server.
	 */
	private String serverName;

	/**
	 * Does the command require the admin connection.
	 */
	public boolean requiresAdminConnection() {
		return false;
	}

	/**
	 * Show the usage of the command
	 */
	public String getExampleUsage() {
		return "[<serverName>]";
	}

	public void initializeArgs(String[] args) throws IncompatibleArgsException {
		if (args.length == 1) {
			serverName = args[0];
		}
	}

	public void invoke(MBeanServerConnection beanServerConnection,
			ObjectName blacktieAdmin, Properties configuration)
			throws CommandFailedException, IOException {
		List<Server> serverLaunchers = (List<Server>) configuration
				.get("blacktie.domain.serverLaunchers");
		boolean found = false;
		Iterator<Server> iterator = serverLaunchers.iterator();
		while (iterator.hasNext()) {
			Server next = iterator.next();
			if (serverName == null || serverName.equals(next.getName())) {
				if (serverName != null) {
					log.debug("Listing machines for: " + serverName);
				} else {
					log.debug("Listing machines");
				}
				List<Machine> localMachinesList = next.getLocalMachine();
				if (localMachinesList.size() != 0) {
					Iterator<Machine> localMachines = localMachinesList
							.iterator();
					while (localMachines.hasNext()) {
						log.debug("Found machine");
						Machine localMachine = localMachines.next();
						String pathToExecutable = localMachine
								.getPathToExecutable();
						String argLine = localMachine.getArgLine();
						String[] split = argLine.split(" ");
						String[] cmdarray = new String[split.length + 1 + 0];
						cmdarray[0] = pathToExecutable;
						System.arraycopy(split, 0, cmdarray, 1, split.length);
						String[] envp = null;
						File dir = new File(localMachine.getWorkingDirectory());
						Process exec = Runtime.getRuntime().exec(cmdarray,
								envp, dir);
						log.debug("Launched server: " + pathToExecutable);
						InputStream inputStream = exec.getInputStream();
						BufferedReader reader = new BufferedReader(
								new InputStreamReader(inputStream));
						while (true) {
							String readLine = reader.readLine();
							log.info(readLine);
							if (readLine
									.endsWith("Server waiting for requests...")) {
								found = true;
								break;

							} else if (readLine.endsWith("serverinit failed")) {
								throw new CommandFailedException(-2);
							}
						}
					}
				}
			}
		}
		if (!found) {
			log.error("No machines configured for host");
			throw new CommandFailedException(-1);
		}
	}
}
