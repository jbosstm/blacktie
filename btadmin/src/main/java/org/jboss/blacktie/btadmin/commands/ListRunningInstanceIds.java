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
import java.util.List;

import javax.management.InstanceNotFoundException;
import javax.management.MBeanException;
import javax.management.MBeanServerConnection;
import javax.management.ObjectName;
import javax.management.ReflectionException;

import org.jboss.blacktie.btadmin.Command;
import org.jboss.blacktie.btadmin.CommandHandler;
import org.jboss.blacktie.btadmin.IncompatibleArgsException;

/**
 * List the running instance ids of a server.
 */
public class ListRunningInstanceIds implements Command {

	/**
	 * The server name.
	 */
	private String serverName;

	/**
	 * Get the usage of the command.
	 */
	public String getExampleUsage() {
		return "<serverName>";
	}

	/**
	 * Get the number of arguments
	 */
	public int getExpectedArgsLength() {
		return 1;
	}

	/**
	 * Initialize the arguments for the command
	 */
	public void initializeArgs(String[] args) throws IncompatibleArgsException {
		serverName = args[0];
	}

	public boolean invoke(MBeanServerConnection beanServerConnection,
			ObjectName blacktieAdmin) throws InstanceNotFoundException,
			MBeanException, ReflectionException, IOException {
		List<Integer> ids = (List<Integer>) beanServerConnection.invoke(
				blacktieAdmin, "listRunningInstanceIds",
				new Object[] { serverName },
				new String[] { "java.lang.String" });
		CommandHandler.output("listRunningInstanceIds", ids);
		return true;
	}

}
