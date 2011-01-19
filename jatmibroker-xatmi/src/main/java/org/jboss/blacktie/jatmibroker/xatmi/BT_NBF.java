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
	
	BT_NBF(String subtype) throws ConnectionException {
		super("BT_NBF", subtype, false, null, null, 0);
		
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
		
		boolean rc = parser.parse(getRawData());
		log.debug("parser buffer rc: " + rc);
	}
}
