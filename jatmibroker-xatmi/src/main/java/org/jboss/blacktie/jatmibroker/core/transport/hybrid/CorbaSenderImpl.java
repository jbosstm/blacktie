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
package org.jboss.blacktie.jatmibroker.core.transport.hybrid;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.transport.Sender;

import AtmiBroker.EndpointQueue;
import AtmiBroker.EndpointQueueHelper;

public class CorbaSenderImpl implements Sender {
	private static final Logger log = LogManager
			.getLogger(CorbaSenderImpl.class);
	private EndpointQueue queue;
	private String name;

	CorbaSenderImpl(org.omg.CORBA.Object serviceFactoryObject, String name) {
		this.queue = EndpointQueueHelper.narrow(serviceFactoryObject);
		this.name = name;
		log.debug("Corba sender for: " + name + " created");
	}

	public void send(Object replyTo, short rval, int rcode, byte[] data,
			int len, int correlationId, int flags, String type, String subtype) {
		log.debug("Sending the message");
		String toReplyTo = (String) replyTo;
		if (toReplyTo == null) {
			log.trace("Reply to set as null");
			toReplyTo = "";
		}
		if (type == null) {
			log.trace("Type set as null");
			type = "";
		}
		if (subtype == null) {
			log.trace("Subtype set as null");
			subtype = "";
		}
		len = len + 1;
		byte[] toSend = new byte[len];
		System.arraycopy(data, 0, toSend, 0, len - 1);
		queue.send(toReplyTo, rval, rcode, toSend, len, correlationId, flags,
				type, subtype);
		log.debug("Sent the message");
	}

	public void close() {
		log.debug("Close called");
		// TODO
	}

	public Object getSendTo() {
		return name;
	}
}
