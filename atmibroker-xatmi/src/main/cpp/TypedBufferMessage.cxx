/*
 * JBoss, Home of Professional Open Source
 * Copyright 2008, Red Hat Middleware LLC, and others contributors as indicated
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
/*
 * BREAKTHRUIT PROPRIETARY - NOT TO BE DISCLOSED OUTSIDE BREAKTHRUIT, LLC.
 */
// copyright 2006, 2008 BreakThruIT

#include "TypedBufferMessage.h"
#include "userlog.h"
#include <stdlib.h>

#include "log4cxx/logger.h"
using namespace log4cxx;
using namespace log4cxx::helpers;
LoggerPtr loggerTypedBufferMessage(Logger::getLogger("TypedBufferMessage"));

TypedBufferMessage::TypedBufferMessage(AtmiBroker::TypedBuffer& aTypedBuffer) :
	typedBuffer(aTypedBuffer) {
}

void TypedBufferMessage::addByte(char * name, unsigned int val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage addByte name: %s val: %c", name, val);

	AtmiBroker::TypedBufferMember * aTypedBufferMember = new AtmiBroker::TypedBufferMember();

	aTypedBufferMember->name = strdup(name);

	// ltoa
	std::ostringstream oss;
	oss << val << std::dec;
	const char* indexStr = oss.str().c_str();

	aTypedBufferMember->value = indexStr;

	typedBuffer.members.length(typedBuffer.members.length() + 1);
	typedBuffer.members[typedBuffer.members.length() - 1] = *aTypedBufferMember;
}

void TypedBufferMessage::addBytes(char * name, unsigned int* val, int len) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage addBytes name: %s val: %d", name, val);

	AtmiBroker::TypedBufferMember * aTypedBufferMember = new AtmiBroker::TypedBufferMember();

	aTypedBufferMember->name = strdup(name);

	// ltoa
	std::ostringstream oss;
	oss << val << std::dec;
	const char* indexStr = oss.str().c_str();

	aTypedBufferMember->value = indexStr;

	typedBuffer.members.length(typedBuffer.members.length() + 1);
	typedBuffer.members[typedBuffer.members.length() - 1] = *aTypedBufferMember;
}

void TypedBufferMessage::addBoolean(char * name, bool val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage addBoolean name: %s val: %d", name, val);

	AtmiBroker::TypedBufferMember * aTypedBufferMember = new AtmiBroker::TypedBufferMember();

	aTypedBufferMember->name = strdup(name);

	if (val)
		aTypedBufferMember->value = "1";
	else
		aTypedBufferMember->value = "0";

	typedBuffer.members.length(typedBuffer.members.length() + 1);
	typedBuffer.members[typedBuffer.members.length() - 1] = *aTypedBufferMember;
}

void TypedBufferMessage::addChar(char * name, char val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage addChar name: %s val: %d", name, val);

	AtmiBroker::TypedBufferMember * aTypedBufferMember = new AtmiBroker::TypedBufferMember();

	aTypedBufferMember->name = strdup(name);

	char aStr[10];
	sprintf(aStr, "%c", val);
	aTypedBufferMember->value = strdup(aStr);

	typedBuffer.members.length(typedBuffer.members.length() + 1);
	typedBuffer.members[typedBuffer.members.length() - 1] = *aTypedBufferMember;
}

void TypedBufferMessage::addDouble(char * name, double val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage addDouble name: %s val: %d", name, val);

	AtmiBroker::TypedBufferMember * aTypedBufferMember = new AtmiBroker::TypedBufferMember();

	aTypedBufferMember->name = strdup(name);

	char aStr[10];
	sprintf(aStr, "%f", val);
	aTypedBufferMember->value = strdup(aStr);

	typedBuffer.members.length(typedBuffer.members.length() + 1);

	typedBuffer.members[typedBuffer.members.length() - 1] = *aTypedBufferMember;
}

