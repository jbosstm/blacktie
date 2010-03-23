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

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

public class AtmiBrokerClientXML {
	private static final Logger log = LogManager
			.getLogger(AtmiBrokerClientXML.class);
	private Properties prop;

	public AtmiBrokerClientXML() {
		prop = new Properties();
	}

	public AtmiBrokerClientXML(Properties prop) {
		this.prop = prop;
	}

	public Properties getProperties() throws ConfigurationException {
		XMLEnvHandler env = new XMLEnvHandler(prop);
		XMLParser xmlenv = new XMLParser(env, "btconfig.xsd");
		xmlenv.parse("btconfig.xml");

		return prop;
	}
}
