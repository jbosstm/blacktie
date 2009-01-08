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

public class TypedBufferInfo implements Serializable {

	protected String trDomainName;
	protected String xmlSchemaName;
	protected TypedBufferMemberInfo[] typedBufferMembers;
	private String name;

	public TypedBufferInfo(AtmiBroker.TypedBuffer aTypedBuffer) {
		name = aTypedBuffer.name;
		trDomainName = "";
		xmlSchemaName = aTypedBuffer.schemaName;
		typedBufferMembers = new TypedBufferMemberInfo[aTypedBuffer.members.length];
		for (int i = 0; i < typedBufferMembers.length; i++) {
			typedBufferMembers[i] = new TypedBufferMemberInfo(aTypedBuffer.members[i]);
		}
	}

	public String getTrDomainName() {
		return trDomainName;
	}

	public String getParentName() {
		return trDomainName;
	}

	public String getXmlSchemaName() {
		return xmlSchemaName;
	}

	public TypedBufferMemberInfo[] getTypedBufferMembers() {
		return typedBufferMembers;
	}

	public void setTrDomainName(String aTrDomainName) {
		trDomainName = aTrDomainName;
	}

	public void setXmlSchemaName(String aXmlSchemaName) {
		xmlSchemaName = aXmlSchemaName;
	}

	public void setTypedBufferMembers(TypedBufferMemberInfo[] aTypedBufferMembers) {
		typedBufferMembers = aTypedBufferMembers;
	}

	public AtmiBroker.TypedBuffer createCorbaTypedBuffer() {
		AtmiBroker.TypedBuffer aCorbaTypedBuffer = new AtmiBroker.TypedBuffer();

		aCorbaTypedBuffer.name = name;
		aCorbaTypedBuffer.schemaName = xmlSchemaName;
		typedBufferMembers = new TypedBufferMemberInfo[typedBufferMembers.length];
		for (int i = 0; i < typedBufferMembers.length; i++) {
			aCorbaTypedBuffer.members[i] = typedBufferMembers[i].createCorbaTypedBufferMember();
		}

		return aCorbaTypedBuffer;
	}

}
