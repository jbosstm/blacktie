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

public class RunServer {

	static {
		System.loadLibrary("testsuite");
	}

	public native void serverinit();

	public native void serverdone();

	public native void tpadvertiseBAR();

	public native void tpadvertiseLOOPY();

	public native void tpadvertiseDEBIT();

	public native void tpadvertiseCREDIT();

	public native void tpadvertiseINQUIRY();

	public native void tpadvertiseTestTPACall();

	public native void tpadvertisetpcallXOctet();

	public native void tpadvertisetpcallXOctetZero();

	public native void tpadvertisetpcallXCommon();

	public native void tpadvertisetpcallXCType();

	public native void tpadvertiseTestTPCancel();

	public native void tpadvertiseTestTPConnect();

	public native void tpadvertiseTestTPConversation();

	public native void tpadvertiseTestTPConversa2();

	public native void tpadvertiseTestTPDiscon();

	public native void tpadvertiseTestTPFree();

	public native void tpadvertiseTestTPGetrply();

	public native void tpadvertiseTestTPRecv();

	public native void tpadvertiseTestTPReturn();

	public native void tpadvertiseTestTPReturn2();

	public native void tpadvertiseTestTPSend();

	public native void tpadvertiseTestTPService();

	public native void tpadvertiseTestTPUnadvertise();

	public native void tpadvertiseTX1();

	public native void tpadvertiseTX2();

	// SERVICE NAMES
	public String getServiceNameBAR() {
		return "BAR";
	}

	public String getServiceNameLOOPY() {
		return "LOOPY";
	}

	public String getServiceNameDEBIT() {
		return "DEBIT";
	}

	public String getServiceNameCREDIT() {
		return "CREDIT";
	}

	public String getServiceNameINQUIRY() {
		return "INQUIRY";
	}

	public String getServiceNameTestTPACall() {
		return "TestTPACall";
	}

	public String getServiceNametpcallXOctet() {
		return "tpcall_x_octet";
	}

	public String getServiceNametpcallXOctetZero() {
		return "tpcall_x_octet_zero";
	}

	public String getServiceNametpcallXCommon() {
		return "tpcall_x_common";
	}

	public String getServiceNametpcallXCType() {
		return "tpcall_x_c_type";
	}

	public String getServiceNameTestTPCancel() {
		return "TestTPCancel";
	}

	public String getServiceNameTestTPConnect() {
		return "TestTPConnect";
	}

	public String getServiceNameTestTPConversation() {
		return "TestTPConversation";
	}

	public String getServiceNameTestTPDiscon() {
		return "TestTPDiscon";
	}

	public String getServiceNameTestTPConversa2() {
		return "TestTPConversation";
	}

	public String getServiceNameTestTPFree() {
		return "TestTPFree";
	}

	public String getServiceNameTestTPGetrply() {
		return "TestTPGetrply";
	}

	public String getServiceNameTestTPRecv() {
		return "TestTPRecv";
	}

	public String getServiceNameTestTPReturn() {
		return "TestTPReturn";
	}

	public String getServiceNameTestTPReturn2() {
		return "TestTPReturn";
	}

	public String getServiceNameTestTPSend() {
		return "TestTPSend";
	}

	public String getServiceNameTestTPService() {
		return "TestTPService";
	}

	public String getServiceNameTestTPUnadvertise() {
		return "TestTPUnadvertise";
	}

	public String getServiceNameTX1() {
		throw new RuntimeException("NOT SUPPORTED");
	}

	public String getServiceNameTX2() {
		throw new RuntimeException("NOT SUPPORTED");
	}

	public String getServiceNameTestRollbackOnly() {
		return "TestRbkOnly";
	}

	public String getServiceNameTTL() {
		return "TTL";
	}

	public native void tpadvertiseTestRollbackOnlyTpcallTPETIMEService();

	public native void tpadvertiseTestTpcallTPEOTYPEService();

	public native void tpadvertiseTestRollbackOnlyTpcallTPESVCFAILService();

	public native void tpadvertiseTestRollbackOnlyTprecvTPEVDISCONIMMService();

	public native void tpadvertiseTestRollbackOnlyTprecvTPEVSVCFAILService();

	public native void tpadvertiseTestRollbackOnlyNoTpreturnService();

	public native void tpadvertiseTTL();

	public native void tpadvertiseTestTPSendTPSendOnly();

	public String getServiceNameTestTPSendTPSendOnly() {
		return "TestTPSend";
	}
}
