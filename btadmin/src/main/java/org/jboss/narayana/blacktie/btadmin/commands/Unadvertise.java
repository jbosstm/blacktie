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
package org.jboss.narayana.blacktie.btadmin.commands;

import java.io.IOException;
import java.util.Properties;

import javax.management.InstanceNotFoundException;
import javax.management.MBeanException;
import javax.management.MBeanServerConnection;
import javax.management.ObjectName;
import javax.management.ReflectionException;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.narayana.blacktie.btadmin.Command;
import org.jboss.narayana.blacktie.btadmin.CommandFailedException;
import org.jboss.narayana.blacktie.btadmin.IncompatibleArgsException;

/**
 * The shutdown command will shutdown the server specified
 */
public class Unadvertise implements Command {
	/**
	 * The logger to use for output
	 */
	private static Logger log = LogManager.getLogger(Unadvertise.class);

	/**
	 * The name of the server.
	 */
	private String serverName;

	/**
	 * The name of the service.
	 */
	private String serviceName;

	/**
	 * Does the command require the admin connection.
	 */
	public boolean requiresAdminConnection() {
		return true;
	}

	/**
	 * Show the usage of the command
	 */
	public String getQuickstartUsage() {
		return "<serverName> <serviceName>";
	}

	public void initializeArgs(String[] args) throws IncompatibleArgsException {
		serverName = args[0];
		serviceName = args[1];
	}

	public void invoke(MBeanServerConnection beanServerConnection,
			ObjectName blacktieAdmin, Properties configuration)
			throws InstanceNotFoundException, MBeanException,
			ReflectionException, IOException, CommandFailedException {
		Boolean result = (Boolean) beanServerConnection.invoke(blacktieAdmin,
				"unadvertise", new Object[] { serverName, serviceName },
				new String[] { "java.lang.String", "java.lang.String" });
		if (result) {
			log.info("Service unadvertised");
		} else {
			log.error("Service could not be unadvertised");
			throw new CommandFailedException(-1);
		}
	}
}
