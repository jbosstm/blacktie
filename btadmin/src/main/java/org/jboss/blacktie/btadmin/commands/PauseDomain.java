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

/**
 * The command
 */
public class PauseDomain implements Command {
	/**
	 * The logger to use for output
	 */
	private static Logger log = LogManager.getLogger(PauseDomain.class);

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
		return "";
	}

	public void initializeArgs(String[] args) throws IncompatibleArgsException {
	}

	public void invoke(MBeanServerConnection beanServerConnection,
			ObjectName blacktieAdmin, Properties configuration)
			throws InstanceNotFoundException, MBeanException,
			ReflectionException, IOException, CommandFailedException {
		Boolean result = (Boolean) beanServerConnection.invoke(blacktieAdmin,
				"pauseDomain", new Object[] {}, new String[] {});
		if (result) {
			log.info("Domain paused");
		} else {
			log.error("Domain could not be paused");
			throw new CommandFailedException(-1);
		}
	}
}
