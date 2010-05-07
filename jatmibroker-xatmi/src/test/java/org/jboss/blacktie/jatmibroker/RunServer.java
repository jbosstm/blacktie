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
package org.jboss.blacktie.jatmibroker;

import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.core.server.AtmiBrokerServer;
import org.jboss.blacktie.jatmibroker.tx.TestRollbackOnlyNoTpreturnService;
import org.jboss.blacktie.jatmibroker.tx.TestRollbackOnlyTpcallTPEOTYPEService;
import org.jboss.blacktie.jatmibroker.tx.TestRollbackOnlyTpcallTPESVCFAILService;
import org.jboss.blacktie.jatmibroker.tx.TestRollbackOnlyTpcallTPETIMEService;
import org.jboss.blacktie.jatmibroker.tx.TestRollbackOnlyTprecvTPEVDISCONIMMService;
import org.jboss.blacktie.jatmibroker.tx.TestRollbackOnlyTprecvTPEVSVCFAILService;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.blacktie.jatmibroker.xatmi.TestSpecExampleOneService;
import org.jboss.blacktie.jatmibroker.xatmi.TestSpecExampleTwoService;
import org.jboss.blacktie.jatmibroker.xatmi.TestTPACallService;
import org.jboss.blacktie.jatmibroker.xatmi.TestTPCallServiceXCType;
import org.jboss.blacktie.jatmibroker.xatmi.TestTPCallServiceXCommon;
import org.jboss.blacktie.jatmibroker.xatmi.TestTPCallServiceXOctet;
import org.jboss.blacktie.jatmibroker.xatmi.TestTPCancelService;
import org.jboss.blacktie.jatmibroker.xatmi.TestTPConnectService;
import org.jboss.blacktie.jatmibroker.xatmi.TestTPConversationService;
import org.jboss.blacktie.jatmibroker.xatmi.TestTPConversationServiceShort;
import org.jboss.blacktie.jatmibroker.xatmi.TestTPDisconService;
import org.jboss.blacktie.jatmibroker.xatmi.TestTPGetRplyOneService;
import org.jboss.blacktie.jatmibroker.xatmi.TestTPGetRplyService;
import org.jboss.blacktie.jatmibroker.xatmi.TestTPGetRplyTwoService;
import org.jboss.blacktie.jatmibroker.xatmi.TestTPRecvService;
import org.jboss.blacktie.jatmibroker.xatmi.TestTPReturnService;
import org.jboss.blacktie.jatmibroker.xatmi.TestTPReturnServiceOpenSession1;
import org.jboss.blacktie.jatmibroker.xatmi.TestTPReturnServiceOpenSession2;
import org.jboss.blacktie.jatmibroker.xatmi.TestTPReturnServiceTpurcode;
import org.jboss.blacktie.jatmibroker.xatmi.TestTPSendService;
import org.jboss.blacktie.jatmibroker.xatmi.TestTPSendTPSendOnlyService;
import org.jboss.blacktie.jatmibroker.xatmi.TestTPServiceService;
import org.jboss.blacktie.jatmibroker.xatmi.TestTTLService;

public class RunServer {

	private AtmiBrokerServer server;

	public void serverinit() throws ConfigurationException, ConnectionException {
		this.server = new AtmiBrokerServer("standalone-server");
	}

	public void serverdone() throws ConnectionException {
		server.close();
	}

	public void tpadvertiseBAR() {
	}

	public void tpadvertiseLOOPY() {
	}

	public void tpadvertiseDEBIT() throws ConnectionException {
		this.server.tpadvertise("TestOne", TestSpecExampleOneService.class
				.getName());
	}

	public void tpadvertiseCREDIT() throws ConnectionException {
		this.server.tpadvertise("TestTwo", TestSpecExampleOneService.class
				.getName());
	}

	public void tpadvertiseINQUIRY() throws ConnectionException {
		this.server.tpadvertise(getServiceNameINQUIRY(),
				TestSpecExampleTwoService.class.getName());
	}

	public void tpadvertiseTestTPACall() throws ConnectionException {
		this.server.tpadvertise("TestOne", TestTPACallService.class.getName());
	}

