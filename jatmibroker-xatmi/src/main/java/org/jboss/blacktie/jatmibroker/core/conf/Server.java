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
package org.jboss.blacktie.jatmibroker.core.conf;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

public class Server {

	private List<Machine> machines = new ArrayList<Machine>();
	private String name;

	public Server(String name) {
		this.name = name;
	}

	public void addMachine(Machine machine) {
		machines.add(machine);
	}

	public List<Machine> getMachines() {
		return machines;
	}

	public void setMachines(List<Machine> machines) {
		this.machines = machines;
	}

	public boolean isHostedLocally() throws UnknownHostException {
		boolean toReturn = false;
		String hostname = InetAddress.getLocalHost().getHostName();
		Iterator<Machine> iterator = machines.iterator();
		while (iterator.hasNext()) {
			if (iterator.next().getHostname().equals(hostname)) {
				toReturn = true;
				break;
			}
		}
		return toReturn;
	}

	public Machine getLocalMachine() throws UnknownHostException {
		Machine toReturn = null;
		String hostname = InetAddress.getLocalHost().getHostName();
		Iterator<Machine> iterator = machines.iterator();
		while (iterator.hasNext()) {
			Machine next = iterator.next();
			if (next.getHostname().equals(hostname)) {
				toReturn = next;
				break;
			}
		}
		return toReturn;
	}

	public String getName() {
		return name;
	}
}
