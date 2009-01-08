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
package org.jboss.blacktie.jatmibroker.core;

public class RunServer extends Thread {
	private static RunServer instance;

	native void runServer();

	static {
		System.loadLibrary("testsuite");
	}

	public static synchronized void startInstance() throws InterruptedException {
		if (instance == null) {
			instance = new RunServer();
			instance.start();
			Thread.currentThread().sleep(1000);
		}
	}

	public static synchronized void stopInstance() {
		instance.stop();
	}

	public void run() {
		runServer();
	}
}
