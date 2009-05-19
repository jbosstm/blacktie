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

import java.io.File;

import javax.xml.XMLConstants;
import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;
import javax.xml.validation.Schema;
import javax.xml.validation.SchemaFactory;

import org.jboss.blacktie.jatmibroker.core.CoreException;
import org.xml.sax.helpers.DefaultHandler;

/**
 * Class to create a parser and parse an XML file
 */
public class XMLParser {

	private DefaultHandler handler;
	private SAXParser saxParser;

	/**
	 * Constructor
	 * 
	 * @param handler
	 *            - DefaultHandler for the SAX parser
	 */
	public XMLParser(DefaultHandler handler, String xsdFilename)
			throws CoreException {
		this.handler = handler;
		create(xsdFilename);
	}

	/**
	 * Create the SAX parser
	 */
	private void create(String xsdFilename) throws CoreException {
		try {
			// Obtain a new instance of a SAXParserFactory.
			SAXParserFactory factory = SAXParserFactory.newInstance();
			factory.setNamespaceAware(true);
			factory.setValidating(true);

			SchemaFactory schemaFactory = SchemaFactory
					.newInstance(XMLConstants.W3C_XML_SCHEMA_NS_URI);
			Schema schema = schemaFactory.newSchema(new File(System
					.getenv("BLACKTIE_SCHEMA_DIR")
					+ "/" + xsdFilename));
			factory.setSchema(schema);

			saxParser = factory.newSAXParser();
		} catch (Throwable t) {
			throw new CoreException("Could not create a SAXParser: ", t);
		}
	}

	/**
	 * Parser a File
	 * 
	 * @param file
	 *            - File
	 */
	public void parse(File file) throws CoreException {
		try {
			saxParser.parse(file, handler);
		} catch (Throwable t) {
			throw new CoreException("Errors parse : " + file, t);
		}
	}
}
