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
	private final String ROLE = "role";

	private String sizeElement;
	private String serviceName;
	private Properties prop;
	private String serverName;

	XMLServiceHandler(String serverName, String serviceName, Properties prop) {
		this.prop = prop;
		this.serverName = serverName;
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
			String key = "blacktie." + serviceName;
			String func_key = key + ".function_name";
			String java_key = key + ".java_class_name";
			String lib_key = key + ".library_name";
			String ad_key = key + ".advertised";

			if (atts != null) {
				for (int i = 0; i < atts.getLength(); i++) {
					String attsLocalName = atts.getLocalName(i);
					if (attsLocalName.equals("function_name")) {
						String function_name = atts.getValue(i);
						prop.put(func_key, function_name);
					} else if (atts.getLocalName(i).equals("java_class_name")) {
						String java_class_name = atts.getValue(i);
						prop.put(java_key, java_class_name);
					} else if (atts.getLocalName(i).equals("library_name")) {
						String library_name = atts.getValue(i);
						prop.put(lib_key, library_name);
					} else if (atts.getLocalName(i).equals("advertised")) {
						String advertised = atts.getValue(i);
						prop.put(ad_key, advertised);
						String skey = "blacktie." + serverName + ".services";
						String object = (String) prop.get(skey);
						if (object == null) {
							object = serviceName;
						} else {
							object = new String(object + "," + serviceName);
						}
						prop.put(skey, object);
					}
				}
			}

			if (prop.get(func_key) == null) {
				prop.put(func_key, serviceName);
			}
		} else if (SIZE.equals(localName)) {
			sizeElement = SIZE;
		} else if (ROLE.equals(localName)) {
			String key = "blacktie." + serviceName + ".security";
			String roleList = prop.getProperty(key, "");
			String name = null;
			String read = "false";
			String write = "false";

			for (int i = 0; i < atts.getLength(); i++) {
				String attsLocalName = atts.getLocalName(i);
				if (attsLocalName.equals("name")) {
					name = atts.getValue(i);
				} else if (atts.getLocalName(i).equals("read")) {
					read = atts.getValue(i);
				} else {
					write = atts.getValue(i);
				}
			}
			String role = name + ':' + read + ':' + write;
			if (roleList.length() > 0) {
				roleList = roleList + ',' + role;
			} else {
				roleList = role;
			}
			prop.put(key, roleList);
		}
	}

	public void endElement(String namespaceURI, String localName, String qName)
			throws SAXException {
		if (SIZE.equals(localName)) {
			sizeElement = "";
		}
	}
}
