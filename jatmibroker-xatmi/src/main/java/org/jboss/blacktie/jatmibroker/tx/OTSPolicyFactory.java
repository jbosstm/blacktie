package org.jboss.blacktie.jatmibroker.tx;

import org.omg.PortableInterceptor.PolicyFactory;
import org.omg.CORBA.Policy;
import org.omg.CORBA.PolicyError;
import org.omg.CORBA.Any;
import org.omg.CORBA.BAD_POLICY_TYPE;
import org.omg.CORBA.LocalObject;

public class OTSPolicyFactory extends LocalObject implements PolicyFactory
{
	public Policy create_policy(int type, Any value) throws PolicyError
	{
		if (type != TxIORInterceptor.OTS_POLICY_TYPE)
			throw new PolicyError("Wrong policy type", BAD_POLICY_TYPE.value);

		short val = value.extract_short();

		OTSPolicy p = new OTSPolicy(val);

		return p;
	}
}