void TypedBufferMessage::addFloat(char * name, float val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage addFloat name: %s val: %d", name, val);

	AtmiBroker::TypedBufferMember * aTypedBufferMember = new AtmiBroker::TypedBufferMember();

	aTypedBufferMember->name = strdup(name);

	char aStr[10];
	sprintf(aStr, "%f", val);
	aTypedBufferMember->value = strdup(aStr);

	typedBuffer.members.length(typedBuffer.members.length() + 1);
	typedBuffer.members[typedBuffer.members.length() - 1] = *aTypedBufferMember;
}

void TypedBufferMessage::addInt(char * name, int val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage addInt name: %s val: %d", name, val);

	AtmiBroker::TypedBufferMember * aTypedBufferMember = new AtmiBroker::TypedBufferMember();

	aTypedBufferMember->name = strdup(name);

	char aStr[10];
	sprintf(aStr, "%d", val);
	aTypedBufferMember->value = strdup(aStr);

	typedBuffer.members.length(typedBuffer.members.length() + 1);
	typedBuffer.members[typedBuffer.members.length() - 1] = *aTypedBufferMember;
}

void TypedBufferMessage::addShort(char * name, short val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage addShort name: %s val: %d", name, val);

	AtmiBroker::TypedBufferMember * aTypedBufferMember = new AtmiBroker::TypedBufferMember();

	aTypedBufferMember->name = strdup(name);

	char aStr[10];
	sprintf(aStr, "%d", val);
	aTypedBufferMember->value = strdup(aStr);

	typedBuffer.members.length(typedBuffer.members.length() + 1);
	typedBuffer.members[typedBuffer.members.length() - 1] = *aTypedBufferMember;
}

void TypedBufferMessage::addLong(char * name, long val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage addLong name: %s val: %d", name, val);

	AtmiBroker::TypedBufferMember * aTypedBufferMember = new AtmiBroker::TypedBufferMember();

	aTypedBufferMember->name = strdup(name);

	char aStr[10];
	sprintf(aStr, "%u", val);
	aTypedBufferMember->value = strdup(aStr);

	typedBuffer.members.length(typedBuffer.members.length() + 1);
	typedBuffer.members[typedBuffer.members.length() - 1] = *aTypedBufferMember;
}

void TypedBufferMessage::addAny(char * name, CORBA::Any val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage addAny name: %s", name);

	AtmiBroker::TypedBufferMember * aTypedBufferMember = new AtmiBroker::TypedBufferMember();

	aTypedBufferMember->name = strdup(name);

	const char *aStr;
	val >>= aStr;
	aTypedBufferMember->value = strdup(aStr);

	typedBuffer.members.length(typedBuffer.members.length() + 1);
	typedBuffer.members[typedBuffer.members.length() - 1] = *aTypedBufferMember;
}

void TypedBufferMessage::addString(char * name, char * val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage addchar * name: %s val: %d", name, val);

	AtmiBroker::TypedBufferMember * aTypedBufferMember = new AtmiBroker::TypedBufferMember();

	aTypedBufferMember->name = strdup(name);
	aTypedBufferMember->value = strdup(val);

	typedBuffer.members.length(typedBuffer.members.length() + 1);
	typedBuffer.members[typedBuffer.members.length() - 1] = *aTypedBufferMember;
}

void TypedBufferMessage::clear() {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage clear");

	typedBuffer.members.length(0);
}

void TypedBufferMessage::deleteNamed(char * name) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage deleteNamed name: %s ", name);

	AtmiBroker::TypedBufferMemberSeq* newTypedBufferMemberSeq = new AtmiBroker::TypedBufferMemberSeq();

	unsigned int i = 0;
	for (i = 0; i < typedBuffer.members.length(); i++) {
		if (strcmp(name, typedBuffer.members[i].name) != 0) {
			newTypedBufferMemberSeq->length(i++);
			(*newTypedBufferMemberSeq)[i] = typedBuffer.members[i];
			userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage deleted name: %s ", name);
		}
	}
	typedBuffer.members.length(newTypedBufferMemberSeq->length());
	typedBuffer.members = *newTypedBufferMemberSeq;
}

