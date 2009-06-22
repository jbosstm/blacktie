package org.jboss.blacktie.jatmibroker.mdb;

import javax.jms.BytesMessage;
import javax.jms.Destination;
import javax.jms.Message;
import javax.jms.MessageListener;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.xatmi.Service;

/**
 * All blacktie services should extend this class so that they can be advertised
 */
public abstract class MDBBlacktieService extends Service implements
		MessageListener {
	/**
	 * A logger to log the output to.
	 */
	private static final Logger log = LogManager
			.getLogger(MDBBlacktieService.class);

	public MDBBlacktieService(String name) {
		super(name);
	}

	public void onMessage(Message message) {
		try {
			BytesMessage bytesMessage = ((BytesMessage) message);
			// TODO String replyTo = message.getStringProperty("reply-to");
			Destination replyTo = message.getJMSReplyTo();
			int len = (int) bytesMessage.getBodyLength() - 1;
			String serviceName = message.getStringProperty("serviceName");
			int flags = new Integer(message.getStringProperty("messageflags"));
			int cd = new Integer(message
					.getStringProperty("messagecorrelationId"));
			byte[] bytes = new byte[len];
			bytesMessage.readBytes(bytes);

			org.jboss.blacktie.jatmibroker.transport.Message toProcess = new org.jboss.blacktie.jatmibroker.transport.Message();
			toProcess.replyTo = replyTo;
			toProcess.len = len;
			toProcess.serviceName = serviceName;
			toProcess.flags = flags;
			toProcess.control = message.getStringProperty("messagecontrol");
			toProcess.cd = cd;
			toProcess.data = bytes;
			log.debug("SERVER onMessage: ior: " + toProcess.control);
			processMessage(toProcess);
		} catch (Throwable t) {
			log.error("Could not service the request", t);
		}
	}
}
