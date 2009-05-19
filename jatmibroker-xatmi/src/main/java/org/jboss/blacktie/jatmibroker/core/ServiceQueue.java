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

public class ServiceQueue {
	private static final Logger log = LogManager.getLogger(ServiceQueue.class);
	private EndpointQueue endpointQueue;
	private List<Runnable> servantCache = new ArrayList<Runnable>();

	ServiceQueue(OrbManagement orbManagement, String serviceName,
			int servantCacheSize, Class atmiBrokerCallback)
			throws JAtmiBrokerException, InstantiationException,
			IllegalAccessException {
		this.endpointQueue = new EndpointQueue(orbManagement, serviceName);

		for (int i = 0; i < servantCacheSize; i++) {
			servantCache.add(new AtmiBroker_ServiceImpl(orbManagement,
					serviceName, atmiBrokerCallback, endpointQueue));
		}
	}

	public void close() {
		endpointQueue.close();
		servantCache.clear();
	}
}