void TypedBufferMessage::deleteItem(char * name, int itemNo) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage deleteItem name: %s  occurence: %d", name, itemNo);

	AtmiBroker::TypedBufferMemberSeq* newTypedBufferMemberSeq = new AtmiBroker::TypedBufferMemberSeq();

	unsigned int i = 0;
	int count = 0;
	for (i = 0; i < typedBuffer.members.length(); i++) {
		if (strcmp(name, typedBuffer.members[i].name) != 0) {
			count++;
			if (count == itemNo) {
				newTypedBufferMemberSeq->length(i++);
				(*newTypedBufferMemberSeq)[i] = typedBuffer.members[i];
				userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage deleted name: %s ", name);
			}
		}
	}
	typedBuffer.members.length(newTypedBufferMemberSeq->length());
	typedBuffer.members = *newTypedBufferMemberSeq;
}

int TypedBufferMessage::getOccurrenceCount(char * name) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage delete name: %s ", name);

	unsigned int i = 0;
	int count = 0;
	for (i = 0; i < typedBuffer.members.length(); i++) {
		if (strcmp(name, typedBuffer.members[i].name) == 0) {
			count++;
		}
	}
	return count;
}

unsigned int TypedBufferMessage::getByteDef(char * name, unsigned int val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "getByteDef name: %s ", name);

	return getByteItemDef(name, 1, val);
}

unsigned int TypedBufferMessage::getByteItemDef(char * name, int itemNo, unsigned int val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage getByteItemDef name: %s occurence: %d", name, itemNo);

	unsigned int i = 0;
	int count = 0;
	for (i = 0; i < typedBuffer.members.length(); i++) {
		if (strcmp(name, typedBuffer.members[i].name) == 0) {
			count++;
			if (count == itemNo) {
				userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "found name: %s ", name);
				unsigned int aByte;
				sscanf(typedBuffer.members[i].value, "%d", &aByte);
				return aByte;
			}
		}
	}
	return 0;
}

unsigned int* TypedBufferMessage::getBytesDef(char * name, unsigned int* val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "getBytesDef name: %s ", name);

	return getBytesItemDef(name, 1, val);
}

unsigned int* TypedBufferMessage::getBytesItemDef(char * name, int itemNo, unsigned int* val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage getBytesItemDef name: %s occurence: %d", name, itemNo);

	unsigned int i = 0;
	int count = 0;
	for (i = 0; i < typedBuffer.members.length(); i++) {
		if (strcmp(name, typedBuffer.members[i].name) == 0) {
			count++;
			if (count == itemNo) {
				userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "found name: %s ", name);
				unsigned int* aBytes = (unsigned int *) malloc(sizeof(unsigned int*) * strlen(typedBuffer.members[i].value));
				for (unsigned int j = 0; j < strlen(typedBuffer.members[i].value); j++)
					aBytes[j] = (unsigned int) typedBuffer.members[i].value[j];
				return aBytes;
			}
		}
	}
	return val;
}

bool TypedBufferMessage::getBooleanDef(char * name, bool val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage getBooleanDef name: %s ", name);

	return getBooleanItemDef(name, 1, val);
}

bool TypedBufferMessage::getBooleanItemDef(char * name, int itemNo, bool val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage getBooleanItemDef name: %s occurence: %d", name, itemNo);

	int count = 0;
	for (unsigned int i = 0; i < typedBuffer.members.length(); i++) {
		if (strcmp(name, typedBuffer.members[i].name) == 0) {
			count++;
			if (count == itemNo) {
				userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "found name: %s ", name);
				bool aBoolean = false;
				if (strcmp(typedBuffer.members[i].value, "1") == 0)
					aBoolean = true;
				return aBoolean;
			}
		}
	}
	return val;
}

char TypedBufferMessage::getCharDef(char * name, char val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage getCharDef name: %s ", name);

	return getCharItemDef(name, 1, val);
}

