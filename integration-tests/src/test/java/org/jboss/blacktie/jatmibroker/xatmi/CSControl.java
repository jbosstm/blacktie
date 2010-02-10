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

public abstract class CSControl extends TestCase
{
	private static final Logger log = LogManager.getLogger(CSControl.class);
	// the byte pattern written by a server to indicate that it has advertised its services
	private static final byte[] HANDSHAKE = {83,69,82,86,73,67,69,83,32,82,69,65,68,89};

	private ProcessBuilder serverBuilder;
	private ProcessBuilder clientBuilder;
	private TestProcess server;
	private TestProcess client;
	private String CS_EXE;
	private String REPORT_DIR;

	public void tearDown() {
		try {
			log.debug("destroying server process");
			server.interrupt();
			server.getProcess().destroy();
			log.debug("destroyed server process");
		} catch(Throwable e) {
			throw new RuntimeException("Server shutdown error: ", e);
		}
	}

	public void setUp() {
		REPORT_DIR = System.getProperty("TEST_REPORTS_DIR", ".");
		CS_EXE = System.getProperty("CLIENT_SERVER_EXE", "./cs");
		clientBuilder = new ProcessBuilder();
		serverBuilder = new ProcessBuilder();
		//clientBuilder.redirectErrorStream(true);
		//serverBuilder.redirectErrorStream(true);

		java.util.Map<String, String> environment = serverBuilder.environment();
		//environment.clear();
		environment.put("LD_LIBRARY_PATH", System.getenv("LD_LIBRARY_PATH"));
		environment.put("BLACKTIE_CONFIGURATION_DIR", System.getenv("BLACKTIE_CONFIGURATION_DIR"));
		environment.put("BLACKTIE_SCHEMA_DIR", System.getenv("BLACKTIE_SCHEMA_DIR"));
		environment.put("JBOSSAS_IP_ADDR", System.getenv("JBOSSAS_IP_ADDR"));
		environment.put("PATH", System.getenv("PATH"));
		clientBuilder.environment().putAll(environment);
		serverBuilder.command(CS_EXE, "-c", "linux", "-i", "1");

		try {
			server = startServer(serverBuilder);
		} catch (IOException e) {
			throw new RuntimeException("Server io exception: ", e);
		} catch (InterruptedException e) {
			throw new RuntimeException("Server interrupted: ", e);
		}
	}

	public void runTest(String name) {
		try {
			log.debug("waiting for test " + name);
			clientBuilder.command(CS_EXE, name);
			TestProcess client = startClient(name, clientBuilder);
			int res = client.exitValue();

			log.info("test " + name + (res == 0 ? " passed " : " failed ") + res);
			assertTrue(res == 0);
		} catch (IOException e) {
			throw new RuntimeException(e);
		} catch (InterruptedException e) {
			throw new RuntimeException(e);
		}
	}

	private TestProcess startClient(String testname, ProcessBuilder builder) throws IOException, InterruptedException {
		FileOutputStream ostream = new FileOutputStream(REPORT_DIR + "/test-" + testname + "-out.txt");
		FileOutputStream estream = new FileOutputStream(REPORT_DIR + "/test-" + testname + "-err.txt");
		TestProcess client = new TestProcess(ostream, estream, "client", builder);
		Thread thread = new Thread(client);

		thread.start();
		log.debug("startClient: waiting to join with client thread ...");
		thread.join();
		log.debug("startClient: joined - waiting for client process to exit");
		client.getProcess().waitFor();
		log.debug("startClient: done");

		return client;
	}

	private TestProcess startServer(ProcessBuilder builder) throws IOException, InterruptedException {
		FileOutputStream ostream = new FileOutputStream(REPORT_DIR + "/server-out.txt");
		FileOutputStream estream = new FileOutputStream(REPORT_DIR + "/server-err.txt");
		TestProcess server = new TestProcess(ostream, estream, "server", builder);
		Thread thread = new Thread(server);

		synchronized (server) {
			// start the C server and wait for it to indicate that it has advertised its services
			thread.start();
			server.wait();
		}

		return server;
	}

	class TestProcess implements Runnable {
		private String type;
		private String command;
		private Process proc;
		private FileOutputStream ostream;
		private FileOutputStream estream;
		private Thread thread;
		private ProcessBuilder builder;

		TestProcess(FileOutputStream ostream, FileOutputStream estream, String type, ProcessBuilder builder) {
			this.ostream = ostream;
			this.estream = estream;
			this.type = type;
			this.command = command;
			this.builder = builder;
		}

		Process getProcess() { return proc; }
		int exitValue() { return proc.exitValue(); }
		void interrupt() { if (thread != null) thread.interrupt(); }

		public void run() {
			thread = Thread.currentThread();

			try {
				proc = builder.start();

				InputStream is = proc.getInputStream();
				InputStream es = proc.getErrorStream();
				byte[] buf = new byte[1024];
				int len;
				int match = -1;

				/*
				 * redirect the process I/O to a file - if it is a server then notify any waiters when the server
				 * outputs a magic sequence indicating that it has advertised its services
				 */
				if ("server".equals(type)) {
					// assume the HANDSHAKE sequence can be read in one go
/*
					while ((len = is.read(buf)) > 0) {
						if (match == -1 && (match = KMPMatch.indexOf(buf, HANDSHAKE, len)) != -1) {
							synchronized (this) { this.notify(); }
						}

						ostream.write(buf, 0, len);
					}
*/
					int pos = 0;
					while ((len = is.read(buf, pos, buf.length - pos)) > 0) {
						ostream.write(buf, pos, len);

						if (match == -1) {
							pos += len;

							if ((match = KMPMatch.indexOf(buf, HANDSHAKE, pos)) != -1) {
								synchronized (this) { this.notify(); }
								pos = 0;
							} else if (pos == buf.length) {
								ostream.write("missing synchronization sequence from service - force notify".getBytes("UTF-8"));
								log.warn("missing synchronization sequence from service");
								synchronized (this) { this.notify(); }
								pos = 0;
								match = 0;
							}
						} else {
							pos = 0;
						}
					}
				} else {
					while ((len = is.read(buf)) > 0)
						ostream.write(buf, 0, len);
				}

				while ((len = es.read(buf)) > 0)
					estream.write(buf, 0, len);
			} catch (IOException e) {
				if (!thread.interrupted())
					log.warn(builder.command() + ": IO error on stream write: " + e);
			}

			try {
				ostream.close();
				estream.close();
			} catch (IOException e) {
			}
		}
	}
}
