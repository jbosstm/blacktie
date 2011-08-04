package org.jboss.narayana.blacktie.quickstart.ejb.eg1;

import javax.naming.Context;
import javax.naming.InitialContext;

public class Client {
	private static final String[] names = { "FirstBTBean/remote",
			"SecondBTBean/remote" };

	public static void main(String[] args) throws Exception {
		Context ctx = new InitialContext();
		Object[] objs = new Object[names.length];
		BTTestRemote[] beans = new BTTestRemote[names.length];
		String[] results = new String[names.length];

		for (int i = 0; i < names.length; i++) {
			objs[i] = ctx.lookup(names[i]);
			beans[i] = (BTTestRemote) objs[i];
			results[i] = beans[i].echo("bean=" + names[(i + 1) % names.length]);
			System.out.println(names[i] + " result: " + results[i]);
		}

		try {
			String s = beans[0].txMandatory("bean=" + names[1]);
			System.out
					.println("Error should have got an EJBTransactionRequiredException exception");
		} catch (javax.ejb.EJBTransactionRequiredException e) {
			System.out.println("Success got EJBTransactionRequiredException");
		}
	}
}
