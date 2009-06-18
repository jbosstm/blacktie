package org.jboss.blacktie.example.ejb.eg1;

import java.util.Map;
import java.util.HashMap;

import javax.ejb.TransactionAttribute;
import javax.ejb.TransactionAttributeType;
import javax.naming.NamingException;
import javax.naming.InitialContext;
import javax.naming.Context;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

public class BTTestBean implements BTTestRemote {
	private static final Logger log = LogManager.getLogger(BTTestBean.class);
	private String name = "";

	public BTTestBean(String name) {this.name = name;}

    public String txRequired(String arg) { return handleRequest("txRequired", arg); }
    public String txSupports(String arg) { return handleRequest("txSupports", arg); }
    public String txMandatory(String arg) { return handleRequest("txMandatory", arg); }
    public String txNever(String arg) { return handleRequest("txNever", arg); }
    public String txRequiresNew(String arg) { return handleRequest("txRequiresNew", arg); }
    public String txNotSupported(String arg) { return handleRequest("txNotSupported", arg); }
    public String echo(String arg) { return handleRequest("echo", arg); }

    protected String handleRequest(String methodName, String arg) {
        Map<String, String> map = new HashMap<String, String> ();
		BTTestRemote bean = null;

        if (parse(arg, map) != 0) {

            for (Map.Entry<String, String> e : map.entrySet()) {
                if ("bean".equals(e.getKey())) {
					try {
						bean = (BTTestRemote) getBean(e.getValue());
						arg = "invocation from " + name; //getClass().getSimpleName();
					} catch (NamingException ex) {
						return e.getKey() + " is not a valid bean name: " + ex.getMessage();
					}
                } else if ("method".equals(e.getKey())) {
					methodName = e.getValue();
				}
            }
		}

		return invokeBean(bean, methodName, arg);
    }

	protected String invokeBean(BTTestRemote bean, String methodName, String arg) {
		log.debug(methodName);
		if (bean == null || bean == this) {
        	return arg + " - via " + name; //getClass().getSimpleName();
		} else if ("txRequired".equals(methodName)) {
			return bean.txRequired(arg);
		} else if ("txSupports".equals(methodName)) {
			return bean.txSupports(arg);
		} else if ("txMandatory".equals(methodName)) {
			return bean.txMandatory(arg);
		} else if ("txNever".equals(methodName)) {
			return bean.txNever(arg);
		} else if ("txRequiresNew".equals(methodName)) {
			return bean.txRequiresNew(arg);
		} else if ("txNotSupported".equals(methodName)) {
			return bean.txNotSupported(arg);
		} else {
			return bean.echo(arg);
		}
	}

	protected Object getBean(String name) throws NamingException {
		Context ctx = new InitialContext();
		return ctx.lookup(name);
	}

	protected static int parse(String arg, Map<String, String> map) {
		System.out.println(arg);
		String[] nvps = arg.split("&");

		for (String nvp : nvps) {
			String[] nv = nvp.split("=");
			if (nv.length == 2)
				map.put(nv[0], nv[1]);
		}

		return map.size();
	}
}