char TypedBufferMessage::getCharItemDef(char * name, int itemNo, char val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage getCharItemDef name: %s occurence: %d", name, itemNo);

	unsigned int i = 0;
	int count = 0;
	for (i = 0; i < typedBuffer.members.length(); i++) {
		if (strcmp(name, typedBuffer.members[i].name) == 0) {
			count++;
			if (count == itemNo) {
				userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "found name: %s ", name);
				char aChar = typedBuffer.members[i].value[0];
				return aChar;
			}
		}
	}
	return val;
}

double TypedBufferMessage::getDoubleDef(char * name, double val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage getDoubleDef name: %s ", name);

	return getDoubleItemDef(name, 1, val);
}

double TypedBufferMessage::getDoubleItemDef(char * name, int itemNo, double val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage getDoubleItemDef name: %s occurence: %d", name, itemNo);

	unsigned int i = 0;
	int count = 0;
	for (i = 0; i < typedBuffer.members.length(); i++) {
		if (strcmp(name, typedBuffer.members[i].name) == 0) {
			count++;
			if (count == itemNo) {
				userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "found name: %s ", name);
				double aDouble;
				sscanf(typedBuffer.members[i].value, "%f", &aDouble);
				return aDouble;
			}
		}
	}
	return val;
}

float TypedBufferMessage::getFloatDef(char * name, float val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage getFloatDef name: %s ", name);

	return getFloatItemDef(name, 1, val);
}

float TypedBufferMessage::getFloatItemDef(char * name, int itemNo, float val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage getFloatItemDef name: %s occurence: %d", name, itemNo);

	unsigned int i = 0;
	int count = 0;
	for (i = 0; i < typedBuffer.members.length(); i++) {
		if (strcmp(name, typedBuffer.members[i].name) == 0) {
			count++;
			if (count == itemNo) {
				userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "found name: %s ", name);
				float aFloat;
				sscanf(typedBuffer.members[i].value, "%f", &aFloat);
				return aFloat;
			}
		}
	}
	return val;
}

int TypedBufferMessage::getIntDef(char * name, int val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage getFloatDef name: %s ", name);

	return getIntItemDef(name, 1, val);
}

int TypedBufferMessage::getIntItemDef(char * name, int itemNo, int val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage getIntItemDef name: %s occurence: %d", name, itemNo);

	unsigned int i = 0;
	int count = 0;
	for (i = 0; i < typedBuffer.members.length(); i++) {
		if (strcmp(name, typedBuffer.members[i].name) == 0) {
			count++;
			if (count == itemNo) {
				userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "found name: %s ", name);
				int aInt;
				sscanf(typedBuffer.members[i].value, "%d", &aInt);
				return aInt;
			}
		}
	}
	return val;
}

short TypedBufferMessage::getShortDef(char * name, short val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage getShortDef name: %s ", name);

	return getShortItemDef(name, 1, val);
}

short TypedBufferMessage::getShortItemDef(char * name, int itemNo, short val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage getShortItemDef name: %s occurence: %d", name, itemNo);

	unsigned int i = 0;
	int count = 0;
	for (i = 0; i < typedBuffer.members.length(); i++) {
		if (strcmp(name, typedBuffer.members[i].name) == 0) {
			count++;
			if (count == itemNo) {
				userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "found name: %s ", name);
				short aShort;
				sscanf(typedBuffer.members[i].value, "%d", &aShort);
				return aShort;
			}
		}
	}
	return val;
}

long TypedBufferMessage::getLongDef(char * name, long val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage getLongDef name: %s ", name);

	return getLongItemDef(name, 1, val);
}

long TypedBufferMessage::getLongItemDef(char * name, int itemNo, long val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage getLongItemDef name: %s occurence: %d", name, itemNo);

	unsigned int i = 0;
	int count = 0;
	for (i = 0; i < typedBuffer.members.length(); i++) {
		if (strcmp(name, typedBuffer.members[i].name) == 0) {
			count++;
			if (count == itemNo) {
				userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "found name: %s ", name);
				long aLong;
				sscanf(typedBuffer.members[i].value, "%u", &aLong);
				return aLong;
			}
		}
	}
	return val;
}

