package org.jboss.blacktie.administration;

import java.util.Properties;
import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.core.conf.XMLEnvHandler;
import org.jboss.blacktie.jatmibroker.core.conf.XMLParser;
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

public class BlacktieAdminService implements BlacktieAdminServiceMBean {
	private static final Logger log = LogManager.getLogger(BlacktieAdminService.class);
	private Properties prop = new Properties();

	public void start() throws Exception {
		XMLEnvHandler handler = new XMLEnvHandler("", prop);
		XMLParser xmlenv = new XMLParser(handler, "Environment.xsd");
		xmlenv.parse("Environment.xml");
	}

	public void stop() throws Exception {
	}

	public String getDomainName() {
		return prop.getProperty("blacktie.domain.name");
	}
}
