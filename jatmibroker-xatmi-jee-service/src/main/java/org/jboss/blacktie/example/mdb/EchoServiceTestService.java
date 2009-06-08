package org.jboss.blacktie.example.mdb;

import javax.ejb.ActivationConfigProperty;
import javax.ejb.MessageDriven;

import org.jboss.blacktie.jatmibroker.mdb.MDBBlacktieService;
import org.jboss.blacktie.jatmibroker.xatmi.Buffer;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;
import org.jboss.ejb3.annotation.Depends;

@MessageDriven(activationConfig =
{
@ActivationConfigProperty(propertyName="destinationType", propertyValue="javax.jms.Queue"),
@ActivationConfigProperty(propertyName="destination", propertyValue="queue/EchoService")
})
@Depends ("jboss.mq.destination:service=Queue,name=EchoService")
public class EchoServiceTestService extends MDBBlacktieService implements javax.jms.MessageListener {

	public EchoServiceTestService() {
		super("EchoService");
	}

	public Response tpservice(TPSVCINFO svcinfo) {
		byte[] rcvd = svcinfo.getBuffer().getData();
		Buffer buffer = new Buffer(null, null);
		buffer.setData(rcvd);
		return new Response((short) 0, 0, buffer, rcvd.length, 0);
	}
}
