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
#include "btnbf.h"
#include "log4cxx/logger.h"
#include "NBFParser.h"

log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("BTNbf"));

int btaddattribute(char** buf, char* attributeId, char* attributeValue, int len) {
	LOG4CXX_TRACE(logger, (char*) "btaddattribute");
	int rc = -1;
	char* p = *buf;
	char* q = rindex(p, '<');

	if(q == NULL) {
		LOG4CXX_WARN(logger, (char*) "buffer not validate");
	} else {
		int n = q - p;
		int size = strlen(p);

		LOG4CXX_DEBUG(logger, (char*) "n is " << n << ", size is " << size);
		char* tmp = (char*) malloc (sizeof(char) * (size - n + 1));
		strcpy(tmp, q);

		int tagsize = strlen(attributeId) * 2 + 5 + 1;
		char* tag = (char*) malloc (sizeof(char) * tagsize);
		sprintf(tag, "<%s></%s>", attributeId, attributeId);

		p = (char*) realloc(p, sizeof(char) * (size + tagsize + 1));
		p[n] = '\0';
		strcat(p, tag);
		strcat(p, tmp);
		*buf = p;

		free(tag);
		free(tmp);

		NBFParser nbf;
		NBFParserHandlers handler(attributeId, 0);
		bool result;

		result = nbf.parse(p, "btnbf", &handler);
		if(result) {
			rc = 0;
			const char* type = handler.getType();
			LOG4CXX_INFO(logger, (char*) "type is " << type);
		}
	}

	return rc;
}

int btgetattribute(char* buf, char* attributeId, int attributeIndex, char* attributeValue, int* len) {
	LOG4CXX_TRACE(logger, (char*) "btgetattribute");
	int rc = -1;
	bool result;
	NBFParser nbf;
	NBFParserHandlers handler(attributeId, attributeIndex);

	result = nbf.parse(buf, "btnbf", &handler);
	if(result) {
		LOG4CXX_DEBUG(logger, (char*) "find attributeId:" << attributeId
					<< " at " << attributeIndex);

		const char* value = handler.getValue();
		const char* type = handler.getType();

		rc = 0;
		if(type == NULL || strcmp(type, "string") == 0) {
			strncpy(attributeValue, value, *len);
			*len = strlen(attributeValue);
		} else if(strcmp(type, "long") == 0) {
			*((long*)attributeValue) = atol(value);
			*len = sizeof(long);
		} else {
			LOG4CXX_WARN(logger, "can not support type of " << type);
			rc = -1;
		}
	}

	return rc;
}
