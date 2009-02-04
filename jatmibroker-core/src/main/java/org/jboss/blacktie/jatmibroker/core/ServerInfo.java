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

public final class ServerInfo implements java.io.Serializable {
	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;

	public ServerInfo() {
	}

	public short maxChannels;
	public short maxSuppliers;
	public short maxConsumers;
	public short maxReplicas;
	public short logLevel;
	public java.lang.String securityType = "";
	public java.lang.String orbType = "";
	public java.lang.String queueSpaceName = "";
	public byte[][] serviceNames;

	public ServerInfo(AtmiBroker.ServerInfo aCorbaServerInfo) {
		this.maxChannels = aCorbaServerInfo.maxChannels;
		this.maxSuppliers = aCorbaServerInfo.maxSuppliers;
		this.maxConsumers = aCorbaServerInfo.maxConsumers;
		this.maxReplicas = aCorbaServerInfo.maxReplicas;
		this.logLevel = aCorbaServerInfo.logLevel;
		this.securityType = aCorbaServerInfo.securityType;
		this.orbType = aCorbaServerInfo.orbType;
		this.queueSpaceName = aCorbaServerInfo.queueSpaceName;
		this.serviceNames = aCorbaServerInfo.serviceNames;
	}

	public ServerInfo(short maxChannels, short maxSuppliers, short maxConsumers, short maxReplicas, String queueSpaceName) {
		this.maxChannels = maxChannels;
		this.maxSuppliers = maxSuppliers;
		this.maxConsumers = maxConsumers;
		this.maxReplicas = maxReplicas;
		this.queueSpaceName = queueSpaceName;
	}

	public AtmiBroker.ServerInfo createCorbaServerInfo() {
		AtmiBroker.ServerInfo aCorbaServerInfo = new AtmiBroker.ServerInfo();
		aCorbaServerInfo.maxChannels = this.maxChannels;
		aCorbaServerInfo.maxSuppliers = this.maxSuppliers;
		aCorbaServerInfo.maxConsumers = this.maxConsumers;
		aCorbaServerInfo.maxReplicas = this.maxReplicas;
		aCorbaServerInfo.logLevel = this.logLevel;
		aCorbaServerInfo.securityType = this.securityType;
		aCorbaServerInfo.orbType = this.orbType;
		aCorbaServerInfo.queueSpaceName = this.queueSpaceName;
		aCorbaServerInfo.serviceNames = this.serviceNames;

		return aCorbaServerInfo;
	}

	public short getMaxChannels() {
		return maxChannels;
	}

	public void setMaxChannels(short maxChannels) {
		this.maxChannels = maxChannels;
	}

	public short getMaxSuppliers() {
		return maxSuppliers;
	}

	public void setMaxSuppliers(short maxSuppliers) {
		this.maxSuppliers = maxSuppliers;
	}

	public short getMaxConsumers() {
		return maxConsumers;
	}

	public void setMaxConsumers(short maxConsumers) {
		this.maxConsumers = maxConsumers;
	}

	public short getMaxReplicas() {
		return maxReplicas;
	}

	public void setMaxReplicas(short maxReplicas) {
		this.maxReplicas = maxReplicas;
	}

	public java.lang.String getOrbType() {
		return orbType;
	}

	public void setOrbType(java.lang.String orbType) {
		this.orbType = orbType;
	}

	public java.lang.String getQueueSpaceName() {
		return queueSpaceName;
	}

	public void setQueueSpaceName(java.lang.String queueSpaceName) {
		this.queueSpaceName = queueSpaceName;
	}
}
