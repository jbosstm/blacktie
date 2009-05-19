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
package org.jboss.blacktie.jatmibroker.core.conf;

import java.util.Properties;
import java.io.File;
import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

public class AtmiBrokerServerXML {
	private static final Logger log = LogManager.getLogger(AtmiBrokerServerXML.class);
	private Properties prop;

	public AtmiBrokerServerXML () {
		prop = new Properties();
	}

	public AtmiBrokerServerXML (Properties prop) {
		this.prop = prop;
	}

	public Properties getProperties() throws Exception {
		return getProperties(null);
	}

	public Properties getProperties(String configDir) throws Exception {
		String serverXML;
		String envXML;
		String serverName;

		serverName = System.getProperty("blacktie.server.name");
		if(serverName == null){
			serverName = "default";
		}

		prop.setProperty("blacktie.server.name", serverName);

		if(configDir == null) {
			configDir = System.getenv("BLACKTIE_CONFIGURATION_DIR");
		}

		if(configDir != null && !configDir.equals("")) {
			serverXML = configDir + "/" + serverName + "/" + "SERVER.xml";
			envXML    = configDir + "/" + "Environment.xml";
		} else {
			serverXML = serverName + "/" + "SERVER.xml";
			envXML    = "Environment.xml";
		}

		XMLServerHandler handler = new XMLServerHandler(prop);
		XMLParser xmlserver = new XMLParser(handler, "Server.xsd");
		xmlserver.parse(new File(serverXML));

		XMLEnvHandler env = new XMLEnvHandler(prop);
		XMLParser xmlenv = new XMLParser(env, "Environment.xsd");
		xmlenv.parse(new File(envXML));

		return prop;
	}
}
