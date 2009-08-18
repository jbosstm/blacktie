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
 * XMLEnvHandler extends DefaultHandler to Environment Info
 */
public class XMLEnvHandler extends DefaultHandler {
	private static final Logger log = LogManager.getLogger(XMLEnvHandler.class);

	private final String DOMAIN = "DOMAIN";
	private final String ENV_VARIABLES = "ENV_VARIABLES";
	private final String ENV_VARIABLE = "ENV_VARIABLE";
	private final String SERVER_NAME = "SERVER";
	private final String SERVICE_NAME = "SERVICE";
	private final String NAME = "NAME";
	private final String VALUE = "VALUE";

	private String domainElement;
	private String serverElement;
	private String serviceElement;
	private String varsElement;
	private String varElement;
	private String nameElement;
	private String valueElement;
	private int orbargs;
	private Boolean isORBOPT;
	private Boolean isTrans;
	private Properties prop;

	private String value;
	private String name;

	private String serverName;
	private String configDir;

	public XMLEnvHandler(String configDir) {
		prop = new Properties();
		this.configDir = configDir;
	}

	public XMLEnvHandler(String configDir, Properties prop) {
		this.prop = prop;
		this.configDir = configDir;
	}

	public Properties getProperty() {
		return prop;
	}

	public void characters(char[] ch, int start, int length)
			throws SAXException {
		String strValue = new String(ch, start, length);
		value += strValue;
	}

	public void startElement(String namespaceURI, String localName,
			String qName, Attributes atts) throws SAXException {
		value = "";

		if (DOMAIN.equals(localName)) {
			domainElement = DOMAIN;
		} else if (ENV_VARIABLES.equals(localName)) {
			varsElement = ENV_VARIABLES;
			orbargs = 0;
		} else if (NAME.equals(localName)) {
			nameElement = NAME;
			isORBOPT = false;
			isTrans = false;
		} else if (VALUE.equals(localName)) {
			valueElement = VALUE;
		} else if (SERVER_NAME.equals(localName)) {
			serverElement = SERVER_NAME;
			if (atts != null) {
				for (int i = 0; i < atts.getLength(); i++) {
					if (atts.getLocalName(i).equals("name")) {
						serverName = atts.getValue(i);
					}
				}
			}
		} else if (SERVICE_NAME.equals(localName)) {
			serviceElement = SERVICE_NAME;
			String serviceName = null;

			if (atts != null) {
				for (int i = 0; i < atts.getLength(); i++) {
					if (atts.getLocalName(i).equals("name")) {
						serviceName = atts.getValue(i);
						if (serviceName.length() > 15) {
							log
									.warn("service "
											+ serviceName
											+ " is longer than XATMI_SERVICE_NAME_LENGTH and will be ignore");
							serviceName = null;
							break;
						}
						String skey = "blacktie." + serviceName + ".server";
						prop.put(skey, serverName);
					} else if (atts.getLocalName(i).equals("transportLibrary")) {
						String transport = atts.getValue(i);
						String key = "blacktie." + serviceName
								+ ".transportLib";
						prop.put(key, transport);
					}
				}
			}

			if (serviceName != null) {
				AtmiBrokerServiceXML xml = new AtmiBrokerServiceXML(serverName,
						serviceName, prop);
				try {
					xml.getProperties(configDir);
				} catch (ConfigurationException e) {
					throw new SAXException(e);
				}
			}
		}
	}

	public void endElement(String namespaceURI, String localName, String qName)
			throws SAXException {
		String jbossasIpAddr = System.getenv("JBOSSAS_IP_ADDR");
		if (DOMAIN.equals(localName)) {
			prop.setProperty("blacktie.domain.name", value);
			domainElement = "";
		} else if (ENV_VARIABLES.equals(localName)) {
			varsElement = "";
			log.debug("blacktie.orb.args is " + orbargs);
			prop.setProperty("blacktie.orb.args", Integer.toString(orbargs));
		} else if (NAME.equals(localName) && value.equals("ORBOPT")) {
			isORBOPT = true;
			nameElement = "";
		} else if (NAME.equals(localName) && value.equals("TRANS_FACTORY_ID")) {
			isTrans = true;
			nameElement = "";
		} else if (NAME.equals(localName)) {
			name = value;
			nameElement = "";
		} else if (VALUE.equals(localName) && isORBOPT) {
			String[] argv;
			argv = value.split(" ");
			orbargs = argv.length;

			for (int i = 1; i <= orbargs; i++) {
				String arg = "blacktie.orb.arg." + i;
				String toSet = argv[i - 1];
				if (jbossasIpAddr != null) {
					toSet = toSet.replace("${JBOSSAS_IP_ADDR}", jbossasIpAddr);
				}
				prop.setProperty(arg, toSet);
				log.debug(arg + " is " + argv[i - 1]);
			}
			isORBOPT = false;
			valueElement = "";
		} else if (VALUE.equals(localName) && isTrans) {
			prop.setProperty("blacktie.trans.factoryid", value);
			isTrans = false;
			valueElement = "";
		} else if (VALUE.equals(localName)) {
			valueElement = "";

			if (jbossasIpAddr != null) {
				value = value.replace("${JBOSSAS_IP_ADDR}", jbossasIpAddr);
			}
			prop.setProperty(name, value);
		} else if (SERVER_NAME.equals(localName)) {
			serverElement = "";
		} else if (SERVICE_NAME.equals(localName)) {
			serviceElement = "";
		}
	}
}
