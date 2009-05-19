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

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.proxy.ServiceQueue;

public class AtmiBroker_ServiceImpl extends Thread {
	private static final Logger log = LogManager
			.getLogger(AtmiBroker_ServiceImpl.class);
	private java.lang.Object callback;
	private String serviceName;

	private AtmiBroker_CallbackConverter atmiBroker_CallbackConverter;
	private EndpointQueue serviceQueue;
	private ServiceQueue endpointQueue;
	private OrbManagement orbManagement;

	AtmiBroker_ServiceImpl(OrbManagement orbManagement, String serviceName,
			Class callback, EndpointQueue endpointQueue)
			throws InstantiationException, IllegalAccessException {
		this.serviceName = serviceName;
		this.callback = callback.newInstance();
		this.atmiBroker_CallbackConverter = this.callback
				.getCallbackConverter();
		this.serviceQueue = endpointQueue;
		this.orbManagement = orbManagement;
		start();
	}

	public void run() {
		while (true) {
			Message message = serviceQueue.receive(0);
			try {
				endpointQueue = AtmiBrokerServiceFactoryImpl.getProxy(
						orbManagement, message.replyTo);

				// TODO HANDLE CONTROL
				// THIS IS THE FIRST CALL
				AtmiBroker_Response serviceResponse = atmiBroker_CallbackConverter
						.serviceRequest(callback, serviceName, message.data,
								message.len, message.flags);
				// TODO THIS SHOULD INVOKE THE CLIENT HANDLER
				// odata.value = serviceRequest.getBytes();
				// olen.value = serviceRequest.getLength();
				endpointQueue.send("", serviceResponse.getRval(),
						serviceResponse.getRcode(), serviceResponse.getBytes(),
						serviceResponse.getLength(),
						serviceResponse.getFlags(), 0);
			} catch (Throwable t) {
				log.error("Could not service the request");
			}
		}
	}

	public String serviceName() {
		return serviceName;
	}
}
