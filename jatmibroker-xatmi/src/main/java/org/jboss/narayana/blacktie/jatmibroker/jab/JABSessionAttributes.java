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
package org.jboss.narayana.blacktie.jatmibroker.jab;

import java.util.Properties;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.narayana.blacktie.jatmibroker.core.conf.AtmiBrokerEnvXML;

/**
 * Create a wrapper for the attributes to use for the sessions. The session attributes are used in the constructor to the
 * JABSession.
 * 
 * @see JABSession
 */
public class JABSessionAttributes {
    /**
     * The logger to output information to
     */
    private static final Logger log = LogManager.getLogger(JABSessionAttributes.class);

    /**
     * The set of properties to use.
     */
    private Properties properties;

    /**
     * Create the session attributes using the default configuration directory
     * 
     * @throws JABException In case the configuration file cannot be accessed
     */
    public JABSessionAttributes() throws JABException {
        try {
            AtmiBrokerEnvXML client = new AtmiBrokerEnvXML();
            this.properties = client.getProperties();
        } catch (Exception e) {
            log.error("Could not load the configuration", e);
            throw new JABException("Could not load the configuration", e);
        }
    }

    /**
     * Get the properties defined in the configuration file
     * 
     * @return The configuration properties
     */
    public Properties getProperties() {
        return properties;
    }
}
