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
package org.jboss.blacktie.jatmibroker.jab;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.proxy.AtmiBrokerServiceFactory;
import org.omg.CORBA.Any;
import org.omg.CORBA.IntHolder;
import org.omg.CORBA.StringHolder;
import org.omg.CosTransactions.Control;

public class JABRemoteService implements Message {
	private static final Logger log = LogManager.getLogger(JABRemoteService.class);
	private JABSession jabSession;
	private AtmiBrokerServiceFactory serviceFactory;
	private String serviceName;
	private byte[] data;
	private JABMessage result;
	private JABMessage typedBuffer = new JABMessage();
	private boolean useData = true;

	public JABRemoteService(JABSession aJABSession, String aServiceName) throws JABException {
		log.debug("JABService constructor ");

		jabSession = aJABSession;
		serviceName = aServiceName;

		try {
			serviceFactory = aJABSession.getServerProxy().getServiceFactoryProxy(serviceName);
		} catch (Exception e) {
			throw new JABException(e);
		}
	}

	public void call(JABTransaction aJABTransaction) throws JABException {
		log.debug("JABService call ");

		try {
			org.omg.CORBA.IntHolder olen = new org.omg.CORBA.IntHolder();
			int flags = 0;
			Control control = null;
			if (aJABTransaction != null) {
				control = aJABTransaction.getControl();
			}
			StringHolder id = new StringHolder();
			IntHolder event = new IntHolder();
			AtmiBroker.octetSeqHolder odata = new AtmiBroker.octetSeqHolder();

			jabSession.getServerProxy().getServiceFactoryProxy(serviceName).start_conversation(id);
			jabSession.getServerProxy().getServiceFactoryProxy(serviceName).send_data(id.value, data, flags);
			short retVal = jabSession.getServerProxy().dequeue_data(odata, olen, flags, event);
			jabSession.getServerProxy().getServiceFactoryProxy(serviceName).end_conversation(id.value);
			
			data = new byte[olen.value];
			System.arraycopy(odata.value, 0, data, 0, olen.value);
			log.debug("service_request response is " + odata.value);
			// }

			log.debug("service_request retVal " + retVal);
			log.debug("service_request size of response is " + olen.value);

		} catch (Exception e) {
			throw new JABException(e);
		}
	}

	public void addByte(String name, byte val) {
		typedBuffer.addByte(name, val);
	}

	public void addBytes(String name, byte[] val) {
		typedBuffer.addBytes(name, val);
	}

	public void addBoolean(String name, boolean val) {
		typedBuffer.addBoolean(name, val);
	}

	public void addChar(String name, char val) {
		typedBuffer.addChar(name, val);
	}

	public void addDouble(String name, double val) {
		typedBuffer.addDouble(name, val);
	}

	public void addFloat(String name, float val) {
		typedBuffer.addFloat(name, val);
	}

	public void addInt(String name, int val) {
		typedBuffer.addInt(name, val);
	}

	public void addShort(String name, short val) {
		typedBuffer.addShort(name, val);
	}

	public void addLong(String name, long val) {
		typedBuffer.addLong(name, val);
	}

	public void addAny(String name, Any val) {
		typedBuffer.addAny(name, val);
	}

	public void addString(String name, String val) {
		typedBuffer.addString(name, val);
	}

	public void clear() {
		log.debug("JABService clear ");
		data = null;
		typedBuffer.clear();
	}

	public void delete(String name) {
		data = null;
		typedBuffer.delete(name);
	}

	public void deleteItem(String name, int itemNo) {
		typedBuffer.deleteItem(name, itemNo);
	}

	public int getOccurrenceCount(String name) {
		return typedBuffer.getOccurrenceCount(name);
	}

	public byte getByteDef(String name, byte val) {
		return typedBuffer.getByteDef(name, val);
	}

	public byte getByteItemDef(String name, int itemNo, byte val) {
		return typedBuffer.getByteItemDef(name, itemNo, val);
	}

	public byte[] getBytesDef(String name, byte[] val) {
		return typedBuffer.getBytesDef(name, val);
	}

	public byte[] getBytesItemDef(String name, int itemNo, byte[] val) {
		return typedBuffer.getBytesItemDef(name, itemNo, val);
	}

	public boolean getBooleanDef(String name, boolean val) {
		return typedBuffer.getBooleanDef(name, val);
	}

