package org.jboss.blacktie.jatmibroker.xatmi.ejb;

import java.util.Properties;

import javax.jms.BytesMessage;
import javax.jms.Message;
import javax.jms.MessageListener;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.JAtmiBrokerException;
import org.jboss.blacktie.jatmibroker.conf.AtmiBrokerServerXML;
import org.jboss.blacktie.jatmibroker.core.Connection;
import org.jboss.blacktie.jatmibroker.core.ConnectionFactory;
import org.jboss.blacktie.jatmibroker.core.Sender;
import org.jboss.blacktie.jatmibroker.xatmi.BlacktieService;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectorException;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;

/**
 * All blacktie services should extend this class so that they can be advertised
 */
public abstract class BlacktieMDBService implements BlacktieService,
		MessageListener {
	/**
	 * A logger to log the output to.
	 */
	private static final Logger log = LogManager
			.getLogger(BlacktieMDBService.class);

	private Connection connection;

	/**
	 * Must have a no-arg constructor
	 * 
	 * @throws ConnectorException
	 * @throws JAtmiBrokerException
	 */
	public BlacktieMDBService() throws JAtmiBrokerException {
		Properties properties = null;
		AtmiBrokerServerXML server = new AtmiBrokerServerXML();
		try {
			properties = server.getProperties("");
		} catch (Exception e) {
			throw new JAtmiBrokerException("Could not load properties", e);
		}
		connection = ConnectionFactory.loadConnectionFactory(properties)
				.createConnection("", "");
	}

	public void onMessage(Message message) {
		try {
			BytesMessage bytesMessage = ((BytesMessage) message);
			String replyTo = message.getStringProperty("reply-to");
			int messagelength = (int) bytesMessage.getBodyLength();
			String serviceName = message.getStringProperty("serviceName");
			long messageflags = new Long(message
					.getStringProperty("messageflags"));
			long messagecorrelationId = new Long(message
					.getStringProperty("messagecorrelationId"));
			byte[] bytes = new byte[(int) messagelength];
			bytesMessage.readBytes(bytes);

			// TODO HANDLE CONTROL
			// THIS IS THE FIRST CALL
			TPSVCINFO tpsvcinfo = new TPSVCINFO(serviceName, bytes,
					messagelength, messageflags, -1);
			Response response = tpservice(tpsvcinfo);
			// TODO THIS SHOULD INVOKE THE CLIENT HANDLER
			// odata.value = serviceRequest.getBytes();
			// olen.value = serviceRequest.getLength();
			Sender sender = connection.createSender(replyTo);
			sender.send("", response.getRval(), response.getRcode(), response
					.getData(), response.getLength(), response.getFlags(), 0);
		} catch (Throwable t) {
			log.error("Could not service the request");
		}
	}
}
