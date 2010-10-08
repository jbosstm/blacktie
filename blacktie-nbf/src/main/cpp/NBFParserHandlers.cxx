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
#include <NBFParserHandlers.h>
#include <log4cxx/logger.h>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/SAXException.hpp>

log4cxx::LoggerPtr NBFParserHandlers::logger(
		log4cxx::Logger::getLogger("NBFParserHandlers"));

NBFParserHandlers::NBFParserHandlers(const char* attrName, int index) {
	if(attrName != NULL) {
		this->attrName = strdup(attrName);
	} else {
		this->attrName = NULL;
	}
	this->attrValue = NULL;
	this->index = index;
	this->curIndex = -1;
	this->found = false;
}

NBFParserHandlers::~NBFParserHandlers() {
	if(attrName != NULL) {
		free(attrName);
	}

	if(attrValue != NULL) {
		free(attrValue);
	}
}

char* NBFParserHandlers::getValue() {
	return attrValue;
}

void NBFParserHandlers::startElement(const XMLCh* const name, AttributeList& attributes) {
	StrX str(name);
	const char* qname = str.localForm();
	LOG4CXX_DEBUG(logger, "name:" << qname);
	if(strcmp(qname, attrName) == 0) {
		found = true;
		curIndex ++;
	}
}

void NBFParserHandlers::endElement( const XMLCh* const name) {
	StrX str(name);
	const char* qname = str.localForm();
	LOG4CXX_DEBUG(logger, "name:" << qname);
	if(strcmp(qname, attrName) == 0) {
		found = false;
	}
}

void NBFParserHandlers::characters(const XMLCh* const name,
								  const XMLSize_t length) {
	StrX str(name);
	const char* value = str.localForm();

	if(found && curIndex == index) {
		LOG4CXX_DEBUG(logger, "find value " << value << " at index " << index);
		attrValue = strdup(value);
	}
}

void NBFParserHandlers::resetDocument() {
}

void NBFParserHandlers::error(const SAXParseException& e) {
	LOG4CXX_ERROR(logger, "Error at (file " << StrX(e.getSystemId())
			<< ", line " << e.getLineNumber()
			<< ", char " << e.getColumnNumber()
			<< "): " << StrX(e.getMessage()));
}

void NBFParserHandlers::fatalError(const SAXParseException& e) {
	LOG4CXX_ERROR(logger, "Fatal Error at (file " << StrX(e.getSystemId())
			<< ", line " << e.getLineNumber()
			<< ", char " << e.getColumnNumber()
			<< "): " << StrX(e.getMessage()));
}

void NBFParserHandlers::warning(const SAXParseException& e) {
	LOG4CXX_WARN(logger, "Warning at (file " << StrX(e.getSystemId())
			<< ", line " << e.getLineNumber()
			<< ", char " << e.getColumnNumber()
			<< "): " << StrX(e.getMessage()));
}
