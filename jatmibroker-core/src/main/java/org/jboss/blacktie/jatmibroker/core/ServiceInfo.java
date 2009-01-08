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

public final class ServiceInfo implements java.io.Serializable {
	public short maxSize;
	public short minSize;
	public short minAvailableSize;
	public short inUse;
	public short available;
	public short logLevel;
	public String securityType = "";
	private String serviceName;

	public ServiceInfo(AtmiBroker.ServiceInfo aCorbaServiceInfo) {
		this.serviceName = aCorbaServiceInfo.serviceName;
		this.maxSize = aCorbaServiceInfo.maxSize;
		this.minSize = aCorbaServiceInfo.minSize;
		this.minAvailableSize = aCorbaServiceInfo.minAvailableSize;
		this.inUse = aCorbaServiceInfo.inUse;
		this.available = aCorbaServiceInfo.available;
		this.logLevel = aCorbaServiceInfo.logLevel;
		this.securityType = aCorbaServiceInfo.securityType;
	}

	public AtmiBroker.ServiceInfo createCorbaServiceInfo() {
		AtmiBroker.ServiceInfo aCorbaServiceInfo = new AtmiBroker.ServiceInfo();
		aCorbaServiceInfo.serviceName = this.serviceName;
		aCorbaServiceInfo.maxSize = this.maxSize;
		aCorbaServiceInfo.minSize = this.minSize;
		aCorbaServiceInfo.minAvailableSize = this.minAvailableSize;
		aCorbaServiceInfo.inUse = this.inUse;
		aCorbaServiceInfo.available = this.available;
		aCorbaServiceInfo.logLevel = this.logLevel;
		aCorbaServiceInfo.securityType = this.securityType;

		return aCorbaServiceInfo;
	}

	public String getServiceName() {
		return serviceName;
	}

	public void setServiceName(String serviceName) {
		this.serviceName = serviceName;
	}

	public short getMaxSize() {
		return maxSize;
	}

	public void setMaxSize(short maxSize) {
		this.maxSize = maxSize;
	}

	public short getMinSize() {
		return minSize;
	}

	public void setMinSize(short minSize) {
		this.minSize = minSize;
	}

	public short getMinAvailableSize() {
		return minAvailableSize;
	}

	public void setMinAvailableSize(short minAvailableSize) {
		this.minAvailableSize = minAvailableSize;
	}
}
