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
		strcpy(attributeValue, value);
		*len = strlen(value);
		rc = 0;
	}

	return rc;
}
