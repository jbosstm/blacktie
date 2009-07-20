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
package org.jboss.blacktie.jatmibroker.conf;

import java.io.File;
import java.util.Properties;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

public class AtmiBrokerServiceXML {
	private static final Logger log = LogManager
			.getLogger(AtmiBrokerServiceXML.class);
	private Properties prop;
	private String serverName;
	private String serviceName;

	public AtmiBrokerServiceXML(String serverName, String serviceName) {
		prop = new Properties();
		this.serverName = serverName;
		this.serviceName = serviceName;
	}

	public AtmiBrokerServiceXML(String serverName, String serviceName,
			Properties prop) {
		this.prop = prop;
		this.serverName = serverName;
		this.serviceName = serviceName;
	}

	public Properties getProperties() throws ConfigurationException {
		return getProperties(null);
	}

	public Properties getProperties(String configDir)
			throws ConfigurationException {
		String serviceXML;

		if (configDir == null) {
			configDir = System.getenv("BLACKTIE_CONFIGURATION_DIR");
		}

		if (configDir != null && !configDir.equals("")) {
			serviceXML = configDir + "/" + serverName + "/" + serviceName
					+ ".xml";
		} else {
			serviceXML = serverName + "/" + serviceName + ".xml";
		}

		log.debug("read configuration from " + configDir + " directory");

		XMLServiceHandler handler = new XMLServiceHandler(serverName,
				serviceName, prop);
		XMLParser xmlservice = new XMLParser(handler, "Service.xsd");
		xmlservice.parse(serviceXML);

		return prop;
	}
}
