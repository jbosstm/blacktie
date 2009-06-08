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
package org.jboss.blacktie.example.ejb;

import java.rmi.RemoteException;
import java.util.Properties;

import javax.ejb.CreateException;
import javax.naming.Context;
import javax.naming.InitialContext;
import javax.naming.Name;
import javax.naming.NamingException;
import javax.rmi.PortableRemoteObject;

import junit.framework.TestCase;

import org.jboss.blacktie.jatmibroker.conf.AtmiBrokerClientXML;
import org.jboss.blacktie.jatmibroker.xatmi.Connector;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectorException;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectorFactory;
import org.jboss.blacktie.jatmibroker.xatmi.Response;

/**
 * This example shows how a Java client can connect to the remote server and
 * invoke a tpcall on it.
 */
public class BarServiceTestCase extends TestCase {
	public void testConnectorCall() throws ConnectorException, NamingException,
			RemoteException, CreateException {
		// Bootstrap the EJB
		Context ic = new InitialContext();
		Name realName = ic.getNameParser("BarService").parse("BarService");
		Object o = ic.lookup(realName);
		BarServiceHome home = (BarServiceHome) PortableRemoteObject.narrow(o,
				BarServiceHome.class);
		BarService service = home.create();

		// Initialise the connection
		AtmiBrokerClientXML xml = new AtmiBrokerClientXml();
		Properties properties = xml.getProperties();
		ConnectorFactory connectorFactory = ConnectorFactory
				.getConnectorFactory(properties);
		Connector connector = connectorFactory.getConnector("", "");

		// Make the call
		byte[] echo = "echo".getBytes();
		Buffer buffer = new X_OCTET(echo.length);
		buffer.setData(echo);
		Response response = connector.tpcall("BAR", buffer, 0);

		// Parse the result
		Buffer responseBuffer = response.getResponse();
		byte[] responseData = responseBuffer.getData();
		assertEquals("echo", new String(responseData));

	}
}
