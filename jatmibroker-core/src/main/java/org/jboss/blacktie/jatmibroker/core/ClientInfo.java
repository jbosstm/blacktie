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

public final class ClientInfo implements java.io.Serializable {
	public ClientInfo() {
	};

	public java.lang.String callback_ior;
	public java.lang.String user_id;
	public java.lang.String user_password;
	public int client_id;
	public int maxChannels;
	public int maxSuppliers;
	public int maxConsumers;
	public int maxReplicas;
	public short logLevel;

	public ClientInfo(int maxChannels, int maxSuppliers, int maxConsumers, int maxReplicas) {
		this.maxChannels = maxChannels;
		this.maxSuppliers = maxSuppliers;
		this.maxConsumers = maxConsumers;
		this.maxReplicas = maxReplicas;
	}

	public ClientInfo(AtmiBroker.ClientInfo aCorbaClientInfo) {
		this.callback_ior = aCorbaClientInfo.callback_ior;
		this.user_id = aCorbaClientInfo.user_id;
		this.user_password = aCorbaClientInfo.user_password;
		this.client_id = aCorbaClientInfo.client_id;
		this.maxChannels = aCorbaClientInfo.maxChannels;
		this.maxSuppliers = aCorbaClientInfo.maxSuppliers;
		this.maxConsumers = aCorbaClientInfo.maxConsumers;
		this.maxReplicas = aCorbaClientInfo.maxReplicas;
		this.logLevel = aCorbaClientInfo.logLevel;
	}

	public AtmiBroker.ClientInfo createCorbaClientInfo() {
		AtmiBroker.ClientInfo aCorbaClientInfo = new AtmiBroker.ClientInfo();

		aCorbaClientInfo.callback_ior = this.callback_ior;
		aCorbaClientInfo.user_id = this.user_id;
		aCorbaClientInfo.user_password = this.user_password;
		aCorbaClientInfo.client_id = this.client_id;
		aCorbaClientInfo.maxChannels = (short) this.maxChannels;
		aCorbaClientInfo.maxSuppliers = (short) this.maxSuppliers;
		aCorbaClientInfo.maxConsumers = (short) this.maxConsumers;
		aCorbaClientInfo.maxReplicas = (short) this.maxReplicas;
		aCorbaClientInfo.logLevel = this.logLevel;

		return aCorbaClientInfo;
	}

	public short getMaxChannels() {
		return (short) maxChannels;
	}

	public void setMaxChannels(short maxChannels) {
		this.maxChannels = maxChannels;
	}

	public short getMaxSuppliers() {
		return (short) maxSuppliers;
	}

	public void setMaxSuppliers(short maxSuppliers) {
		this.maxSuppliers = maxSuppliers;
	}

	public short getMaxConsumers() {
		return (short) maxConsumers;
	}

	public void setMaxConsumers(short maxConsumers) {
		this.maxConsumers = maxConsumers;
	}

	public short getMaxReplicas() {
		return (short) maxReplicas;
	}

	public void setMaxReplicas(short maxReplicas) {
		this.maxReplicas = maxReplicas;
	}
}
