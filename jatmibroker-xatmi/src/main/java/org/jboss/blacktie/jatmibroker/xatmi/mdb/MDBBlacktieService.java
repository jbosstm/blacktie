package org.jboss.blacktie.jatmibroker.xatmi.mdb;

import javax.jms.BytesMessage;
import javax.jms.Destination;
import javax.jms.Message;
import javax.jms.MessageListener;
import javax.jms.Queue;
import javax.jms.Topic;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.transport.JtsTransactionImple;
import org.jboss.blacktie.jatmibroker.core.transport.hybrid.JMSReceiverImpl;
import org.jboss.blacktie.jatmibroker.xatmi.BlackTieService;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;

/**
 * All BlackTie MDB services should extend this class so that they can be
 * advertised
 */
public abstract class MDBBlacktieService extends BlackTieService implements
		MessageListener {
	/**
	 * A logger to log the output to.
	 */
	private static final Logger log = LogManager
			.getLogger(MDBBlacktieService.class);

	/**
	 * The onMessage method formats the JMS received bytes message into a format
	 * understood by the XATMI API.
	 * 
	 * @param message
	 *            The message received wrapping an XATMI invocation
	 */
	public void onMessage(Message message) {
		try {
			String serviceName = null;
			Destination jmsDestination = message.getJMSDestination();
			if (jmsDestination instanceof Queue) {
				serviceName = ((Queue) jmsDestination).getQueueName();
			} else {
				serviceName = ((Topic) jmsDestination).getTopicName();
			}
			serviceName = serviceName.substring(serviceName.indexOf('_') + 1);
			log.info(serviceName);
			BytesMessage bytesMessage = ((BytesMessage) message);
			org.jboss.blacktie.jatmibroker.core.transport.Message toProcess = JMSReceiverImpl
					.convertFromBytesMessage(bytesMessage);
			log.debug("SERVER onMessage: transaction control ior: "
					+ toProcess.control);
			if (JtsTransactionImple.hasTransaction()) {
				throw new ConnectionException(Connection.TPEPROTO,
						"Blacktie MDBs must not be called with a transactional context");
			}
			processMessage(serviceName, toProcess);
			log.debug("Processed message");
		} catch (Throwable t) {
			log.error("Could not service the request", t);
		}
	}
}
