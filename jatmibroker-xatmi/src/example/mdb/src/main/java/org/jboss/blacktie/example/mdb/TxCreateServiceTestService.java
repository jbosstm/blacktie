package org.jboss.blacktie.example.mdb;

import javax.ejb.ActivationConfigProperty;
import javax.ejb.MessageDriven;

import org.jboss.blacktie.jatmibroker.mdb.MDBBlacktieService;
import org.jboss.blacktie.jatmibroker.xatmi.*;
import org.jboss.blacktie.jatmibroker.transport.JtsTransactionImple;
import org.jboss.blacktie.jatmibroker.conf.ConfigurationException;
import org.jboss.ejb3.annotation.Depends;

import org.jboss.blacktie.jatmibroker.jab.JABTransaction;
import org.jboss.blacktie.jatmibroker.jab.JABException;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

@javax.ejb.TransactionAttribute(javax.ejb.TransactionAttributeType.NOT_SUPPORTED)
@MessageDriven(activationConfig =
{
@ActivationConfigProperty(propertyName="destinationType", propertyValue="javax.jms.Queue"),
@ActivationConfigProperty(propertyName="destination", propertyValue="queue/TxCreateService")
})
@Depends ("jboss.messaging.destination:service=Queue,name=TxCreateService")
public class TxCreateServiceTestService extends MDBBlacktieService implements javax.jms.MessageListener {
	private static final Logger log = LogManager.getLogger(TxCreateServiceTestService.class);

	public TxCreateServiceTestService() {
		super("TxCreateService");
	}

	public void setUp() throws ConnectionException, ConfigurationException
    {
		ConnectionFactory connectionFactory = ConnectionFactory
				.getConnectionFactory();
		Connection connection = connectionFactory.getConnection();
	}

	public static String serviceRequest(String args) {
		if (!JtsTransactionImple.begin())
			return "Service could not start a new transaction";

		String ior = JtsTransactionImple.getTransactionIOR();
		log.debug("TxCreateService ior: " + ior);
		return args;
	}

	public Response tpservice(TPSVCINFO svcinfo) {
		byte[] rcvd = svcinfo.getBuffer().getData();
		String resp = serviceRequest(new String(rcvd));
        Buffer buffer = new Buffer(null, null);
		buffer.setData(resp.getBytes());
		return new Response((short) 0, 0, buffer, resp.length(), 0);
	}
}
