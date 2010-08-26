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
package org.jboss.blacktie.jatmibroker.core.transport;

import java.util.Properties;

import javax.jms.JMSException;
import javax.naming.NamingException;

import junit.framework.TestCase;

import org.jboss.blacktie.jatmibroker.core.conf.AtmiBrokerEnvXML;
import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;

public class ConnectionTest extends TestCase {

	public void testJMSManagementNoCredentials() throws ConfigurationException,
			NamingException, JMSException {
		AtmiBrokerEnvXML xml = new AtmiBrokerEnvXML();
		Properties properties = xml.getProperties();
		properties.remove("StompConnectUsr");
		properties.remove("StompConnectPwd");
		JMSManagement management = new JMSManagement(properties);
		management.close();
	}

	public void testJMSManagementCredentials() throws ConfigurationException,
			NamingException, JMSException {
		AtmiBrokerEnvXML xml = new AtmiBrokerEnvXML();
		Properties properties = xml.getProperties();
		JMSManagement management = new JMSManagement(properties);
		management.close();
	}

	public void testDoubleCloseFactory() throws ConfigurationException {
		AtmiBrokerEnvXML xml = new AtmiBrokerEnvXML();
		Properties properties = xml.getProperties();

		TransportFactory transportFactory = TransportFactory
				.getTransportFactory(properties);
		transportFactory.close();
		transportFactory.close();
	}

	public void testDoubleCloseTransport() throws ConfigurationException,
			ConnectionException {
		AtmiBrokerEnvXML xml = new AtmiBrokerEnvXML();
		Properties properties = xml.getProperties();

		TransportFactory transportFactory = TransportFactory
				.getTransportFactory(properties);
		Transport createTransport = transportFactory.createTransport();
		createTransport.close();
		createTransport.close();
	}

	public void testCloseFactoryWithTransport() throws ConfigurationException,
			ConnectionException {
		AtmiBrokerEnvXML xml = new AtmiBrokerEnvXML();
		Properties properties = xml.getProperties();

		TransportFactory transportFactory = TransportFactory
				.getTransportFactory(properties);
		transportFactory.createTransport();
		transportFactory.close();
	}

}
