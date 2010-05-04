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
		this.server.tpadvertise("TestOne", TestSpecExampleTwoService.class
				.getName());
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
		this.server.tpadvertise("TestOne", TestTPReturnService.class.getName());
	}

	public void tpadvertiseTestTPReturn2() throws ConnectionException {
		this.server.tpadvertise("TestOne", TestTPReturnServiceTpurcode.class
				.getName());
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
	public String getServiceNameBAR() {
		throw new RuntimeException("NOT IMPLEMENTED");
	}

	public String getServiceNameLOOPY() {
		throw new RuntimeException("NOT IMPLEMENTED");
	}

	public String getServiceNameDEBIT() {
		return "TestOne";
	}

	public String getServiceNameCREDIT() {
		return "TestTwo";
	}

	public String getServiceNameINQUIRY() {
		return "TestOne";
	}

	public String getServiceNameTestTPACall() {
		return "TestOne";
	}

	public String getServiceNametpcallXOctet() {
		return "TestOne";
	}

	public String getServiceNametpcallXOctetZero() {
		throw new RuntimeException("NOT IMPLEMENTED");
	}

	public String getServiceNametpcallXCommon() {
		return "TestOne";
	}

	public String getServiceNametpcallXCType() {
		return "TestOne";
	}

	public String getServiceNameTestTPCancel() {
		return "TestOne";
	}

	public String getServiceNameTestTPConnect() {
		return "TestOne";
	}

	public String getServiceNameTestTPConversation() {
		return "TestOne";
	}

	public String getServiceNameTestTPDiscon() {
		return "TestOne";
	}

	public String getServiceNameTestTPConversa2() {
		return "TestOne";
	}

	public String getServiceNameTestTPFree() {
		throw new RuntimeException("NOT SUPPORTED");
	}

	public String getServiceNameTestTPGetrply() {
		return "TestOne";
	}

	public String getServiceNameTestTPRecv() {
		return "TestOne";
	}

	public String getServiceNameTestTPReturn() {
		return "TestOne";
	}

	public String getServiceNameTestTPReturn2() {
		return "TestOne";
	}

	public String getServiceNameTestTPSend() {
		return "TestOne";
	}

	public String getServiceNameTestTPSendTPSendOnly() {
		return "TestOne";
	}

	public String getServiceNameTestTPService() {
		return "TestOne";
	}

	public String getServiceNameTestTPUnadvertise() {
		throw new RuntimeException("NOT SUPPORTED");
	}

	public String getServiceNameTX1() {
		throw new RuntimeException("NOT SUPPORTED");
	}

	public String getServiceNameTX2() {
		throw new RuntimeException("NOT SUPPORTED");
	}

	public String getServiceNameTTL() {
		return "TestOne";
	}

	public String getServiceNameTestRollbackOnly() {
		return "TestOne";
		// return "TestRbkOnly";
	}

	public String getServiceNameTestTPGetrplyOne() {
		return "TestOne";
	}

	public String getServiceNameTestTPGetrplyTwo() {
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
