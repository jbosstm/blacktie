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

void del_string(char* buf, int pos, int len) {
	int i, j;
	int n = strlen(buf);

	if(pos <= 0 || len <= 0 || pos + len >= n) {
		LOG4CXX_WARN(logger, (char*) "pos is " << pos << ", len is " << len << ", n is " << n);
		return;
	}

	for(i = pos, j = pos + len; j <= n; i++, j++) {
		buf[i] = buf[j];
	}
}

void insert_string(char** buf, const char* s, int pos) {
	char* p = *buf;
	int n = strlen(p);

	if(pos >= 0 && pos <= n) {
		char* q = p + pos;
		char* tmp = (char*) malloc (sizeof(char) * (n - pos + 1));
		strcpy(tmp, q);

		int size = strlen(s);
		p = (char*) realloc (p, sizeof(char) * (n + size + 1));
		p[pos] = '\0';
		strcat(p, s);
		strcat(p, tmp);
		*buf = p;
		free(tmp);
	}
}

int btaddattribute(char** buf, char* attributeId, char* attributeValue, int len) {
	LOG4CXX_TRACE(logger, (char*) "btaddattribute");
	int rc = -1;
	char* p = *buf;
	char* q = rindex(p, '<');

	if(q == NULL) {
		LOG4CXX_WARN(logger, (char*) "buffer not validate");
	} else {
		int n = q - p;

		int tagsize = strlen(attributeId) * 2 + 5 + 1;
		char* tag = (char*) malloc (sizeof(char) * tagsize);
		sprintf(tag, "<%s></%s>", attributeId, attributeId);
		int startPos = n + strlen(attributeId) + 2;
		insert_string(buf, tag, n);
		free(tag);

		NBFParser nbf;
		NBFParserHandlers handler(attributeId, 0);
		bool result;

		result = nbf.parse(*buf, "btnbf", &handler);
		if(result) {
			rc = 0;
			const char* type = handler.getType();
			LOG4CXX_INFO(logger, (char*) "type is " << type);
			char* value;
			if(strcmp(type, "string") == 0) {
				value = attributeValue;
			} else if(strcmp(type, "long") == 0) {
				value = (char*) malloc (sizeof(char) * 64);
				sprintf(value, "%ld", *((long*)attributeValue));
			}

			insert_string(buf, value, startPos);
			if(value != attributeValue) {
				LOG4CXX_DEBUG(logger, (char*) "release value for not string");
				free(value);
			}
		} else {
			LOG4CXX_WARN(logger, (char*) "can not add attribute " << attributeId);
			del_string(*buf, n, tagsize); 
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
