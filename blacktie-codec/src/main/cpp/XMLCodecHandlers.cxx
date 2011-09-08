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
#include <stdio.h>
#include <XMLCodecHandlers.h>
#include <Base64.h>
#include <log4cxx/logger.h>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/SAXException.hpp>
#include <xercesc/framework/psvi/XSTypeDefinition.hpp>
#include <xercesc/framework/psvi/PSVIElement.hpp>
#include <xercesc/framework/psvi/XSConstants.hpp>

log4cxx::LoggerPtr XMLCodecHandlers::logger(
		log4cxx::Logger::getLogger("XMLCodecHandlers"));

XMLCodecHandlers::XMLCodecHandlers(char* type, char* subtype) {
	this->type = strdup(type);
	this->subtype = strdup(subtype);
	buffer = NULL;
	length = 0;
	foundOctet = false;
}

XMLCodecHandlers::~XMLCodecHandlers() {
	if(type) {
		free(type);
	}

	if(subtype) {
		free(subtype);
	}
}

char* XMLCodecHandlers::getBuffer() {
	return buffer;
}

long XMLCodecHandlers::getLength() {
	return length;
}

void XMLCodecHandlers::startElement(const XMLCh* const name, AttributeList& attributes) {
	StrX str(name);
	const char* qname = str.localForm();
	LOG4CXX_DEBUG(logger, "start name:" << qname);
	if(qname && strcmp(qname, "bt:X_OCTET") == 0) {
		foundOctet = true;
	}
}

void XMLCodecHandlers::endElement( const XMLCh* const name) {
	StrX str(name);
	const char* qname = str.localForm();
	LOG4CXX_DEBUG(logger, "end name:" << qname);
	if(qname && strcmp(qname, "bt:X_OCTET") == 0) {
		foundOctet = false;
	}
}

void XMLCodecHandlers::characters(const XMLCh* const name,
								  const XMLSize_t length) {
	StrX str(name);
	const char* value = str.localForm();
	LOG4CXX_DEBUG(logger, "value:" << value);
	if(foundOctet) {
		buffer = base64_decode((char*)value, &this->length);
	}
}

void XMLCodecHandlers::resetDocument() {
}

void XMLCodecHandlers::error(const SAXParseException& e) {
	LOG4CXX_ERROR(logger, "Error at (file " << StrX(e.getSystemId())
			<< ", line " << e.getLineNumber()
			<< ", char " << e.getColumnNumber()
			<< "): " << StrX(e.getMessage()));
}

void XMLCodecHandlers::fatalError(const SAXParseException& e) {
	LOG4CXX_ERROR(logger, "Fatal Error at (file " << StrX(e.getSystemId())
			<< ", line " << e.getLineNumber()
			<< ", char " << e.getColumnNumber()
			<< "): " << StrX(e.getMessage()));
}

void XMLCodecHandlers::warning(const SAXParseException& e) {
	LOG4CXX_WARN(logger, "Warning at (file " << StrX(e.getSystemId())
			<< ", line " << e.getLineNumber()
			<< ", char " << e.getColumnNumber()
			<< "): " << StrX(e.getMessage()));
}

void XMLCodecHandlers::handleElementPSVI(const XMLCh* const localName, 
		const XMLCh* const uri,
		PSVIElement* elementInfo) {
	LOG4CXX_DEBUG(logger, "handleElementPSVI " << StrX(localName));
	
}

void XMLCodecHandlers::handlePartialElementPSVI(const XMLCh* const localName, 
		const XMLCh* const uri,
		PSVIElement* elementInfo) {
	LOG4CXX_DEBUG(logger, "handlePartialElementPSVI " << StrX(localName));
}

void XMLCodecHandlers::handleAttributesPSVI(const XMLCh* const localName, 
		const XMLCh* const uri,  
		PSVIAttributeList* elementInfo) {
	LOG4CXX_DEBUG(logger, "handleAttributesPSVI " << StrX(localName));
}
