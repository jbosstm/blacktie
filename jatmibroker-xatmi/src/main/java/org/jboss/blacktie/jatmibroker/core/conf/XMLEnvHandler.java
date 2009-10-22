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

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Properties;
import java.util.Set;

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
	private final String BUFFER = "BUFFER";
	private final String ATTRIBUTE = "ATTRIBUTE";
	private final String SERVER_NAME = "SERVER";
	private final String SERVICE_NAME = "SERVICE";
	private final String ADMIN_SERVICE_NAME = "ADMIN_SERVICE";
	private final String NAME = "NAME";
	private final String VALUE = "VALUE";
	private final String ORB = "ORB";
	private final String MQ = "MQ";

	private Properties prop;

	private String value;
	private String name;

	private String serverName;
	private String configDir;

	private String jbossasIpAddr = System.getenv("JBOSSAS_IP_ADDR");

	private Set<String> servers = new HashSet<String>();

	private Map<String, BufferStructure> buffers = new HashMap<String, BufferStructure>();

	private String currentBufferName;

	static int CHAR_SIZE = 1;
	static int LONG_SIZE = 8;
	static int INT_SIZE = 4;
	static int SHORT_SIZE = 2;
	static int FLOAT_SIZE = 4;
	static int DOUBLE_SIZE = 8;

	public XMLEnvHandler(String configDir, Properties prop) {
		this.prop = prop;
		prop.put("blacktie.domain.servers", servers);
		prop.put("blacktie.domain.buffers", buffers);
		this.configDir = configDir;
	}

	public void characters(char[] ch, int start, int length)
			throws SAXException {
		String strValue = new String(ch, start, length);
		value += strValue;
	}

	public void startElement(String namespaceURI, String localName,
			String qName, Attributes atts) throws SAXException {
		value = "";

		if (SERVER_NAME.equals(localName)) {
			if (atts != null) {
				for (int i = 0; i < atts.getLength(); i++) {
					if (atts.getLocalName(i).equals("name")) {
						serverName = atts.getValue(i);
						servers.add(serverName);
					}
				}
			}
		} else if (BUFFER.equals(localName)) {
			currentBufferName = atts.getValue(0);
			BufferStructure buffer = buffers.get(currentBufferName);
			if (buffer == null) {
				buffer = new BufferStructure();
				buffer.name = currentBufferName;
				buffer.wireSize = 0;
				buffer.memSize = 0;
				buffer.lastPad = 0;
				buffers.put(currentBufferName, buffer);
			} else {
				log.error("Duplicate buffer detected: " + currentBufferName);
				currentBufferName = null;
			}

		} else if (ATTRIBUTE.equals(localName)) {
			if (currentBufferName != null) {
				BufferStructure buffer = buffers.get(currentBufferName);
				AttributeStructure attribute = new AttributeStructure();
				attribute.id = null;
				attribute.type = null;
				attribute.count = 0;
				attribute.length = 0;
				attribute.wirePosition = 0;
				attribute.memPosition = 0;
				String type = null;
				for (int i = 0; i < atts.getLength(); i++) {
					if (atts.getLocalName(i).equals("id")) {
						attribute.id = atts.getValue(i);
					} else if (atts.getLocalName(i).equals("type")) {
						type = atts.getValue(i);
					} else if (atts.getLocalName(i).equals("arrayCount")) {
						attribute.count = Integer.parseInt(atts.getValue(i));
					} else if (atts.getLocalName(i).equals("arrayLength")) {
						attribute.length = Integer.parseInt(atts.getValue(i));
					}
				}

				int typeSize = -1;
				boolean contains = buffer.attributeNames.contains(attribute.id);
				boolean fail = false;
				if (!contains) {
					// short, int, long, float, double, char
					if (type.equals("short")) {
						typeSize = SHORT_SIZE;
						attribute.instanceSize = SHORT_SIZE;
						attribute.type = short.class;
					} else if (type.equals("int")) {
						typeSize = INT_SIZE;
						attribute.instanceSize = INT_SIZE;
						attribute.type = int.class;
					} else if (type.equals("long")) {
						typeSize = LONG_SIZE;
						attribute.instanceSize = LONG_SIZE;
						attribute.type = long.class;
					} else if (type.equals("float")) {
						typeSize = FLOAT_SIZE;
						attribute.instanceSize = FLOAT_SIZE;
						attribute.type = float.class;
					} else if (type.equals("double")) {
						typeSize = DOUBLE_SIZE;
						attribute.instanceSize = DOUBLE_SIZE;
						attribute.type = double.class;
					} else if (type.equals("char")) {
						typeSize = CHAR_SIZE;
						attribute.instanceSize = CHAR_SIZE;
						attribute.type = char.class;
					} else if (type.equals("char[]")) {
						if (attribute.length == 0) {
							attribute.length = 1;
						}
						typeSize = CHAR_SIZE;
						attribute.instanceSize = CHAR_SIZE * attribute.length;
						attribute.type = char[].class;
					} else if (type.equals("short[]")) {
						if (attribute.length == 0) {
							attribute.length = 1;
						}
						typeSize = SHORT_SIZE;
						attribute.instanceSize = SHORT_SIZE * attribute.length;
						attribute.type = short[].class;
					} else if (type.equals("int[]")) {
						if (attribute.length == 0) {
							attribute.length = 1;
						}
						typeSize = INT_SIZE;
						attribute.instanceSize = INT_SIZE * attribute.length;
						attribute.type = int[].class;
					} else if (type.equals("long[]")) {
						if (attribute.length == 0) {
							attribute.length = 1;
						}
						typeSize = LONG_SIZE;
						attribute.instanceSize = LONG_SIZE * attribute.length;
						attribute.type = long[].class;
					} else if (type.equals("float[]")) {
						if (attribute.length == 0) {
							attribute.length = 1;
						}
						typeSize = FLOAT_SIZE;
						attribute.instanceSize = FLOAT_SIZE * attribute.length;
						attribute.type = float[].class;
					} else if (type.equals("double[]")) {
						if (attribute.length == 0) {
							attribute.length = 1;
						}
						typeSize = DOUBLE_SIZE;
						attribute.instanceSize = DOUBLE_SIZE * attribute.length;
						attribute.type = double[].class;
					} else if (type.equals("char[][]")) {
						if (attribute.length == 0) {
							attribute.length = 1;
						}
						if (attribute.count == 0) {
							attribute.count = 1;
						}
						typeSize = CHAR_SIZE;
						attribute.instanceSize = CHAR_SIZE * attribute.length
								* attribute.count;
						attribute.type = char[][].class;
					} else {
						log.error("Unknown attribute type: " + attribute.type);
						fail = true;
					}

					if (!fail) {
						buffer.attributes.add(attribute);

						// Extend the buffer by the required extra buffer size
						if (buffer.lastPad < typeSize) {
							buffer.lastPad = typeSize;
						}

						buffer.memSize = buffer.memSize
								+ (buffer.memSize % typeSize);
						attribute.memPosition = buffer.memSize;
						attribute.wirePosition = buffer.wireSize;
						buffer.wireSize = buffer.wireSize
								+ attribute.instanceSize;
						buffer.memSize = buffer.memSize
								+ attribute.instanceSize;
					}
				} else {
					log.error("Duplicate attribute detected: " + attribute.id);
				}
			} else {
				log.error("No buffer is being processed");
			}
		} else if (ORB.equals(localName)) {
			for (int j = 0; j < atts.getLength(); j++) {
				if (atts.getLocalName(j).equals("OPT")) {
					String[] argv;
					argv = atts.getValue(j).split(" ");
					int orbargs = argv.length;

					for (int i = 1; i <= orbargs; i++) {
						String arg = "blacktie.orb.arg." + i;
						String toSet = argv[i - 1];
						if (jbossasIpAddr != null) {
							toSet = toSet.replace("${JBOSSAS_IP_ADDR}",
									jbossasIpAddr);
						}
						prop.setProperty(arg, toSet);
						log.debug(arg + " is " + toSet);
					}

					log.debug("blacktie.orb.args is " + orbargs);
					prop.setProperty("blacktie.orb.args", Integer
							.toString(orbargs));
				} else if (atts.getLocalName(j).equals("TRANS_FACTORY_ID")) {
					prop.setProperty("blacktie.trans.factoryid", atts
							.getValue(j));
				}
			}
		} else if (MQ.equals(localName)) {
			for (int i = 0; i < atts.getLength(); i++) {
				if (atts.getLocalName(i).equals("USER")) {
					String value = atts.getValue(i);
					prop.setProperty("StompConnectUsr", value);
				} else if (atts.getLocalName(i).equals("PASSWORD")) {
					String value = atts.getValue(i);
					prop.setProperty("StompConnectPwd", value);
				} else if (atts.getLocalName(i).equals("DESTINATION_TIMEOUT")) {
					String value = atts.getValue(i);
					prop.setProperty("DestinationTimeout", value);
				} else if (atts.getLocalName(i).equals("RECEIVE_TIMEOUT")) {
					String value = atts.getValue(i);
					prop.setProperty("RequestTimeout", value);
				} else if (atts.getLocalName(i).equals("TIME_TO_LIVE")) {
					String value = atts.getValue(i);
					prop.setProperty("TimeToLive", value);
				} else if (atts.getLocalName(i).equals("NAMING_URL")) {
					String value = atts.getValue(i);
					if (jbossasIpAddr != null) {
						value = value.replace("${JBOSSAS_IP_ADDR}",
								jbossasIpAddr);
					}
					prop.setProperty("java.naming.provider.url", value);
				}
			}
		} else if (SERVICE_NAME.equals(localName)
				|| ADMIN_SERVICE_NAME.equals(localName)) {
			String serviceName = null;
			String transport = null;

			if (atts != null) {
				if (ADMIN_SERVICE_NAME.equals(localName)) {
					serviceName = serverName + "_ADMIN";
				}

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
					}
				}

				if (serviceName != null) {
					String key = "blacktie." + serviceName + ".server";
					prop.put(key, serverName);
				}
			}

			prop.put("blacktie." + serviceName + ".transportLib", "hybrid");

			if (serviceName != null) {
				AtmiBrokerServiceXML xml = new AtmiBrokerServiceXML(serverName,
						serviceName, prop);
				try {
					xml.getProperties(configDir);
				} catch (ConfigurationException e) {
					throw new SAXException(e.getMessage(), e);
				}
			}
		}
	}

	public void endElement(String namespaceURI, String localName, String qName)
			throws SAXException {
		if (DOMAIN.equals(localName)) {
			prop.setProperty("blacktie.domain.name", value);
		} else if (NAME.equals(localName)) {
			name = value;
		} else if (VALUE.equals(localName)) {
			if (jbossasIpAddr != null) {
				value = value.replace("${JBOSSAS_IP_ADDR}", jbossasIpAddr);
			}
			prop.setProperty(name, value);
		}
	}
}