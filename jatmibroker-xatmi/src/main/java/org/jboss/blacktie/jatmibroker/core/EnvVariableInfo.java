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
package org.jboss.blacktie.jatmibroker.core;

public final class EnvVariableInfo implements java.io.Serializable {
	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;

	public EnvVariableInfo() {
	}

	public java.lang.String name = "";
	public java.lang.String value = "";

	public EnvVariableInfo(AtmiBroker.EnvVariableInfo aEnvVariableInfo) {
		this.name = aEnvVariableInfo.name;
		this.value = aEnvVariableInfo.value;
	}

	public EnvVariableInfo(String name2, String value2) {
		this.name = name2;
		this.value = value2;
	}

	public AtmiBroker.EnvVariableInfo createCorbaEnvVariableInfo() {
		AtmiBroker.EnvVariableInfo aEnvVariableInfo = new AtmiBroker.EnvVariableInfo();
		aEnvVariableInfo.name = this.name;
		aEnvVariableInfo.value = this.value;

		return aEnvVariableInfo;
	}

	public java.lang.String getValue() {
		return value;
	}

	public void setValue(java.lang.String value) {
		this.value = value;
	}

	public java.lang.String getName() {
		return name;
	}

	public void setName(java.lang.String name) {
		this.name = name;
	}
}
