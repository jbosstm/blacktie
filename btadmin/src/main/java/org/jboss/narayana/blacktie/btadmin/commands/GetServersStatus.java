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
import org.jboss.narayana.blacktie.btadmin.IncompatibleArgsException;
import org.w3c.dom.Document;
import org.w3c.dom.ls.DOMImplementationLS;
import org.w3c.dom.ls.LSSerializer;

/**
 * The command.
 */
public class GetServersStatus implements Command {
	/**
	 * The logger to use for output
	 */
	private static Logger log = LogManager.getLogger(GetServersStatus.class);

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

	/**
	 * This is a no-op for this command
	 */
	public void initializeArgs(String[] args) throws IncompatibleArgsException {
		// NO-OP as no arguments
	}

	/**
	 * List the running servers to console and log file
	 */
	public void invoke(MBeanServerConnection beanServerConnection,
			ObjectName blacktieAdmin, Properties configuration)
			throws InstanceNotFoundException, MBeanException,
			ReflectionException, IOException {
		org.w3c.dom.Element output = (org.w3c.dom.Element) beanServerConnection
				.invoke(blacktieAdmin, "getServersStatus", null, null);
		Document document = output.getOwnerDocument();
		DOMImplementationLS domImplLS = (DOMImplementationLS) document
				.getImplementation();
		LSSerializer serializer = domImplLS.createLSSerializer();
		String str = serializer.writeToString(output);
		log.info(str);
	}
}