char * TypedBufferMessage::getStringDef(char * name, char * val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage getStringDef name: %s ", name);

	return getStringItemDef(name, 1, val);
}

char * TypedBufferMessage::getStringItemDef(char * name, int itemNo, char * val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage getStringItemDef name: %s occurence: %d", name, itemNo);

	unsigned int i = 0;
	int count = 0;
	for (i = 0; i < typedBuffer.members.length(); i++) {
		if (strcmp(name, typedBuffer.members[i].name) == 0) {
			count++;
			if (count == itemNo) {
				userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "found name: %s ", name);
				const char * aString = (const char *) typedBuffer.members[i].value;
				return (char*) aString;
			}
		}
	}
	return val;
}

void TypedBufferMessage::setByte(char * name, unsigned int val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage setByte name: %s ", name);

	getByteItemDef(name, 1, val);
}

void TypedBufferMessage::setByteItem(char * name, int itemNo, unsigned int val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage setByteItem name: %s occurence: %d", name, itemNo);

	unsigned int i = 0;
	int count = 0;
	for (i = 0; i < typedBuffer.members.length(); i++) {
		if (strcmp(name, typedBuffer.members[i].name) == 0) {
			count++;
			if (count == itemNo) {
				userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "found name: %s ", name);
				char str[5];
				sprintf(str, "%u", val);
				typedBuffer.members[i].value = strdup(str);
			}
		}
	}
}

void TypedBufferMessage::setBytes(char * name, unsigned int* val, int len) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "setBytes name: %s ", name);

	setBytesItem(name, 1, val, len);
}

void TypedBufferMessage::setBytesItem(char * name, int itemNo, unsigned int* val, int len) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage setBytesItem name: %s occurence: %d", name, itemNo);

	unsigned int i = 0;
	int count = 0;
	for (i = 0; i < typedBuffer.members.length(); i++) {
		if (strcmp(name, typedBuffer.members[i].name) == 0) {
			count++;
			if (count == itemNo) {
				userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "found name: %s ", name);
				// typedBuffer.members[i].value = String.valueOf(val); TEMP
			}
		}
	}
}

void TypedBufferMessage::setBoolean(char * name, bool val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage setBoolean name: %s ", name);
	getBooleanItemDef(name, 1, val);
}

void TypedBufferMessage::setBooleanItem(char * name, int itemNo, bool val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage setBooleanItem name: %s occurence: %d", name, itemNo);

	unsigned int i = 0;
	int count = 0;
	for (i = 0; i < typedBuffer.members.length(); i++) {
		if (strcmp(name, typedBuffer.members[i].name) == 0) {
			count++;
			if (count == itemNo) {
				userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "found name: %s ", name);
				if (val)
					typedBuffer.members[i].value = strdup("1");
				else
					typedBuffer.members[i].value = strdup("0");
			}
		}
	}
}

void TypedBufferMessage::setChar(char * name, char val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage setChar name: %s ", name);

	setCharItem(name, 1, val);
}

void TypedBufferMessage::setCharItem(char * name, int itemNo, char val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage setCharItem name: %s occurence: %d", name, itemNo);

	unsigned int i = 0;
	int count = 0;
	for (i = 0; i < typedBuffer.members.length(); i++) {
		if (strcmp(name, typedBuffer.members[i].name) == 0) {
			count++;
			if (count == itemNo) {
				userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "found name: %s ", name);
				char str[5];
				sprintf(str, "%c", val);
				typedBuffer.members[i].value = strdup(str);
			}
		}
	}
}

void TypedBufferMessage::setDouble(char * name, double val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage setDouble name: %s ", name);

	setDoubleItem(name, 1, val);
}

