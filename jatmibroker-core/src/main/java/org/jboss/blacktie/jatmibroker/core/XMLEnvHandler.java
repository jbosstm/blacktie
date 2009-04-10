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

import java.text.NumberFormat;
import java.util.Properties;

import org.xml.sax.helpers.DefaultHandler;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

/**
 * XMLEnvHandler extends DefaultHandler to Environment Info
 */
public class XMLEnvHandler extends DefaultHandler {
	private final String DOMAIN = "DOMAIN";
	private final String ENV_VARIABLES = "ENV_VARIABLES";
	private final String ENV_VARIABLE  = "ENV_VARIABLE";
	private final String NAME = "NAME";
	private final String VALUE = "VALUE";

	private String domainElement;
	private Properties prop;

	public XMLEnvHandler() {
		prop = new Properties();
	}

	public XMLEnvHandler(Properties prop) {
		this.prop = prop;
	}

	public Properties getProperty() {
		return prop;
	}

  	public void characters(char[] ch, int start, int length) throws SAXException {
		String strValue = new String(ch, start, length);

		if(DOMAIN.equals(domainElement)) {
			prop.setProperty("blacktie.domain.name", strValue);
		}
	}

	public void startElement(String namespaceURI, String localName, String qName, Attributes atts) throws SAXException {
		if(DOMAIN.equals(localName)) {
			domainElement = DOMAIN;
		}
	}

  	public void endElement(String namespaceURI, String localName, String qName) throws SAXException {
		if(DOMAIN.equals(localName)) {
			domainElement = "";
		}
	}
}
