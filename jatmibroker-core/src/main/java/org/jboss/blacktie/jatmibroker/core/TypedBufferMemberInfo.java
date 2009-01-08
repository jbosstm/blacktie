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
package org.jboss.blacktie.jatmibroker.core;

import java.io.Serializable;

public class TypedBufferMemberInfo implements Serializable {
	protected String primitiveDataTypeName;
	protected String typedBufferName;
	protected int fieldNumber;
	protected String fieldName;
	protected int fieldSize;
	protected int sequenceSize;
	protected String flag;
	protected String initialValue;
	protected String value;
	private String name;

	public TypedBufferMemberInfo(AtmiBroker.TypedBufferMember aCorbaTypedBufferMember) {
		name = aCorbaTypedBufferMember.name;
		value = aCorbaTypedBufferMember.value;
		primitiveDataTypeName = aCorbaTypedBufferMember.type;
		typedBufferName = aCorbaTypedBufferMember.name;
		fieldNumber = aCorbaTypedBufferMember.number;
		fieldName = aCorbaTypedBufferMember.fieldName;
		fieldSize = aCorbaTypedBufferMember.value.length();
		sequenceSize = aCorbaTypedBufferMember.seqSize;
		flag = String.valueOf(aCorbaTypedBufferMember.flag);
		initialValue = aCorbaTypedBufferMember.initialValue;
		value = aCorbaTypedBufferMember.value;
	}

	public String getPrimitiveDataTypeName() {
		return primitiveDataTypeName;
	}

	public String getTypedBufferName() {
		return typedBufferName;
	}

	public String getParentName() {
		return typedBufferName;
	}

	public int getFieldNumber() {
		return fieldNumber;
	}

	public String getFieldName() {
		return fieldName;
	}

	public int getFieldSize() {
		return fieldSize;
	}

	public int getSequenceSize() {
		return sequenceSize;
	}

	public String getFlag() {
		return flag;
	}

	public String getInitialValue() {
		return initialValue;
	}

	public String getValue() {
		return value;
	}

	public void setPrimitiveDataTypeName(String aName) {
		primitiveDataTypeName = aName;
	}

	public void setTypedBufferName(String aTypedBufferName) {
		typedBufferName = aTypedBufferName;
	}

	public void setFieldNumber(int aFieldNumber) {
		fieldNumber = aFieldNumber;
	}

	public void setFieldName(String aName) {
		fieldName = aName;
	}

	public void setFieldSize(int aSize) {
		fieldSize = aSize;
	}

	public void setSequenceSize(int aSequenceSize) {
		sequenceSize = aSequenceSize;
	}

	public void setFlag(String aFlag) {
		flag = aFlag;
	}

	public void setInitialValue(String aInitialValue) {
		initialValue = aInitialValue;
	}

	public void setValue(String aValue) {
		value = aValue;
	}

	public AtmiBroker.TypedBufferMember createCorbaTypedBufferMember() {
		AtmiBroker.TypedBufferMember aCorbaTypedBufferMember = new AtmiBroker.TypedBufferMember();

		aCorbaTypedBufferMember.name = name;
		aCorbaTypedBufferMember.value = value;
		aCorbaTypedBufferMember.type = primitiveDataTypeName;
		aCorbaTypedBufferMember.name = typedBufferName;
		aCorbaTypedBufferMember.number = (short) fieldNumber;
		aCorbaTypedBufferMember.fieldName = fieldName;
		aCorbaTypedBufferMember.seqSize = (short) sequenceSize;
		aCorbaTypedBufferMember.flag = flag;
		aCorbaTypedBufferMember.initialValue = initialValue;
		aCorbaTypedBufferMember.value = value;

		return aCorbaTypedBufferMember;
	}
}
