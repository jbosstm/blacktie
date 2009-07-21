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

import junit.framework.TestCase;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

public class XMLParserTest extends TestCase {
	private static final Logger log = LogManager.getLogger(XMLParserTest.class);

	public void setUp() throws InterruptedException {
	}

	public void tearDown() {
	}

	public void testWrongEnvironmentXML() throws ConfigurationException {
		Properties prop = new Properties();
		XMLEnvHandler handler = new XMLEnvHandler("", prop);
		XMLParser xmlenv = new XMLParser(handler, "Environment.xsd");
		try {
			if (xmlenv.parse("WrongEnvironment.xml")) {
				fail("Should have thrown a parser exception");
			} else {
				fail("Should have found the file");
			}
		} catch (ConfigurationException e) {
			// THIS IS OK
		}

	}

	public void testEnvironmentXML() throws Exception {
		Properties prop = new Properties();
		XMLEnvHandler handler = new XMLEnvHandler("", prop);
		XMLParser xmlenv = new XMLParser(handler, "Environment.xsd");
		xmlenv.parse("Environment.xml");

		String domain = "fooapp";
		String transid = "TransactionManagerService.OTS";
		String args = "2";
		String arg1 = "-ORBInitRef";
		String arg2 = "NameService=corbaloc::localhost:3528/NameService";
		String server = "standalone-server";
		String serviceName = "TestOne";
		String transport = "libatmibroker-stomp.so";
		// String library = "libBAR.so";
		// String function = "TestOne";
		String advertised = "true";
		String size = "1";

		assertTrue(server.equals(prop.getProperty("blacktie.TestOne.server")));
		assertTrue(transport.equals(prop
				.getProperty("blacktie.XMLParserTest.transportLib")));
		// assertTrue(function.equals(prop
		// .getProperty("blacktie.TestOne.function_name")));
		// assertTrue(library
		// .equals(prop.getProperty("blacktie.TestOne.library_name")));
		assertTrue(advertised.equals(prop
				.getProperty("blacktie.TestOne.advertised")));
		assertTrue(domain.equals(prop.getProperty("blacktie.domain.name")));
		assertTrue(transid.equals(prop.getProperty("blacktie.trans.factoryid")));
		assertTrue(args.equals(prop.getProperty("blacktie.orb.args")));
		assertTrue(arg1.equals(prop.getProperty("blacktie.orb.arg.1")));
		assertTrue(arg2.equals(prop.getProperty("blacktie.orb.arg.2")));
		assertTrue(size.equals(prop.getProperty("blacktie.TestOne.size")));
		assertTrue("log4cxx.properties".equals(prop
				.getProperty("LOG4CXXCONFIG")));
	}
	//
	// public void testServerXML() throws Exception {
	// Properties prop = new Properties();
	//
	// XMLServerHandler handler = new XMLServerHandler(prop);
	// XMLParser xmlserver = new XMLParser(handler, "Server.xsd");
	// xmlserver.parse(new File("linux/foo/SERVER.xml"));
	// }
	//
	// public void testClientXML() throws Exception {
	// Properties prop = new Properties();
	//
	// XMLClientHandler handler = new XMLClientHandler(prop);
	// XMLParser xmlclient = new XMLParser(handler, "Client.xsd");
	// xmlclient.parse(new File("linux/CLIENT.xml"));
	//
	// String server = "foo";
	// assertTrue(server.equals(prop.getProperty("blacktie.server.name")));
	// }
}
