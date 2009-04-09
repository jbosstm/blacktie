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

import java.io.File;

import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;

import org.xml.sax.helpers.DefaultHandler;
import org.xml.sax.XMLReader;

/**
 * Class to create a parser and parse an XML file
 */
public class XMLParser {

	private DefaultHandler handler;
	private SAXParser saxParser;

	/**
	 * Constructor
	 * @param handler - DefaultHandler for the SAX parser
	 */
	public XMLParser(DefaultHandler handler) throws JAtmiBrokerException {
		this.handler = handler;
		create();
	}

	/**
	 * Create the SAX parser
	 */
	private void create() throws JAtmiBrokerException {
		try{
			// Obtain a new instance of a SAXParserFactory.
			SAXParserFactory factory = SAXParserFactory.newInstance();
			factory.setNamespaceAware(true);
			factory.setValidating(true);

			saxParser = factory.newSAXParser();
		} catch (Throwable t) {
			throw new JAtmiBrokerException("Could not create a SAXParser: ", t);
		}
	}

	/**
	 * Parser a File
	 * @param file - File
	 */
	public void parse(File file) throws JAtmiBrokerException {
		try{
			saxParser.parse(file, handler);
		} catch (Throwable t) {
			throw new JAtmiBrokerException("Errors parse : " + file, t);
		}
	}
}
