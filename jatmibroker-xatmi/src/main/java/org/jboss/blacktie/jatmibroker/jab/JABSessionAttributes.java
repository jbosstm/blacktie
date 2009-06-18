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
package org.jboss.blacktie.jatmibroker.jab;

import java.util.Properties;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.conf.AtmiBrokerClientXML;
import org.jboss.blacktie.jatmibroker.transport.OrbManagement;

/**
 * Create the session attributes
 */
public class JABSessionAttributes {
	private static final Logger log = LogManager
			.getLogger(JABSessionAttributes.class);
	private Properties properties;
	private OrbManagement orbManagement;

	/**
	 * Create session attributes using the default configuration from
	 * blacktie.config.dir
	 * 
	 * @throws JABException
	 */
	public JABSessionAttributes(String configurationDirectory)
			throws JABException {
		try {
			AtmiBrokerClientXML client = new AtmiBrokerClientXML();
			this.properties = client.getProperties(configurationDirectory);
		} catch (Exception e) {
			throw new JABException("Could not load the configuration", e);
		}
	}

	public JABSessionAttributes() throws JABException {
		this(null);
	}

	public Properties getProperties() {
		return properties;
	}
}
