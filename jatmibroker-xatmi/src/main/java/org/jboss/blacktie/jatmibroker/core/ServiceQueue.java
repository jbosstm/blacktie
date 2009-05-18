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

import java.util.ArrayList;
import java.util.List;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

import AtmiBroker.ServiceInfo;

public class ServiceQueue {
	private static final Logger log = LogManager.getLogger(ServiceQueue.class);
	private String serviceName;
	private List<Runnable> servantCache = new ArrayList<Runnable>();
	private EndpointQueue endpointQueue;

	ServiceQueue(String serviceName, int servantCacheSize, Class atmiBrokerCallback, AtmiBroker_CallbackConverter atmiBroker_CallbackConverter) throws JAtmiBrokerException, InstantiationException, IllegalAccessException {
		this.serviceName = serviceName;
		this.endpointQueue = new EndpointQueue(serviceName);

		for (int i = 0; i < servantCacheSize; i++) {
			servantCache.add(new AtmiBroker_ServiceImpl(serviceName, atmiBrokerCallback, atmiBroker_CallbackConverter, endpointQueue));
		}
	}

	public ServiceInfo get_service_info() {
		log.error("NO-OP get_service_info");
		ServiceInfo serviceInfo = new ServiceInfo();
		serviceInfo.poolSize = (short) servantCache.size();
		serviceInfo.securityType = "";
		serviceInfo.serviceName = serviceName;
		return serviceInfo;
	}

	public void close() {
		endpointQueue.disconnect();
	}
}