	public void tpadvertisetpcallXOctet() throws ConnectionException {
		this.server.tpadvertise("TestOne", TestTPCallServiceXOctet.class
				.getName());
	}

	public void tpadvertisetpcallXOctetZero() throws ConnectionException {
	}

	public void tpadvertisetpcallXCommon() throws ConnectionException {
		this.server.tpadvertise("TestOne", TestTPCallServiceXCommon.class
				.getName());
	}

	public void tpadvertisetpcallXCType() throws ConnectionException {
		this.server.tpadvertise("TestOne", TestTPCallServiceXCType.class
				.getName());
	}

	public void tpadvertiseTestTPCancel() throws ConnectionException {
		this.server.tpadvertise("TestOne", TestTPCancelService.class.getName());
	}

	public void tpadvertiseTestTPConnect() throws ConnectionException {
		this.server
				.tpadvertise("TestOne", TestTPConnectService.class.getName());
	}

	public void tpadvertiseTestTPConversation() throws ConnectionException {
		this.server.tpadvertise("TestOne", TestTPConversationService.class
				.getName());
	}

	public void tpadvertiseTestTPConversa2() throws ConnectionException {
		this.server.tpadvertise("TestOne", TestTPConversationServiceShort.class
				.getName());
	}

	public void tpadvertiseTestTPDiscon() throws ConnectionException {
		this.server.tpadvertise("TestOne", TestTPDisconService.class.getName());
	}

	public void tpadvertiseTestTPFree() throws ConnectionException {
	}

	public void tpadvertiseTestTPGetrply() throws ConnectionException {
		this.server
				.tpadvertise("TestOne", TestTPGetRplyService.class.getName());
	}

	public void tpadvertiseTestTPRecv() throws ConnectionException {
		this.server.tpadvertise("TestOne", TestTPRecvService.class.getName());
	}

	public void tpadvertiseTestTPReturn() throws ConnectionException {
		this.server.tpadvertise(getServiceNameTestTPReturn(),
				TestTPReturnService.class.getName());
	}

	public void tpadvertiseTestTPReturn2() throws ConnectionException {
		this.server.tpadvertise(getServiceNameTestTPReturn2(),
				TestTPReturnServiceTpurcode.class.getName());
	}

	public void tpadvertiseTestTPReturn3() throws ConnectionException {
		this.server.tpadvertise(getServiceNameTestTPReturn(),
				TestTPReturnServiceOpenSession1.class.getName());

	}

	public void tpadvertiseTestTPReturn4() throws ConnectionException {
		this.server.tpadvertise(getServiceNameTestTPReturn2(),
				TestTPReturnServiceOpenSession2.class.getName());

	}

	public void tpadvertiseTestTPSend() throws ConnectionException {
		this.server.tpadvertise("TestOne", TestTPSendService.class.getName());
	}

	public void tpadvertiseTestTPSendTPSendOnly() throws ConnectionException {
		this.server.tpadvertise("TestOne", TestTPSendTPSendOnlyService.class
				.getName());
	}

	public void tpadvertiseTestTPService() throws ConnectionException {
		this.server
				.tpadvertise("TestOne", TestTPServiceService.class.getName());
	}

	public void tpadvertiseTestTPUnadvertise() throws ConnectionException {
	}

	public void tpadvertiseTX1() throws ConnectionException {
	}

	public void tpadvertiseTX2() throws ConnectionException {
	}

	public void tpadvertiseTTL() throws ConnectionException {
		this.server.tpadvertise("TestOne", TestTTLService.class.getName());
	}

	// SERVICE NAMES
	public static String getServiceNameBAR() {
		throw new RuntimeException("NOT IMPLEMENTED");
	}

	public static String getServiceNameLOOPY() {
		throw new RuntimeException("NOT IMPLEMENTED");
	}

	public static String getServiceNameDEBIT() {
		return "TestOne";
	}

	public static String getServiceNameCREDIT() {
		return "TestTwo";
	}

	public static String getServiceNameINQUIRY() {
		return "TestOne";
	}

	public static String getServiceNameTestTPACall() {
		return "TestOne";
	}

	public static String getServiceNametpcallXOctet() {
		return "TestOne";
	}

