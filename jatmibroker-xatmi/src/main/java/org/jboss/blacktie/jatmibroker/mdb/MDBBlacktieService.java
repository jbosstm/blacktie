package org.jboss.blacktie.jatmibroker.mdb;

import java.util.Properties;

import javax.jms.BytesMessage;
import javax.jms.Message;
import javax.jms.MessageListener;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.JAtmiBrokerException;
import org.jboss.blacktie.jatmibroker.conf.AtmiBrokerClientXML;
import org.jboss.blacktie.jatmibroker.transport.Sender;
import org.jboss.blacktie.jatmibroker.transport.Transport;
import org.jboss.blacktie.jatmibroker.transport.TransportFactory;
import org.jboss.blacktie.jatmibroker.xatmi.BlacktieService;
import org.jboss.blacktie.jatmibroker.xatmi.Buffer;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;

/**
 * All blacktie services should extend this class so that they can be advertised
 */
public abstract class MDBBlacktieService implements BlacktieService,
		MessageListener {
	/**
	 * A logger to log the output to.
	 */
	private static final Logger log = LogManager
			.getLogger(MDBBlacktieService.class);

	private Transport transport;

	/**
	 * Must have a no-arg constructor
	 * 
	 * @throws ConnectionException
	 * @throws JAtmiBrokerException
	 */
	public MDBBlacktieService() throws JAtmiBrokerException {
		Properties properties = null;
		AtmiBrokerClientXML xml = new AtmiBrokerClientXML();
		try {
			properties = xml.getProperties();
		} catch (Exception e) {
			throw new JAtmiBrokerException("Could not load properties", e);
		}
		transport = TransportFactory.loadTransportFactory(properties)
				.createTransport("", "");
	}

	public void onMessage(Message message) {
		try {
			BytesMessage bytesMessage = ((BytesMessage) message);
			String replyTo = message.getStringProperty("reply-to");
			int messagelength = (int) bytesMessage.getBodyLength();
			String serviceName = message.getStringProperty("serviceName");
			long flags = new Long(message.getStringProperty("messageflags"));
			int cd = new Integer(message
					.getStringProperty("messagecorrelationId"));
			byte[] bytes = new byte[(int) messagelength];
			bytesMessage.readBytes(bytes);

			// TODO HANDLE CONTROL

			// THIS IS THE FIRST CALL
			Buffer buffer = new Buffer(null, null, messagelength);
			buffer.setData(bytes);
			// TODO NO SESSIONS
			TPSVCINFO tpsvcinfo = new TPSVCINFO(serviceName, buffer, flags,
					null);

			Response response = tpservice(tpsvcinfo);
			// TODO THIS SHOULD INVOKE THE CLIENT HANDLER
			// odata.value = serviceRequest.getBytes();
			// olen.value = serviceRequest.getLength();
			Sender sender = transport.createSender(replyTo);
			sender.send("", response.getRval(), response.getRcode(), response
					.getBuffer().getData(), response.getBuffer().getLen(),
					response.getFlags(), 0);
		} catch (Throwable t) {
			log.error("Could not service the request");
		}
	}
}
