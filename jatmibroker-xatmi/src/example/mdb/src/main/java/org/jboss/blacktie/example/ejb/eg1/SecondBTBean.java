package org.jboss.blacktie.example.ejb.eg1;

import javax.ejb.Stateless;
import javax.ejb.TransactionAttribute;
import javax.ejb.TransactionAttributeType;

@Stateless
public class SecondBTBean /* extends BTTestBean */implements BTTestRemote {
	private BTTestBean bean = new BTTestBean("SecondBTBean");

	@TransactionAttribute(TransactionAttributeType.REQUIRED)
	public String txRequired(String arg) {
		return bean.handleRequest("txRequired", arg);
	}

	@TransactionAttribute(TransactionAttributeType.SUPPORTS)
	public String txSupports(String arg) {
		return bean.handleRequest("txSupports", arg);
	}

	@TransactionAttribute(TransactionAttributeType.MANDATORY)
	public String txMandatory(String arg) {
		return bean.handleRequest("txMandatory", arg);
	}

	@TransactionAttribute(TransactionAttributeType.NEVER)
	public String txNever(String arg) {
		return bean.handleRequest("txNever", arg);
	}

	@TransactionAttribute(TransactionAttributeType.REQUIRES_NEW)
	public String txRequiresNew(String arg) {
		return bean.handleRequest("txRequiresNew", arg);
	}

	@TransactionAttribute(TransactionAttributeType.NOT_SUPPORTED)
	public String txNotSupported(String arg) {
		return bean.handleRequest("txNotSupported", arg);
	}

	public String echo(String arg) {
		return bean.handleRequest("echo", arg);
	}
}
