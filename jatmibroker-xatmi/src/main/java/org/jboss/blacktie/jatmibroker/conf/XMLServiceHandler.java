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
 * XMLServiceHandler extends DefaultHandler to Client Info
 */
public class XMLServiceHandler extends DefaultHandler {
	private static final Logger log = LogManager
			.getLogger(XMLServiceHandler.class);

	private final String SERVICE_NAME = "SERVICE_DESCRIPTION";
	private final String SIZE = "SIZE";

	private String serviceElement;
	private String sizeElement;
	private String serviceName;
	private Properties prop;

	XMLServiceHandler(String serviceName) {
		prop = new Properties();
		this.serviceName = serviceName;
	}

	XMLServiceHandler(String serviceName, Properties prop) {
		this.prop = prop;
		this.serviceName = serviceName;
	}

	public void characters(char[] ch, int start, int length)
			throws SAXException {
		String strValue = new String(ch, start, length);

		if (SIZE.equals(sizeElement)) {
			String key = "blacktie." + serviceName + ".size";
			prop.setProperty(key, strValue);
		}
	}

	public void startElement(String namespaceURI, String localName,
			String qName, Attributes atts) throws SAXException {
		if (SERVICE_NAME.equals(localName)) {
			serviceElement = SERVICE_NAME;
			String key = "blacktie." + serviceName;
			String func_key = key + ".function_name";
			String lib_key = key + ".library_name";
			String ad_key = key + ".advertised";

			if(atts != null){
				for(int i = 0; i < atts.getLength(); i++){
					if(atts.getLocalName(i).equals("function_name")){
						String function_name = atts.getValue(i);
						prop.put(func_key, function_name);
					} else if(atts.getLocalName(i).equals("library_name")){
						String library_name = atts.getValue(i);
						prop.put(lib_key, library_name);
					} else if(atts.getLocalName(i).equals("advertised")) {
						String advertised = atts.getValue(i);
						prop.put(ad_key, advertised);
					}
				}
			}

			if(prop.get(func_key) == null) {
				prop.put(func_key, serviceName);
			}
		} else if(SIZE.equals(localName)) {
			sizeElement = SIZE;
		}
	}

	public void endElement(String namespaceURI, String localName, String qName)
			throws SAXException {
		if (SERVICE_NAME.equals(localName)) {
			serviceElement = "";
		} else if(SIZE.equals(localName)) {
			sizeElement = "";
		}
	}
}
