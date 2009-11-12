/*
 * JBoss, Home of Professional Open Source
 * Copyright 2009, Red Hat, Inc., and others contributors as indicated
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
package org.jboss.blacktie.jatmibroker.xatmi;

import java.io.*;
import java.util.Map;
import junit.framework.TestCase;
import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

public class CSControl
{
	private static final Logger log = LogManager.getLogger(CSControl.class);

	private TestProcess server;
	private TestProcess client;
	private TestCase testCase = new TestCase() {};
	private String CS_EXE;
	private String REPORT_DIR;
	private ProcessBuilder serverBuilder;
	private ProcessBuilder clientBuilder;

	public CSControl() {
		setUp();
	}

	protected void finalize() throws Throwable {
		tearDown();
		super.finalize();
	}

	void tearDown() {
		try {
			if (server != null) {
				log.info("destroying server process");
				server.destroy();
			}
		} catch(Throwable e) {
		}
	}

	void setUp() {
		REPORT_DIR = System.getProperty("TEST_REPORTS_DIR", ".");
		CS_EXE = System.getProperty("CLIENT_SERVER_EXE", "./cs");

		log.debug("CS Tests: SETUP REPORT_DIR=" + REPORT_DIR + " CLIENT_SERVER_EXE=" + CS_EXE);
		clientBuilder = new ProcessBuilder();
		serverBuilder = new ProcessBuilder();

		clientBuilder.redirectErrorStream(true);
		serverBuilder.redirectErrorStream(true);

		Map<String, String> environment = serverBuilder.environment();

		environment.clear();
		environment.put("LD_LIBRARY_PATH", System.getenv("LD_LIBRARY_PATH"));
		environment.put("BLACKTIE_CONFIGURATION_DIR", System.getenv("BLACKTIE_CONFIGURATION_DIR"));
		environment.put("BLACKTIE_SCHEMA_DIR", System.getenv("BLACKTIE_SCHEMA_DIR"));
		environment.put("JBOSSAS_IP_ADDR", System.getenv("JBOSSAS_IP_ADDR"));

		clientBuilder.environment().putAll(environment);
		serverBuilder.command(CS_EXE, "-c", "linux", "-i", "1");

		try {
			server = startServer(serverBuilder);
		} catch (IOException e) {
			throw new RuntimeException(e);
		} catch (InterruptedException e) {
			throw new RuntimeException(e);
		}
	}

	public void runTest(String name) {
		try {
			log.debug("waiting for test " + name);
			// NOTE clientBuilder is not synchronized
			clientBuilder.command(CS_EXE, name);
			TestProcess client = startClient(name, true, clientBuilder);
			log.debug("startClient: joined - waiting for client process to exit");
			int res = client.waitFor();

			log.info("test " + name + (res == 0 ? " passed " : " failed ") + res);
			testCase.assertTrue(res == 0);
		} catch (IOException e) {
			throw new RuntimeException(e);
		} catch (InterruptedException e) {
			throw new RuntimeException(e);
		}
	}

	TestProcess startClient(String testname, boolean waitFor, ProcessBuilder builder) throws IOException, InterruptedException {
		TestProcess client = new TestProcess(REPORT_DIR + "/test-" + testname + ".txt", "client: ", builder);
		Thread thread = new Thread(client);

		thread.start();
		if (waitFor) {
			log.debug("startClient: waiting to join with client thread ...");
			thread.join();
		}

		return client;
	}

	TestProcess startServer(ProcessBuilder builder) throws IOException, InterruptedException {
		TestProcess server = new TestProcess(REPORT_DIR + "/server.txt", "server: ", builder);
		Thread thread = new Thread(server);

		synchronized (server) {
			thread.start();
			server.wait();	// wait for the process to be created
		}

		return server;
	}

	class TestProcess implements Runnable {
		private String prefix;
		private FileOutputStream ostream;
		private ProcessBuilder builder;

		private Process proc;
		private Thread thread;

		TestProcess(String outFile, String prefix, ProcessBuilder builder) throws IOException {
			this.ostream = new FileOutputStream(outFile);
			this.prefix = prefix;
			this.builder = builder;
		}

		int waitFor() throws InterruptedException { return proc.waitFor();}
		void destroy() {
			if (thread != null)
				thread.interrupt();

			if (proc != null)
				proc.destroy();
		}

		public void run() {
			thread = Thread.currentThread();

			try {
				proc = builder.start();

				synchronized (this) {
					this.notify();
				}

				InputStream is = proc.getInputStream();
				byte[] buf = new byte[1024];
				int len;

				while ((len = is.read(buf)) > 0)
					ostream.write(buf, 0, len);
			} catch (IOException e) {
				if (!thread.interrupted())
					log.info(builder.command() + "IO error on stream write: " + e);
			}

			try {
				ostream.close();
			} catch (IOException e) {
			}
		}
	}
}
