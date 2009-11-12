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

public class CSTest extends junit.framework.TestCase
{
	private static final org.apache.log4j.Logger log = org.apache.log4j.LogManager.getLogger(CSTest.class);
	private static CSControl control = new CSControl();

	// XsdValidator is not thread safe
	public void test_211() { control.runTest("211"); }
	// tpcall incorrectly returns TPNOTIME whenever the TPNOBLOCK or TPNOTIME flags are specified
	public void test_2120() { control.runTest("2120"); }
	// Similarly specifying TPNOBLOCK means that if a blocking condition does exist then the caller
	// should get the error TPEBLOCK
	public void test_2121() { control.runTest("2121"); }
	// tpcall should return TPEINVAL if the service name is invalid
	public void test_213() { control.runTest("213"); }
	// TPSIGRSTRT flag isn't supported on tpcall	
	public void test_214() { control.runTest("214"); }
	// tpcall failure with multiple threads
	public void test_215() { control.runTest("215"); }
	//tp bufs should morph if they're the wrong type
	public void test_2160() { control.runTest("2160"); }
	// passing the wrong return buffer type with TPNOCHANGE
	public void test_2161() { control.runTest("2161"); }
	// make sure tpurcode works
	public void test_217() { control.runTest("217"); }
	// sanity check
	public void test_0() { control.runTest("0"); }
	// tell the server to set a flag on tpreturn (should generate TPESVCERR)
	public void test_1() { control.runTest("1"); }
	// set flag on tpreturn should fail
	public void test_2() { control.runTest("2"); }
	// telling the service to not tpreturn should generate an error
	public void test_3() { control.runTest("3"); }
	// telling service to call tpreturn outside service routine should have no effect
	public void test_4() { control.runTest("4"); }
	// tpreturn outside service routing
	public void test_5() { control.runTest("5"); }
}
