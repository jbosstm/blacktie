package org.jboss.blacktie.jatmibroker.xatmi.ejb;

import java.util.ArrayList;
import java.util.List;
import java.util.Properties;

import javax.jms.BytesMessage;
import javax.jms.Message;
import javax.jms.MessageListener;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.OrbManagement;
import org.jboss.blacktie.jatmibroker.core.conf.AtmiBrokerServerXML;
import org.jboss.blacktie.jatmibroker.core.corba.SenderImpl;
import org.jboss.blacktie.jatmibroker.core.proxy.Sender;
import org.jboss.blacktie.jatmibroker.xatmi.BlacktieService;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectorException;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;
import org.jboss.blacktie.jatmibroker.xatmi.buffers.Buffer;

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

	/**
	 * Create the orb management function for responding to data.
	 */
	private OrbManagement orbManagement;

	/**
	 * Must have a no-arg constructor
	 * 
	 * @throws ConnectorException
	 */
	public BlacktieMDBService() throws ConnectorException {
		Properties properties = new Properties();
		AtmiBrokerServerXML server = new AtmiBrokerServerXML(properties);
		String configDir = System.getProperty("blacktie.config.dir");
		try {
			server.getProperties(configDir);
		} catch (Exception e) {
			throw new ConnectorException(-1, "Could not load properties", e);
		}

		String domainName = properties.getProperty("blacktie.domain.name");
		int numberOfOrbArgs = Integer.parseInt(properties
				.getProperty("blacktie.orb.args"));
		List<String> orbArgs = new ArrayList<String>(numberOfOrbArgs);
		for (int i = 1; i <= numberOfOrbArgs; i++) {
			orbArgs.add(properties.getProperty("blacktie.orb.arg." + i));
		}
		String[] args = orbArgs.toArray(new String[] {});
		try {
			orbManagement = new OrbManagement(args, domainName, true);
		} catch (Throwable t) {
			throw new ConnectorException(-1, "Could not connect to orb", t);
		}
	}

	public void onMessage(Message message) {
		try {
			BytesMessage bytesMessage = ((BytesMessage) message);
			String replyTo = message.getStringProperty("reply-to");
			long messagelength = bytesMessage.getBodyLength();
			String serviceName = message.getStringProperty("serviceName");
			long messageflags = new Long(message
					.getStringProperty("messageflags"));
			long messagerval = new Long(message
					.getStringProperty("messagerval"));
			long messagercode = new Long(message
					.getStringProperty("messagercode"));
			long messagecorrelationId = new Long(message
					.getStringProperty("messagecorrelationId"));
			byte[] bytes = new byte[(int) messagelength];
			bytesMessage.readBytes(bytes);

			Sender sender = SenderImpl.createSender(orbManagement, replyTo);

			// TODO HANDLE CONTROL
			// THIS IS THE FIRST CALL
			Buffer buffer = new Buffer("unknown", "unknown",
					(int) messagelength);
			buffer.setData(bytes);
			TPSVCINFO tpsvcinfo = new TPSVCINFO(serviceName, buffer,
					messageflags, -1);
			Response response = tpservice(tpsvcinfo);
			// TODO THIS SHOULD INVOKE THE CLIENT HANDLER
			// odata.value = serviceRequest.getBytes();
			// olen.value = serviceRequest.getLength();
			sender.send("", response.getRval(), response.getRcode(), response
					.getResponse().getData(), response.getResponse().getSize(),
					response.getFlags(), 0);
		} catch (Throwable t) {
			log.error("Could not service the request");
		}
	}
}
