package org.jboss.blacktie.jatmibroker.xatmi.mdb;

import javax.jms.BytesMessage;
import javax.jms.Message;
import javax.jms.MessageListener;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.blacktie.jatmibroker.xatmi.Service;

/**
 * All blacktie MDB services should extend this class so that they can be
 * advertised
 */
public abstract class MDBBlacktieService extends Service implements
		MessageListener {
	/**
	 * A logger to log the output to.
	 */
	private static final Logger log = LogManager
			.getLogger(MDBBlacktieService.class);

	/**
	 * MDB services should use this constructor with the name of the service
	 * they are using.
	 * 
	 * @param name
	 *            The name of the service
	 * @throws ConnectionException
	 *             In case the connection cannot be established
	 * @throws ConfigurationException
	 *             In case the Environment.xml is invalid or the transport does
	 *             not exist
	 */
	public MDBBlacktieService(String name) throws ConfigurationException,
			ConnectionException {
		super(name);
	}

	public void onMessage(Message message) {
		try {
			BytesMessage bytesMessage = ((BytesMessage) message);
			// TODO String replyTo = message.getStringProperty("reply-to");
			String replyTo = message.getStringProperty("messagereplyto");
			int len = (int) bytesMessage.getBodyLength();
			String serviceName = message.getStringProperty("serviceName");
			int flags = new Integer(message.getStringProperty("messageflags"));
			int cd = new Integer(message
					.getStringProperty("messagecorrelationId"));
			byte[] bytes = new byte[len];
			bytesMessage.readBytes(bytes);

			org.jboss.blacktie.jatmibroker.core.transport.Message toProcess = new org.jboss.blacktie.jatmibroker.core.transport.Message();
			toProcess.replyTo = replyTo;
			toProcess.len = len;
			toProcess.serviceName = serviceName;
			toProcess.flags = flags;
			toProcess.control = message.getStringProperty("messagecontrol");
			toProcess.cd = cd;
			toProcess.data = bytes;
			log.debug("SERVER onMessage: ior: " + toProcess.control);
			processMessage(toProcess);
			log.debug("Processed message");
		} catch (Throwable t) {
			log.error("Could not service the request", t);
		}
	}
}