void TypedBufferMessage::setDoubleItem(char * name, int itemNo, double val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage setDoubleItem name: %s occurence: %d", name, itemNo);

	unsigned int i = 0;
	int count = 0;
	for (i = 0; i < typedBuffer.members.length(); i++) {
		if (strcmp(name, typedBuffer.members[i].name) == 0) {
			count++;
			if (count == itemNo) {
				userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "found name: %s ", name);
				char str[15];
				sprintf(str, "%f", val);
				typedBuffer.members[i].value = strdup(str);
			}
		}
	}
}

void TypedBufferMessage::setFloat(char * name, float val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage setFloat name: %s ", name);

	setFloatItem(name, 1, val);
}

void TypedBufferMessage::setFloatItem(char * name, int itemNo, float val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage setFloatItem name: %s occurence: %d", name, itemNo);

	unsigned int i = 0;
	int count = 0;
	for (i = 0; i < typedBuffer.members.length(); i++) {
		if (strcmp(name, typedBuffer.members[i].name) == 0) {
			count++;
			if (count == itemNo) {
				userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "found name: %s ", name);
				char str[15];
				sprintf(str, "%f", val);
				typedBuffer.members[i].value = strdup(str);
			}
		}
	}
}

void TypedBufferMessage::setInt(char * name, int val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage setFloat name: %s ", name);

	setIntItem(name, 1, val);
}

void TypedBufferMessage::setIntItem(char * name, int itemNo, int val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage setIntItem name: %s occurence: %d", name, itemNo);

	unsigned int i = 0;
	int count = 0;
	for (i = 0; i < typedBuffer.members.length(); i++) {
		if (strcmp(name, typedBuffer.members[i].name) == 0) {
			count++;
			if (count == itemNo) {
				userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "found name: %s ", name);
				char str[15];
				sprintf(str, "%d", val);
				typedBuffer.members[i].value = strdup(str);
			}
		}
	}
}

void TypedBufferMessage::setShort(char * name, short val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage setShort name: %s ", name);

	setShortItem(name, 1, val);
}

void TypedBufferMessage::setShortItem(char * name, int itemNo, short val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage setShortItem name: %s occurence: %d", name, itemNo);

	unsigned int i = 0;
	int count = 0;
	for (i = 0; i < typedBuffer.members.length(); i++) {
		if (strcmp(name, typedBuffer.members[i].name) == 0) {
			count++;
			if (count == itemNo) {
				userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "found name: %s ", name);
				char str[15];
				sprintf(str, "%d", val);
				typedBuffer.members[i].value = strdup(str);
			}
		}
	}
}

void TypedBufferMessage::setLong(char * name, long val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage setLong name: %s ", name);

	setLongItem(name, 1, val);
}

void TypedBufferMessage::setLongItem(char * name, int itemNo, long val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage setLongItem name: %s occurence: %d", name, itemNo);

	unsigned int i = 0;
	int count = 0;
	for (i = 0; i < typedBuffer.members.length(); i++) {
		if (strcmp(name, typedBuffer.members[i].name) == 0) {
			count++;
			if (count == itemNo) {
				userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "found name: %s ", name);
				char str[15];
				sprintf(str, "%u", val);
				typedBuffer.members[i].value = strdup(str);
			}
		}
	}
}

void TypedBufferMessage::setString(char * name, char * val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage setchar * name: %s ", name);

	setStringItem(name, 1, val);
}

void TypedBufferMessage::setStringItem(char * name, int itemNo, char * val) {
	userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "TypedBufferMessage setStringItem name: %s occurence: %d", name, itemNo);

	unsigned int i = 0;
	int count = 0;
	for (i = 0; i < typedBuffer.members.length(); i++) {
		if (strcmp(name, typedBuffer.members[i].name) == 0) {
			count++;
			if (count == itemNo) {
				userlog(Level::getInfo(), loggerTypedBufferMessage, (char*) "found name: %s ", name);
				typedBuffer.members[i].value = strdup(val);
			}
		}
	}
}