	public boolean getBooleanItemDef(String name, int itemNo, boolean val) {
		return typedBuffer.getBooleanItemDef(name, itemNo, val);
	}

	public char getCharDef(String name, char val) {
		return typedBuffer.getCharDef(name, val);
	}

	public char getCharItemDef(String name, int itemNo, char val) {
		return typedBuffer.getCharItemDef(name, itemNo, val);
	}

	public double getDoubleDef(String name, double val) {
		return typedBuffer.getDoubleDef(name, val);
	}

	public double getDoubleItemDef(String name, int itemNo, double val) {
		return typedBuffer.getDoubleItemDef(name, itemNo, val);
	}

	public float getFloatDef(String name, float val) {
		return typedBuffer.getFloatDef(name, val);
	}

	public float getFloatItemDef(String name, int itemNo, float val) {
		return typedBuffer.getFloatItemDef(name, itemNo, val);
	}

	public int getIntDef(String name, int val) {
		return typedBuffer.getIntDef(name, val);
	}

	public int getIntItemDef(String name, int itemNo, int val) {
		return typedBuffer.getIntItemDef(name, itemNo, val);
	}

	public short getShortDef(String name, short val) {
		return typedBuffer.getShortDef(name, val);
	}

	public short getShortItemDef(String name, int itemNo, short val) {
		return typedBuffer.getShortItemDef(name, itemNo, val);
	}

	public long getLongDef(String name, long val) {
		return typedBuffer.getLongDef(name, val);
	}

	public long getLongItemDef(String name, int itemNo, long val) {
		return typedBuffer.getLongItemDef(name, itemNo, val);
	}

	public String getStringDef(String name, String val) {
		return typedBuffer.getStringDef(name, val);
	}

	public String getStringItemDef(String name, int itemNo, String val) {
		return typedBuffer.getStringItemDef(name, itemNo, val);
	}

	public void setByte(String name, byte val) {
		typedBuffer.setByte(name, val);
	}

	public void setByteItem(String name, int itemNo, byte val) {
		typedBuffer.setByteItem(name, itemNo, val);
	}

	public void setBytes(String name, byte[] val) {
		data = val;
		typedBuffer.setBytes(name, val);
	}

	public void setBytesItem(String name, int itemNo, byte[] val) {
		typedBuffer.setBytesItem(name, itemNo, val);
	}

	public void setBoolean(String name, boolean val) {
		typedBuffer.setBoolean(name, val);
	}

	public void setBooleanItem(String name, int itemNo, boolean val) {
		typedBuffer.setBooleanItem(name, itemNo, val);
	}

	public void setChar(String name, char val) {
		typedBuffer.setChar(name, val);
	}

	public void setCharItem(String name, int itemNo, char val) {
		typedBuffer.setCharItem(name, itemNo, val);
	}

	public void setDouble(String name, double val) {
		typedBuffer.setDouble(name, val);
	}

	public void setDoubleItem(String name, int itemNo, double val) {
		typedBuffer.setDoubleItem(name, itemNo, val);
	}

	public void setFloat(String name, float val) {
		typedBuffer.setFloat(name, val);
	}

	public void setFloatItem(String name, int itemNo, float val) {
		typedBuffer.setFloatItem(name, itemNo, val);
	}

	public void setInt(String name, int val) {
		typedBuffer.setInt(name, val);
	}

	public void setIntItem(String name, int itemNo, int val) {
		typedBuffer.setIntItem(name, itemNo, val);
	}

	public void setShort(String name, short val) {
		typedBuffer.setShort(name, val);
	}

	public void setShortItem(String name, int itemNo, short val) {
		typedBuffer.setShortItem(name, itemNo, val);
	}

	public void setLong(String name, long val) {
		typedBuffer.setLong(name, val);
	}

	public void setLongItem(String name, int itemNo, long val) {
		typedBuffer.setLongItem(name, itemNo, val);
	}

	public void setString(String name, String val) {
		data = val.getBytes();
		typedBuffer.setString(name, val);
	}

	public void setStringItem(String name, int itemNo, String val) {
		typedBuffer.setStringItem(name, itemNo, val);
	}

	public JABMessage getResponse() {
		return result;
	}

	public String getResponseString() {
		return new String(data);
	}

	public byte[] getResponseBytes() {
		return data;
	}
}
