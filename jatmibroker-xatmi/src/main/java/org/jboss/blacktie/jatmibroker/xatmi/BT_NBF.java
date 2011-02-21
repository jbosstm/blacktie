/*
 * JBoss, Home of Professional Open Source
 * Copyright 2008, Red Hat, Inc., and others contributors as indicated
 * by the @authors tag. All rights reserved.
 * See the copyright.txt in the distribution for a
 * full listing of individual contributors.
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General public  License, v. 2.1.
 * This program is distributed in the hope that it will be useful, but WITHOUT A
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General public  License for more details.
 * You should have received a copy of the GNU Lesser General public  License,
 * v.2.1 along with this distribution; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */
package org.jboss.blacktie.jatmibroker.xatmi;

import java.io.File;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.nbf.NestedBufferParser;

public class BT_NBF extends Buffer {

	private static final Logger log = LogManager.getLogger(BT_NBF.class);
	/**
	 * The default ID
	 */
	private static final long serialVersionUID = 1L;
	private NestedBufferParser parser;
	private String rootElement;

	BT_NBF(String subtype) throws ConnectionException {
		super("BT_NBF", subtype, false, null, null, 0);

		rootElement = "</" + subtype + ">";
		String xsd = "buffers/" + subtype + ".xsd";
		File file = new File(xsd);
		if(!file.exists()) {
			throw new ConnectionException(Connection.TPEOS, "can not find " + xsd);
		}

		StringBuffer buffer = new StringBuffer();
		buffer.append("<?xml version='1.0'?>");
		buffer.append("<");
		buffer.append(subtype);
		buffer.append(" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"");
		buffer.append(" xmlns=\"http://www.jboss.org/blacktie\"");
		buffer.append(" xsi:schemaLocation=\"http://www.jboss.org/blacktie buffers/");
		buffer.append(subtype);
		buffer.append(".xsd\">");
		buffer.append("</");
		buffer.append(subtype);
		buffer.append(">");

		setRawData(buffer.toString().getBytes());
		parser = new NestedBufferParser(xsd);	
		parser.parse(getRawData());
	}

	private String insertString(String buffer, String attr) {
		int k = buffer.indexOf(rootElement);
		return buffer.substring(0, k) + attr + buffer.substring(k);
	}

	public boolean btaddattribute(String attrId, Object attrValue) {
		boolean rc = false;
		try {
			parser.setId(attrId);
			String buffer = new String(getRawData());
			String attr = "<" + attrId + "></" + attrId + ">";

			String newbuffer = insertString(buffer, attr);
			rc = parser.parse(newbuffer.getBytes());

			if(rc) {
				String type = parser.getType();
				StringBuffer buf = new StringBuffer();
				if(type.equals("long")) {
					buf.append((Long)attrValue);	
				} else if(type.equals("string")) {
					buf.append((String)attrValue);
				} else if(type.equals("integer")) {
					buf.append((Integer)attrValue);
				} else if(type.equals("short")) {
					buf.append((Short)attrValue);
				} else if(type.equals("float")) {
					buf.append((Float)attrValue);
				} else if(type.endsWith("_type")) {
					String nbf = new String(((BT_NBF)attrValue).getRawData());
					int k = nbf.indexOf(".xsd\">");
					int size = nbf.length();
					String test =nbf.substring(k + 6, size - attrId.length() - 3);
					buf.append(test);
				} else {
					log.error("Can not support type " + type);
					rc = false;
				}

				if(buf.length() > 0) {
					String newattr = "<" + attrId + ">" + buf + "</" + attrId + ">";
					String attrbuf = insertString(buffer, newattr);

					rc = parser.parse(attrbuf.getBytes());
					if(rc) {
						setRawData(attrbuf.getBytes());
					}
				}
			}
		} catch (ConnectionException e) {
			log.error("btaddattribute failed with " + e.getMessage());
		} catch (ClassCastException e) {
			rc = false;
			log.warn("type is " + parser.getType() + 
					" but attrValue type is " + attrValue.getClass().getName());
		} catch (Throwable e) {
			log.error("btaddattribute failed with " + e.getMessage());
		}
		return rc;
	}

	public Object btgetattribute(String attrId, int index) {
		Object toReturn = null;

		try {
			boolean rc = false;
			parser.setId(attrId);
			parser.setIndex(index);
			rc = parser.parse(getRawData());

			if(rc) {
				String type = parser.getType();
				String value = parser.getValue();

				log.debug("vlaue is " + value);
				if(value == null) {
					log.error("can not find " + attrId + " at index " + index);
				} else {
					if(type.equals("long")) {
						toReturn = Long.parseLong(value);
					} else if(type.equals("string")) {
						toReturn = value;
					} else if(type.equals("integer")) {
						toReturn = Integer.parseInt(value);
					} else if(type.equals("short")) {
						toReturn = Short.parseShort(value);
					} else if(type.equals("float")) {
						toReturn = Float.parseFloat(value);
					} else if(type.endsWith("_type")) {
						//log.info(value);
						toReturn = new BT_NBF(attrId);
						String nbf = new String(((BT_NBF)toReturn).getRawData());
						int k = nbf.indexOf(".xsd\">") + 6;
						int size = nbf.length();

						String buf = nbf.substring(0, k) + value + nbf.substring(k, size);
						//log.info(buf);
						((BT_NBF)toReturn).setRawData(buf.getBytes());

					} else {
						log.error("Can not support type " + type);
					}
				}
			}
		} catch (ConnectionException e) {
			log.error("btgetattribute failed with " + e.getMessage());
		}

		return toReturn;
	}

	public boolean btdelattribute(String attrId, int index) {
		boolean toReturn = false;

		try {
			boolean rc;
			parser.setId(attrId);
			parser.setIndex(index);
			rc = parser.parse(getRawData());

			if(rc) {
				String value = parser.getValue();
				if(value == null) {
					log.error("can not find " + attrId + " at index " + index);
				} else {
					String buf = new String(getRawData());
					String element = "<" + attrId + ">" + value + "</" + attrId + ">";
					//log.info("element is " + element);
					int n = 0;
					for(int i = 0; i <= index; i++) {
						int k = buf.indexOf(element);
						if(k >= 0) {
							String tmpbuf = buf.substring(k + element.length());
							buf = tmpbuf;
							//log.info("tmpbuf is " + tmpbuf);
							n = k;
						}
					}

					String newbuf = new String(getRawData());
					int size = newbuf.length();
					newbuf = newbuf.substring(0, n) + newbuf.substring(n + element.length(), size);
					log.info(newbuf);
					toReturn = true;
				}
			}
		}catch (ConnectionException e) {
			log.error("btgetattribute failed with " + e.getMessage());
		}
		return toReturn;
	}
}
