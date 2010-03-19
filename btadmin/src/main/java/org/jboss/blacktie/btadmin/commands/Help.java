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

import java.util.Properties;

import javax.management.MBeanServerConnection;
import javax.management.ObjectName;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.btadmin.Command;
import org.jboss.blacktie.btadmin.CommandFailedException;
import org.jboss.blacktie.btadmin.CommandHandler;
import org.jboss.blacktie.btadmin.IncompatibleArgsException;

/**
 * The shutdown command will quit the terminal
 */
public class Help implements Command {
	/**
	 * The logger to use for output
	 */
	private static Logger log = LogManager.getLogger(Help.class);

	/**
	 * The command to get help for
	 */
	private String command;

	public boolean requiresAdminConnection() {
		return false;
	}

	public String getExampleUsage() {
		return "[command]";
	}

	public void initializeArgs(String[] args) throws IncompatibleArgsException {
		if (args.length > 0) {
			command = args[0];
		}
	}

	public void invoke(MBeanServerConnection beanServerConnection,
			ObjectName blacktieAdmin, Properties configuration)
			throws CommandFailedException {
		String[] commands = new String[] { "startup", "shutdown",
				"pauseDomain", "resumeDomain", "listRunningServers",
				"listRunningInstanceIds", "advertise", "unadvertise",
				"version", "help", "quit" };

		for (int i = 0; i < commands.length; i++) {
			if (command != null && !command.equals(commands[i])) {
				continue;
			}
			try {
				Command command = CommandHandler.loadCommand(commands[i]);
				log.info(commands[i] + " " + command.getExampleUsage());
			} catch (Exception e) {
				log.error("Could not get help for command: " + commands[i], e);
				throw new CommandFailedException(-1);
			}
		}
	}
}
