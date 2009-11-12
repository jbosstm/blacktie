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
	private String[] ENV_ARRAY;

	public CSControl() {
		setUp();
	}

	protected void finalize() throws Throwable {
		tearDown();
		super.finalize();
	}

	public void tearDown() {
		try {
			if (server != null) {
				log.info("destroying server process");
				server.interrupt();
				server.getProcess().destroy();
			}
		} catch(Throwable e) {
		}
	}

	public void setUp() {
		REPORT_DIR = System.getProperty("TEST_REPORTS_DIR", ".");
		CS_EXE = System.getProperty("CLIENT_SERVER_EXE", "./cs");

		if (server == null) {
			log.info("CS Tests: SETUP REPORT_DIR=" + REPORT_DIR + " CLIENT_SERVER_EXE=" + CS_EXE);

			ENV_ARRAY = new String[4];

			ENV_ARRAY[0] = "LD_LIBRARY_PATH=" + System.getenv("LD_LIBRARY_PATH");
			ENV_ARRAY[1] = "BLACKTIE_CONFIGURATION_DIR=" + System.getenv("BLACKTIE_CONFIGURATION_DIR");
			ENV_ARRAY[2] = "BLACKTIE_SCHEMA_DIR=" + System.getenv("BLACKTIE_SCHEMA_DIR");
			ENV_ARRAY[3] = "JBOSSAS_IP_ADDR=" + System.getenv("JBOSSAS_IP_ADDR");

			for (int i = 0; i < 4; i++)
				log.debug(ENV_ARRAY[i]);

			try {
				server = startServer(ENV_ARRAY);
				Thread.sleep(3000);
			} catch (IOException e) {
				System.err.printf("Server io exception: " + e);
			} catch (InterruptedException e) {
				System.err.printf("Server interrupted: " + e);
			}
		}
	}

	void runTest(String name) {
		try {
			log.debug("waiting for test " + name);
			TestProcess client = startClient(name, true, ENV_ARRAY);
			int res = client.exitValue();

			log.info("test " + name + (res == 0 ? " passed " : " failed ") + res);
			testCase.assertTrue(res == 0);
		} catch (IOException e) {
			throw new RuntimeException(e);
		} catch (InterruptedException e) {
			throw new RuntimeException(e);
		}
	}

	TestProcess startClient(String testname, boolean waitFor, String[] envp) throws IOException, InterruptedException {
		FileOutputStream ostream = new FileOutputStream(REPORT_DIR + "/test-" + testname + "-out.txt");
		FileOutputStream estream = new FileOutputStream(REPORT_DIR + "/test-" + testname + "-err.txt");
		TestProcess client = new TestProcess(ostream, estream, "client: ", CS_EXE + " " + testname, envp);
		Thread thread = new Thread(client);

		thread.start();
		if (waitFor) {
			log.debug("startClient: waiting to join with client thread ...");
			thread.join();
			log.debug("startClient: joined - waiting for client process to exit");
			client.getProcess().waitFor();
			log.debug("startClient: done");
		} else {
		}

		return client;
	}

	TestProcess startServer(String[] envp) throws IOException {
		FileOutputStream ostream = new FileOutputStream(REPORT_DIR + "/server-out.txt");
		FileOutputStream estream = new FileOutputStream(REPORT_DIR + "/server-err.txt");
		TestProcess server = new TestProcess(ostream, estream, "server: ", CS_EXE + " -c linux -i 1", envp);
		Thread thread = new Thread(server);
		thread.start();

		return server;
	}

	class TestProcess implements Runnable {
		private String prefix;
		private String command;
		private Process proc;
		private FileOutputStream ostream;
		private FileOutputStream estream;
		private String[] envp;
		private Thread thread;

		TestProcess(FileOutputStream ostream, FileOutputStream estream, String prefix, String command, String[] envp) {
			this.ostream = ostream;
			this.estream = estream;
			this.prefix = prefix;
			this.command = command;
			this.envp = envp;
		}

		Process getProcess() { return proc; }
		int exitValue() { return proc.exitValue(); }
		void interrupt() { if (thread != null) thread.interrupt(); }

		public void run() {
			thread = Thread.currentThread();

			try {
				Runtime rt = Runtime.getRuntime();
				proc = rt.exec(command, envp);

				InputStream is = proc.getInputStream();
				InputStream es = proc.getErrorStream();
				byte[] buf = new byte[1024];
				int len;

				while ((len = is.read(buf)) > 0)
					ostream.write(buf, 0, len);

				while ((len = es.read(buf)) > 0)
					estream.write(buf, 0, len);
			} catch (IOException e) {
				if (!thread.interrupted())
					log.info(command + ": IO error on stream write: " + e);
			}

			try {
				ostream.close();
				estream.close();
			} catch (IOException e) {
			}
		}
	}
}
