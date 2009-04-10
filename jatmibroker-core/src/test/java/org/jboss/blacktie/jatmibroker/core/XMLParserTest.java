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
package org.jboss.blacktie.jatmibroker.core;

import junit.framework.TestCase;
import java.io.File;
import java.util.Properties;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

import org.jboss.blacktie.jatmibroker.core.XMLParser;
import org.jboss.blacktie.jatmibroker.core.XMLEnvHandler;
import org.xml.sax.helpers.DefaultHandler;

public class XMLParserTest extends TestCase {
	private static final Logger log = LogManager.getLogger(XMLParserTest.class);

	public void setUp() throws InterruptedException {
	}

	public void tearDown() {
	}

	public void test() throws Exception {
		DefaultHandler handler = new DefaultHandler();
		XMLParser xmlserver = new XMLParser(handler, "Server.xsd");
		xmlserver.parse(new File("linux/SERVER.xml"));

		XMLParser xmlclient = new XMLParser(handler, "Client.xsd");
		xmlclient.parse(new File("linux/CLIENT.xml"));

		XMLEnvHandler envhandler = new XMLEnvHandler();
		XMLParser xmlenv = new XMLParser(envhandler, "Environment.xsd");
		xmlenv.parse(new File("linux/Environment.xml"));
		Properties prop = envhandler.getProperty();

		String domain = "fooapp";
		assertTrue(domain.equals(prop.getProperty("blacktie.domain.name")));

		//XMLParser xmlservice = new XMLParser(handler, "Service.xsd");
		//xmlclient.parse(new File("linux/Bar.xml"));
	}
}
