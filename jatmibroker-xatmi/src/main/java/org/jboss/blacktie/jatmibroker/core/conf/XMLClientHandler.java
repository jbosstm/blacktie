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
package org.jboss.blacktie.jatmibroker.conf;

import java.util.Properties;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;

/**
 * XMLClientHandler extends DefaultHandler to Client Info
 */
public class XMLClientHandler extends DefaultHandler {
	private static final Logger log = LogManager
			.getLogger(XMLClientHandler.class);

	private final String SERVER_NAME = "SERVER_NAME";
	private final String MAX_REPLICAS = "MAX_REPLICAS";
	private final String MAX_CHANNELS = "MAX_CHANNELS";
	private final String MAX_SUPPLIERS = "MAX_SUPPLIERS";
	private final String MAX_CONSUMERS = "MAX_CONSUMERS";
	private final String SERVICE_NAME = "SERVICE_NAME";

	private String serverElement;
	private Properties prop;

	XMLClientHandler() {
		prop = new Properties();
	}

	XMLClientHandler(Properties prop) {
		this.prop = prop;
	}

	public void characters(char[] ch, int start, int length)
			throws SAXException {
		String strValue = new String(ch, start, length);

		if (SERVER_NAME.equals(serverElement)) {
			prop.setProperty("blacktie.server.name", strValue);
			log.debug("blacktie.server.name = " + strValue);
		}
	}

	public void startElement(String namespaceURI, String localName,
			String qName, Attributes atts) throws SAXException {
		if (SERVER_NAME.equals(localName)) {
			serverElement = SERVER_NAME;
		}
	}

	public void endElement(String namespaceURI, String localName, String qName)
			throws SAXException {
		if (SERVER_NAME.equals(localName)) {
			serverElement = "";
		}
	}
}