	public static String getServiceNametpcallXOctetZero() {
		throw new RuntimeException("NOT IMPLEMENTED");
	}

	public static String getServiceNametpcallXCommon() {
		return "TestOne";
	}

	public static String getServiceNametpcallXCType() {
		return "TestOne";
	}

	public static String getServiceNameTestTPCancel() {
		return "TestOne";
	}

	public static String getServiceNameTestTPConnect() {
		return "TestOne";
	}

	public static String getServiceNameTestTPConversation() {
		return "TestOne";
	}

	public static String getServiceNameTestTPDiscon() {
		return "TestOne";
	}

	public static String getServiceNameTestTPConversa2() {
		return "TestOne";
	}

	public static String getServiceNameTestTPFree() {
		throw new RuntimeException("NOT SUPPORTED");
	}

	public static String getServiceNameTestTPGetrply() {
		return "TestOne";
	}

	public static String getServiceNameTestTPRecv() {
		return "TestOne";
	}

	public static String getServiceNameTestTPReturn() {
		return "TestOne";
	}

	public static String getServiceNameTestTPReturn2() {
		return "TestTwo";
	}

	public static String getServiceNameTestTPSend() {
		return "TestOne";
	}

	public static String getServiceNameTestTPSendTPSendOnly() {
		return "TestOne";
	}

	public static String getServiceNameTestTPService() {
		return "TestOne";
	}

	public static String getServiceNameTestTPUnadvertise() {
		throw new RuntimeException("NOT SUPPORTED");
	}

	public static String getServiceNameTX1() {
		throw new RuntimeException("NOT SUPPORTED");
	}

	public static String getServiceNameTX2() {
		throw new RuntimeException("NOT SUPPORTED");
	}

	public static String getServiceNameTTL() {
		return "TestOne";
	}

	public static String getServiceNameTestRollbackOnly() {
		return "TestOne";
		// return "TestRbkOnly";
	}

	public static String getServiceNameTestTPGetrplyOne() {
		return "TestOne";
	}

	public static String getServiceNameTestTPGetrplyTwo() {
		return "TestTwo";
	}

	public void tpadvertiseTestRollbackOnlyTpcallTPETIMEService()
			throws ConnectionException {
		this.server.tpadvertise(getServiceNameTestRollbackOnly(),
				TestRollbackOnlyTpcallTPETIMEService.class.getName());
	}

	public void tpadvertiseTestTpcallTPEOTYPEService()
			throws ConnectionException {
		this.server.tpadvertise(getServiceNameTestRollbackOnly(),
				TestRollbackOnlyTpcallTPEOTYPEService.class.getName());

	}

	public void tpadvertiseTestRollbackOnlyTpcallTPESVCFAILService()
			throws ConnectionException {
		this.server.tpadvertise(getServiceNameTestRollbackOnly(),
				TestRollbackOnlyTpcallTPESVCFAILService.class.getName());
	}

	public void tpadvertiseTestRollbackOnlyTprecvTPEVDISCONIMMService()
			throws ConnectionException {
		this.server.tpadvertise(getServiceNameTestRollbackOnly(),
				TestRollbackOnlyTprecvTPEVDISCONIMMService.class.getName());
	}

	public void tpadvertiseTestRollbackOnlyTprecvTPEVSVCFAILService()
			throws ConnectionException {
		this.server.tpadvertise(getServiceNameTestRollbackOnly(),
				TestRollbackOnlyTprecvTPEVSVCFAILService.class.getName());
	}

	public void tpadvertiseTestRollbackOnlyNoTpreturnService()
			throws ConnectionException {
		this.server.tpadvertise(getServiceNameTestRollbackOnly(),
				TestRollbackOnlyNoTpreturnService.class.getName());
	}

	public void tpadvertiseTestTPGetrplyOne() throws ConnectionException {
		this.server.tpadvertise(getServiceNameTestTPGetrplyOne(),
				TestTPGetRplyOneService.class.getName());

	}

	public void tpadvertiseTestTPGetrplyTwo() throws ConnectionException {
		this.server.tpadvertise(getServiceNameTestTPGetrplyTwo(),
				TestTPGetRplyTwoService.class.getName());

	}
}
