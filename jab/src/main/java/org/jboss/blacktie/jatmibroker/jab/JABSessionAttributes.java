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

public class JABSessionAttributes {
	private static final Logger log = LogManager.getLogger(JABSessionAttributes.class);
	private String transactionManagerName;
	private Properties properties = new Properties();

	public JABSessionAttributes(String aCompanyName, String aServerName, String transactionManagerName, String[] args) throws JABException {
		log.debug("JABSessionAttributes constructor ");
		properties.put("blacktie.domain.name", aCompanyName);
		properties.put("blacktie.server.name", aServerName);
		this.transactionManagerName = transactionManagerName;
		properties.put("blacktie.orb.args", String.valueOf(args.length));
		for (int i = 1; i <= args.length; i++) {
			properties.put("blacktie.orb.arg." + i, args[i - 1]);
		}
	}

	public String getTransactionManagerName() {
		return transactionManagerName;
	}

	public Properties getProperties() {
		return properties;
	}

	public String getDomainName() {
		return (String) properties.get("blacktie.domain.name");
	}
}
