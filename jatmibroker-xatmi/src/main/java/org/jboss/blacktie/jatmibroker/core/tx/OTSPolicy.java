package org.jboss.blacktie.jatmibroker.core.tx;

import org.omg.CORBA.LocalObject;
import org.omg.CORBA.Policy;

public class OTSPolicy extends LocalObject implements Policy {
	private short tpv_;

	public OTSPolicy(short val) {
		this.tpv_ = val;
	}

	public short tpv() {
		return tpv_;
	}

	public int policy_type() {
		return TxIORInterceptor.OTS_POLICY_TYPE;
	}

	public Policy copy() {
		return new OTSPolicy(tpv_);
	}

	public void destroy() {
	}
}
