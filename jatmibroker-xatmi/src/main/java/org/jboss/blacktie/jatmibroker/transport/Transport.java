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
package org.jboss.blacktie.jatmibroker.transport;

import org.jboss.blacktie.jatmibroker.JAtmiBrokerException;

public interface Transport {

	/**
	 * Get the receiver
	 * 
	 * @param serviceName
	 * @return
	 * @throws JAtmiBrokerException
	 */
	public Sender getSender(String serviceName) throws JAtmiBrokerException;

	/**
	 * Create a sender to a service queue
	 * 
	 * @param replyTo
	 * @return
	 * @throws JAtmiBrokerException
	 */
	public Sender createSender(String replyTo) throws JAtmiBrokerException;

	/**
	 * Create a receiver on a service queue
	 * 
	 * @param serviceName
	 * @return
	 * @throws JAtmiBrokerException
	 */
	public Receiver createReceiver(String serviceName)
			throws JAtmiBrokerException;

	/**
	 * Create a receiver on a temporary queue.
	 * 
	 * @return
	 * @throws JAtmiBrokerException
	 */
	public Receiver createReceiver() throws JAtmiBrokerException;

	public void close();
}
