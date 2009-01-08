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

// TypedBufferMessage.h

#ifndef TYPED_BUFFER_MESSAGE_H
#define TYPED_BUFFER_MESSAGE_H

#include "atmiBrokerMacro.h"

#ifdef TAO_COMP
#include <orbsvcs/CosNotifyChannelAdminS.h>
#include <orbsvcs/CosNotifyCommS.h>
#include "AtmiBrokerC.h"
#elif ORBIX_COMP
#include <omg/CosNotifyChannelAdmin.hh>
#include <omg/CosNotifyCommS.hh>
#include "AtmiBroker.hh"
#endif
#ifdef VBC_COMP
#include <CosNotifyChannelAdmin_s.hh>
#include <CosNotifyComm_s.hh>
#include "AtmiBroker_c.hh"
#endif

class ATMIBROKER_DLL TypedBufferMessage {

public:

	TypedBufferMessage(AtmiBroker::TypedBuffer & typedBuffer);

	~TypedBufferMessage();

	void addByte(char * name, unsigned int val);

	void addBytes(char * name, unsigned int* val, int len);

	void addBoolean(char * name, bool val);

	void addChar(char * name, char val);

	void addDouble(char * name, double val);

	void addFloat(char * name, float val);

	void addInt(char * name, int val);

	void addShort(char * name, short val);

	void addLong(char * name, long val);

	void addAny(char * name, CORBA::Any val);

	void addString(char * name, char * val);

	void clear();

	void deleteNamed(char * name);

	void deleteItem(char * name, int itemNo);

	int getOccurrenceCount(char * name);

	unsigned int getByteDef(char * name, unsigned int val);

	unsigned int getByteItemDef(char * name, int itemNo, unsigned int val);

	unsigned int* getBytesDef(char * name, unsigned int* val);

	unsigned int* getBytesItemDef(char * name, int itemNo, unsigned int* val);

	bool getBooleanDef(char * name, bool val);

	bool getBooleanItemDef(char * name, int itemNo, bool val);

	char getCharDef(char * name, char val);

	char getCharItemDef(char * name, int itemNo, char val);

	double getDoubleDef(char * name, double val);

	double getDoubleItemDef(char * name, int itemNo, double val);

	float getFloatDef(char * name, float val);

	float getFloatItemDef(char * name, int itemNo, float val);

	int getIntDef(char * name, int val);

	int getIntItemDef(char * name, int itemNo, int val);

	short getShortDef(char * name, short val);

	short getShortItemDef(char * name, int itemNo, short val);

	long getLongDef(char * name, long val);

	long getLongItemDef(char * name, int itemNo, long val);

	char * getStringDef(char * name, char * val);

	char * getStringItemDef(char * name, int itemNo, char * val);

	void setByte(char * name, unsigned int val);

	void setByteItem(char * name, int itemNo, unsigned int val);

	void setBytes(char * name, unsigned int* val, int len);

	void setBytesItem(char * name, int itemNo, unsigned int* val, int len);

	void setBoolean(char * name, bool val);

	void setBooleanItem(char * name, int itemNo, bool val);

	void setChar(char * name, char val);

	void setCharItem(char * name, int itemNo, char val);

	void setDouble(char * name, double val);

	void setDoubleItem(char * name, int itemNo, double val);

	void setFloat(char * name, float val);

	void setFloatItem(char * name, int itemNo, float val);

	void setInt(char * name, int val);

	void setIntItem(char * name, int itemNo, int val);

	void setShort(char * name, short val);

	void setShortItem(char * name, int itemNo, short val);

	void setLong(char * name, long val);

	void setLongItem(char * name, int itemNo, long val);

	void setString(char * name, char * val);

	void setStringItem(char * name, int itemNo, char * val);

private:

	AtmiBroker::TypedBuffer & typedBuffer;

};

#endif //TYPED_BUFFER_MESSAGE_H
