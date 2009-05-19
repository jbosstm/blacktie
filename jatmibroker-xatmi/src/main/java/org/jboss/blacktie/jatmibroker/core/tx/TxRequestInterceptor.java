package org.jboss.blacktie.jatmibroker.core.tx;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.AtmiBrokerServerProxy;
import org.jboss.blacktie.jatmibroker.jab.JABTransaction;
import org.jboss.blacktie.jatmibroker.jab.ThreadActionData;
import org.omg.CORBA.BAD_PARAM;
import org.omg.CORBA.LocalObject;
import org.omg.CORBA.SystemException;
import org.omg.CosTransactions.Control;
import org.omg.IOP.Codec;
import org.omg.IOP.ServiceContext;
import org.omg.PortableInterceptor.ClientRequestInfo;
import org.omg.PortableInterceptor.ClientRequestInterceptor;
import org.omg.PortableInterceptor.ServerRequestInfo;
import org.omg.PortableInterceptor.ServerRequestInterceptor;

public class TxRequestInterceptor extends LocalObject implements
		ClientRequestInterceptor, ServerRequestInterceptor {
	private static final Logger log = LogManager
			.getLogger(TxRequestInterceptor.class);

	// ORB request service context id for tagging service contexts (as
	// transactional)
	// must match up with C++ implementation (see TxInterceptor.h)
	private static final int tx_context_id = 0xabcd;

	private Codec codec;
	private String name;

	public TxRequestInterceptor(String name, Codec codec) {
		this.codec = codec;
		this.name = name;
	}

	//
	// Interceptor operations
	//

	public String name() {
		return name;
	}

	public void destroy() {
	}

	private String extract_ior_from_context(ServerRequestInfo ri) {
		return null;
	}

	private String extract_ior_from_context(ClientRequestInfo ri) {
		return null;
	}

	private Control ior_to_control(String ior) {
		/*
		 * CORBA::Object_ptr p = atmi_string_to_object(ior, orbname_);
		 * 
		 * if (!CORBA::is_nil(p)) { CosTransactions::Control_ptr cptr =
		 * CosTransactions::Control::_narrow(p); CORBA::release(p); // dispose
		 * of it now that we have narrowed the object reference
		 * LOG4CXX_LOGLS(atmiTxInterceptorLogger, log4cxx::Level::getTrace(),
		 * (char) "narrowed to " << cptr);
		 * 
		 * return cptr; }
		 */
		return null;
	}

	/*
	 * public void receive_request_service_contexts(ServerRequestInfo ri) {
	 * System
	 * .out.println("TxRequestInterceptor.receive_request_service_contexts");
	 * try { TaggedComponent comp =
	 * ri.get_effective_component(TxIORInterceptor.TAG_OTS_POLICY);
	 * ior_to_control(extract_ior_from_context(ri)); // must be transactional //
	 * TxServerClientInterceptor } catch (org.omg.CORBA.SystemException e) {
	 * System.out.println("Not a transactionalal request: " + e.getMessage()); }
	 * catch (Throwable e) {System.out.println(
	 * "Unexpected error whilst checking for a transactional request: " + e); }
	 * }
	 */
	public void send_request(ClientRequestInfo ri) {
		System.out.println("TxRequestInterceptor.send_request");
		JABTransaction curr = ThreadActionData.currentAction();

		if (curr != null) {
			System.out.println("adding tx");
			Control control = curr.getControl();
			String ior = AtmiBrokerServerProxy.orbManagement.getOrb()
					.object_to_string(control);
			System.out.println("tx ior: " + ior);
			ri.add_request_service_context(new ServiceContext(tx_context_id,
					ior.getBytes()), false);
		}
	}

	public void send_poll(ClientRequestInfo ri) {
		System.out.println("TxRequestInterceptor.send_poll");
	}

	public void receive_reply(ClientRequestInfo ri) {
		System.out.println("TxRequestInterceptor.receive_reply");
	}

	public void receive_exception(ClientRequestInfo ri) {
		System.out.println("TxRequestInterceptor.receive_exception");
	}

	public void receive_other(ClientRequestInfo ri) {
		System.out.println("TxRequestInterceptor.receive_other");
	}

	//
	// ServerRequestInterceptor operations
	//

	public void receive_request_service_contexts(ServerRequestInfo ri) {
		System.out
				.println("TxRequestInterceptor.receive_request_service_contexts");

		try {
			ServiceContext serviceContext = ri
					.get_request_service_context(tx_context_id);
			byte[] data = serviceContext.context_data;
			System.out
					.println("TxRequestInterceptor.receive_request_service_contexts: data: "
							+ data);
		} catch (BAD_PARAM e) {
			System.out
					.println("TxRequestInterceptor.receive_request_service_contexts: not transactional");
			// not a transactional request - ignore
		} catch (SystemException e) {
			System.out.println("receive_request_service_contexts error: "
					+ e.getMessage());
		}
	}

	public void receive_request(ServerRequestInfo ri) {
		System.out.println("TxRequestInterceptor.receive_request");
	}

	public void send_reply(ServerRequestInfo ri) {
		System.out.println("TxRequestInterceptor.send_reply");
	}

	public void send_exception(ServerRequestInfo ri) {
		System.out.println("TxRequestInterceptor.send_exception");
	}

	public void send_other(ServerRequestInfo ri) {
		System.out.println("TxRequestInterceptor.send_other");
	}
}
