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
package org.jboss.blacktie.jatmibroker.jab;

public class JABMessage {// implements Message {
// private static final Logger log = LogManager.getLogger(JABMessage.class);
// private TypedBuffer typedBuffer;
// private List<AtmiBroker.TypedBufferMember> members = new
// ArrayList<AtmiBroker.TypedBufferMember>();
// private int size;
//
// public JABMessage(TypedBuffer typedBuffer, int size) {
// this.typedBuffer = typedBuffer;
// this.size = size;
// }
//
// public JABMessage() {
// typedBuffer = new TypedBuffer();
// }
//
// public void addByte(String name, byte val) {
// log.debug("JABTypedBuffer addByte name: " + name + " val: " + val);
// TypedBufferMember aTypedBufferMember = new TypedBufferMember();
// aTypedBufferMember.name = name;
// aTypedBufferMember.value = String.valueOf(val);
// members.add(aTypedBufferMember);
// }
//
// public void addBytes(String name, byte[] val) {
// log.debug("JABTypedBuffer addBytes name: " + name + " val: " + val);
// TypedBufferMember aTypedBufferMember = new TypedBufferMember();
// aTypedBufferMember.name = name;
// aTypedBufferMember.value = String.valueOf(val);
// members.add(aTypedBufferMember);
// }
//
// public void addBoolean(String name, boolean val) {
// log.debug("JABTypedBuffer addBoolean name: " + name + " val: " + val);
// TypedBufferMember aTypedBufferMember = new TypedBufferMember();
// aTypedBufferMember.name = name;
// aTypedBufferMember.value = String.valueOf(val);
// members.add(aTypedBufferMember);
// }
//
// public void addChar(String name, char val) {
// log.debug("JABTypedBuffer addChar name: " + name + " val: " + val);
// TypedBufferMember aTypedBufferMember = new TypedBufferMember();
// aTypedBufferMember.name = name;
// aTypedBufferMember.value = String.valueOf(val);
// members.add(aTypedBufferMember);
// }
//
// public void addDouble(String name, double val) {
// log.debug("JABTypedBuffer addDouble name: " + name + " val: " + val);
// TypedBufferMember aTypedBufferMember = new TypedBufferMember();
// aTypedBufferMember.name = name;
// aTypedBufferMember.value = String.valueOf(val);
// members.add(aTypedBufferMember);
// }
//
// public void addFloat(String name, float val) {
// log.debug("JABTypedBuffer addFloat name: " + name + " val: " + val);
// TypedBufferMember aTypedBufferMember = new TypedBufferMember();
// aTypedBufferMember.name = name;
// aTypedBufferMember.value = String.valueOf(val);
// members.add(aTypedBufferMember);
// }
//
// public void addInt(String name, int val) {
// log.debug("JABTypedBuffer addInt name: " + name + " val: " + val);
// TypedBufferMember aTypedBufferMember = new TypedBufferMember();
// aTypedBufferMember.name = name;
// aTypedBufferMember.value = String.valueOf(val);
// members.add(aTypedBufferMember);
// }
//
// public void addShort(String name, short val) {
// log.debug("JABTypedBuffer addShort name: " + name + " val: " + val);
// TypedBufferMember aTypedBufferMember = new TypedBufferMember();
// aTypedBufferMember.name = name;
// aTypedBufferMember.value = String.valueOf(val);
// members.add(aTypedBufferMember);
// }
//
// public void addLong(String name, long val) {
// log.debug("JABTypedBuffer addLong name: " + name + " val: " + val);
// TypedBufferMember aTypedBufferMember = new TypedBufferMember();
// aTypedBufferMember.name = name;
// aTypedBufferMember.value = String.valueOf(val);
// members.add(aTypedBufferMember);
// }
//
// public void addAny(String name, Any val) {
// log.debug("JABTypedBuffer addAny name: " + name + " val: " + val);
// TypedBufferMember aTypedBufferMember = new TypedBufferMember();
// aTypedBufferMember.name = name;
// aTypedBufferMember.value = val.extract_string();
// members.add(aTypedBufferMember);
// }
//
// public void addString(String name, String val) {
// log.debug("JABTypedBuffer addString name: " + name + " val: " + val);
// TypedBufferMember aTypedBufferMember = new TypedBufferMember();
// aTypedBufferMember.name = name;
// aTypedBufferMember.value = val;
// members.add(aTypedBufferMember);
// size = val.length();
// }
//
// public void clear() {
// log.debug("JABTypedBuffer clear");
// members.clear();
// }
//
// public void delete(String name) {
// log.debug("JABTypedBuffer delete name: " + name);
// Iterator<TypedBufferMember> iterator = members.iterator();
// while (iterator.hasNext()) {
// TypedBufferMember next = iterator.next();
// if (next.name.equals(name)) {
// iterator.remove();
// log.debug("JABTypedBuffer deleted name: " + name);
// }
// }
// }
//
// public void deleteItem(String name, int itemNo) {
// log.debug("JABTypedBuffer deleteItem name: " + name + " occurence:" +
// itemNo);
// Iterator<TypedBufferMember> iterator = members.iterator();
// int count = 0;
// while (iterator.hasNext()) {
// TypedBufferMember next = iterator.next();
// if (next.name.equals(name) && count == itemNo) {
// iterator.remove();
// } else {
// count++;
// }
// }
// }
//
// public int getOccurrenceCount(String name) {
// log.debug("JABTypedBuffer delete name: " + name);
//
// int i = 0;
// int count = 0;
// for (i = 0; i < members.size(); i++) {
// if (name.equals(members.get(i).name)) {
// count++;
// }
// }
// return count;
// }
//
// public byte getByteDef(String name, byte val) {
// log.debug("getByteDef name: " + name);
// return getByteItemDef(name, 1, val);
// }
//
// public byte getByteItemDef(String name, int itemNo, byte val) {
// log.debug("JABTypedBuffer getByteItemDef name: " + name + " occurence:" +
// itemNo);
// int i = 0;
// int count = 0;
// for (i = 0; i < members.size(); i++) {
// if (name.equals(members.get(i).name)) {
// count++;
// if (count == itemNo) {
// log.debug("found name: " + name);
// byte aByte = Byte.valueOf(members.get(i).value).byteValue();
// return aByte;
// }
// }
// }
// return val;
// }
//
// public byte[] getBytesDef(String name, byte[] val) {
// log.debug("getBytesDef name: " + name);
// return getBytesItemDef(name, 1, val);
// }
//
// public byte[] getBytesItemDef(String name, int itemNo, byte[] val) {
// log.debug("JABTypedBuffer getBytesItemDef name: " + name + " occurence:" +
// itemNo);
// int i = 0;
// int count = 0;
// for (i = 0; i < members.size(); i++) {
// if (name.equals(members.get(i).name)) {
// count++;
// if (count == itemNo) {
// log.debug("found name: " + name);
// byte[] aBytes = members.get(i).value.getBytes();
// return aBytes;
// }
// }
// }
// return val;
// }
//
// public boolean getBooleanDef(String name, boolean val) {
// log.debug("JABTypedBuffer getBooleanDef name: " + name);
// return getBooleanItemDef(name, 1, val);
// }
//
// public boolean getBooleanItemDef(String name, int itemNo, boolean val) {
// log.debug("JABTypedBuffer getBooleanItemDef name: " + name + " occurence:" +
// itemNo);
// int i = 0;
// int count = 0;
// for (i = 0; i < members.size(); i++) {
// if (name.equals(members.get(i).name)) {
// count++;
// if (count == itemNo) {
// log.debug("found name: " + name);
// boolean aBoolean = Boolean.valueOf(members.get(i).value).booleanValue();
// return aBoolean;
// }
// }
// }
// return val;
// }
//
// public char getCharDef(String name, char val) {
// log.debug("JABTypedBuffer getCharDef name: " + name);
// return getCharItemDef(name, 1, val);
// }
//
// public char getCharItemDef(String name, int itemNo, char val) {
// log.debug("JABTypedBuffer getCharItemDef name: " + name + " occurence:" +
// itemNo);
// int i = 0;
// int count = 0;
// for (i = 0; i < members.size(); i++) {
// if (name.equals(members.get(i).name)) {
// count++;
// if (count == itemNo) {
// log.debug("found name: " + name);
// char aChar = members.get(i).value.charAt(0);
// return aChar;
// }
// }
// }
// return val;
// }
//
// public double getDoubleDef(String name, double val) {
// log.debug("JABTypedBuffer getDoubleDef name: " + name);
// return getDoubleItemDef(name, 1, val);
// }
//
// public double getDoubleItemDef(String name, int itemNo, double val) {
// log.debug("JABTypedBuffer getDoubleItemDef name: " + name + " occurence:" +
// itemNo);
// int i = 0;
// int count = 0;
// for (i = 0; i < members.size(); i++) {
// if (name.equals(members.get(i).name)) {
// count++;
// if (count == itemNo) {
// log.debug("found name: " + name);
// double aDouble = Double.valueOf(members.get(i).value).doubleValue();
// return aDouble;
// }
// }
// }
// return val;
// }
//
// public float getFloatDef(String name, float val) {
// log.debug("JABTypedBuffer getFloatDef name: " + name);
// return getFloatItemDef(name, 1, val);
// }
//
// public float getFloatItemDef(String name, int itemNo, float val) {
// log.debug("JABTypedBuffer getFloatItemDef name: " + name + " occurence:" +
// itemNo);
// int i = 0;
// int count = 0;
// for (i = 0; i < members.size(); i++) {
// if (name.equals(members.get(i).name)) {
// count++;
// if (count == itemNo) {
// log.debug("found name: " + name);
// float aFloat = Float.valueOf(members.get(i).value).floatValue();
// return aFloat;
// }
// }
// }
// return val;
// }
//
// public int getIntDef(String name, int val) {
// log.debug("JABTypedBuffer getFloatDef name: " + name);
// return getIntItemDef(name, 1, val);
// }
//
// public int getIntItemDef(String name, int itemNo, int val) {
// log.debug("JABTypedBuffer getIntItemDef name: " + name + " occurence:" +
// itemNo);
// int i = 0;
// int count = 0;
// for (i = 0; i < members.size(); i++) {
// if (name.equals(members.get(i).name)) {
// count++;
// if (count == itemNo) {
// log.debug("found name: " + name);
// int aInt = Integer.valueOf(members.get(i).value).intValue();
// return aInt;
// }
// }
// }
// return val;
// }
//
// public short getShortDef(String name, short val) {
// log.debug("JABTypedBuffer getShortDef name: " + name);
// return getShortItemDef(name, 1, val);
// }
//
// public short getShortItemDef(String name, int itemNo, short val) {
// log.debug("JABTypedBuffer getShortItemDef name: " + name + " occurence:" +
// itemNo);
// int i = 0;
// int count = 0;
// for (i = 0; i < members.size(); i++) {
// if (name.equals(members.get(i).name)) {
// count++;
// if (count == itemNo) {
// log.debug("found name: " + name);
// short aShort = Short.valueOf(members.get(i).value).shortValue();
// return aShort;
// }
// }
// }
// return val;
// }
//
// public long getLongDef(String name, long val) {
// log.debug("JABTypedBuffer getLongDef name: " + name);
// return getLongItemDef(name, 1, val);
// }
//
// public long getLongItemDef(String name, int itemNo, long val) {
// log.debug("JABTypedBuffer getLongItemDef name: " + name + " occurence:" +
// itemNo);
// int i = 0;
// int count = 0;
// for (i = 0; i < members.size(); i++) {
// if (name.equals(members.get(i).name)) {
// count++;
// if (count == itemNo) {
// log.debug("found name: " + name);
// long aLong = Long.valueOf(members.get(i).value).longValue();
// return aLong;
// }
// }
// }
// return val;
// }
//
// public String getStringDef(String name, String val) {
// log.debug("JABTypedBuffer getStringDef name: " + name);
// return getStringItemDef(name, 1, val);
// }
//
// public String getStringItemDef(String name, int itemNo, String val) {
// log.debug("JABTypedBuffer getStringItemDef name: " + name + " occurence:" +
// itemNo);
// int i = 0;
// int count = 0;
// for (i = 0; i < members.size(); i++) {
// if (name.equals(members.get(i).name)) {
// count++;
// if (count == itemNo) {
// log.debug("found name: " + name);
// String aString = members.get(i).value;
// return aString;
// }
// }
// }
// return val;
// }
//
// public void setByte(String name, byte val) {
// log.debug("JABTypedBuffer setByte name: " + name);
// getByteItemDef(name, 1, val);
// }
//
// public void setByteItem(String name, int itemNo, byte val) {
// log.debug("JABTypedBuffer setByteItem name: " + name + " occurence:" +
// itemNo);
// int i = 0;
// int count = 0;
// for (i = 0; i < members.size(); i++) {
// if (name.equals(members.get(i).name)) {
// count++;
// if (count == itemNo) {
// log.debug("found name: " + name);
// members.get(i).value = String.valueOf(val);
// }
// }
// }
// }
//
// public void setBytes(String name, byte[] val) {
// log.debug("setBytes name: " + name);
// setBytesItem(name, 1, val);
// }
//
// public void setBytesItem(String name, int itemNo, byte[] val) {
// log.debug("JABTypedBuffer setBytesItem name: " + name + " occurence:" +
// itemNo);
// int i = 0;
// int count = 0;
// for (i = 0; i < members.size(); i++) {
// if (name.equals(members.get(i).name)) {
// count++;
// if (count == itemNo) {
// log.debug("found name: " + name);
// members.get(i).value = String.valueOf(val);
// }
// }
// }
// }
//
// public void setBoolean(String name, boolean val) {
// log.debug("JABTypedBuffer setBoolean name: " + name);
// getBooleanItemDef(name, 1, val);
// }
//
// public void setBooleanItem(String name, int itemNo, boolean val) {
// log.debug("JABTypedBuffer setBooleanItem name: " + name + " occurence:" +
// itemNo);
// int i = 0;
// int count = 0;
// for (i = 0; i < members.size(); i++) {
// if (name.equals(members.get(i).name)) {
// count++;
// if (count == itemNo) {
// log.debug("found name: " + name);
// members.get(i).value = String.valueOf(val);
// }
// }
// }
// }
//
// public void setChar(String name, char val) {
// log.debug("JABTypedBuffer setChar name: " + name);
// setCharItem(name, 1, val);
// }
//
// public void setCharItem(String name, int itemNo, char val) {
// log.debug("JABTypedBuffer setCharItem name: " + name + " occurence:" +
// itemNo);
// int i = 0;
// int count = 0;
// for (i = 0; i < members.size(); i++) {
// if (name.equals(members.get(i).name)) {
// count++;
// if (count == itemNo) {
// log.debug("found name: " + name);
// members.get(i).value = String.valueOf(val);
// }
// }
// }
// }
//
// public void setDouble(String name, double val) {
// log.debug("JABTypedBuffer setDouble name: " + name);
// setDoubleItem(name, 1, val);
// }
//
// public void setDoubleItem(String name, int itemNo, double val) {
// log.debug("JABTypedBuffer setDoubleItem name: " + name + " occurence:" +
// itemNo);
// int i = 0;
// int count = 0;
// for (i = 0; i < members.size(); i++) {
// if (name.equals(members.get(i).name)) {
// count++;
// if (count == itemNo) {
// log.debug("found name: " + name);
// members.get(i).value = String.valueOf(val);
// }
// }
// }
// }
//
// public void setFloat(String name, float val) {
// log.debug("JABTypedBuffer setFloat name: " + name);
// setFloatItem(name, 1, val);
// }
//
// public void setFloatItem(String name, int itemNo, float val) {
// log.debug("JABTypedBuffer setFloatItem name: " + name + " occurence:" +
// itemNo);
// int i = 0;
// int count = 0;
// for (i = 0; i < members.size(); i++) {
// if (name.equals(members.get(i).name)) {
// count++;
// if (count == itemNo) {
// log.debug("found name: " + name);
// members.get(i).value = String.valueOf(val);
// }
// }
// }
// }
//
// public void setInt(String name, int val) {
// log.debug("JABTypedBuffer setFloat name: " + name);
// setIntItem(name, 1, val);
// }
//
// public void setIntItem(String name, int itemNo, int val) {
// log.debug("JABTypedBuffer setIntItem name: " + name + " occurence:" +
// itemNo);
// int i = 0;
// int count = 0;
// for (i = 0; i < members.size(); i++) {
// if (name.equals(members.get(i).name)) {
// count++;
// if (count == itemNo) {
// log.debug("found name: " + name);
// members.get(i).value = String.valueOf(val);
// }
// }
// }
// }
//
// public void setShort(String name, short val) {
// log.debug("JABTypedBuffer setShort name: " + name);
// setShortItem(name, 1, val);
// }
//
// public void setShortItem(String name, int itemNo, short val) {
// log.debug("JABTypedBuffer setShortItem name: " + name + " occurence:" +
// itemNo);
// int i = 0;
// int count = 0;
// for (i = 0; i < members.size(); i++) {
// if (name.equals(members.get(i).name)) {
// count++;
// if (count == itemNo) {
// log.debug("found name: " + name);
// members.get(i).value = String.valueOf(val);
// }
// }
// }
// }
//
// public void setLong(String name, long val) {
// log.debug("JABTypedBuffer setLong name: " + name);
// setLongItem(name, 1, val);
// }
//
// public void setLongItem(String name, int itemNo, long val) {
// log.debug("JABTypedBuffer setLongItem name: " + name + " occurence:" +
// itemNo);
// int i = 0;
// int count = 0;
// for (i = 0; i < members.size(); i++) {
// if (name.equals(members.get(i).name)) {
// count++;
// if (count == itemNo) {
// log.debug("found name: " + name);
// members.get(i).value = String.valueOf(val);
// }
// }
// }
// }
//
// public void setString(String name, String val) {
// log.debug("JABTypedBuffer setString name: " + name);
// setStringItem(name, 1, val);
// }
//
// public void setStringItem(String name, int itemNo, String val) {
// log.debug("JABTypedBuffer setStringItem name: " + name + " occurence:" +
// itemNo);
// int i = 0;
// int count = 0;
// for (i = 0; i < members.size(); i++) {
// if (name.equals(members.get(i).name)) {
// count++;
// if (count == itemNo) {
// log.debug("found name: " + name);
// members.get(i).value = val;
// }
// }
// }
// }
//
// int size() {
// return size;
// }
//
// TypedBuffer getTypedBuffer() {
// return typedBuffer;
// }
}
