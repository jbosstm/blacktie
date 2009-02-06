package org.jboss.blacktie.example.ejb;

import java.rmi.RemoteException;
import java.util.Properties;

import javax.ejb.EJBException;
import javax.ejb.SessionBean;

import org.jboss.blacktie.jatmibroker.ejb.connector.Connector;
import org.jboss.blacktie.jatmibroker.ejb.connector.ConnectorException;
import org.jboss.blacktie.jatmibroker.ejb.connector.ConnectorFactory;
import org.jboss.blacktie.jatmibroker.ejb.connector.Response;
import org.jboss.blacktie.jatmibroker.ejb.connector.TPSVCINFO;
import org.jboss.blacktie.jatmibroker.ejb.connector.buffers.Buffer;
import org.jboss.blacktie.jatmibroker.ejb.connector.buffers.X_OCTET;
import org.jboss.blacktie.jatmibroker.ejb.connector.ejb.AbstractBlacktieService;
import org.jboss.blacktie.jatmibroker.ejb.connector.impl.ConnectorFactoryImpl;

/**
 * This is an example EJB that can advertise itself using configuration and can
 * respond to requests by returning the same value back.
 */
public class BarServiceBean extends AbstractBlacktieService implements SessionBean {

	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;

	static {
		try {
			ConnectorFactory cf = ConnectorFactoryImpl.getConnectorFactory();
			Connector connector = cf.getConnector();
			connector.tpadvertise("BAR", BarServiceBean.class);
		} catch (ConnectorException e) {
			throw new Error(e);
		}
	}

	public Response tpservice(TPSVCINFO svcinfo) {
		Buffer data = svcinfo.getData();
		Buffer buffer = new X_OCTET(data.getSize());
		buffer.setData(data.getData());
		Response response = new Response(buffer);
		return response;
	}

	public String testEJBCall() throws RemoteException {
		try {
			Properties properties = new Properties();
			properties.put("blacktie.orb.args", "2");
			properties.put("blacktie.orb.arg.1", "-ORBInitRef");
			properties.put("blacktie.orb.arg.2", "NameService=corbaloc::localhost:3528/NameService");
			properties.put("blacktie.domain.name", "jboss");
			properties.put("blacktie.server.name", "example");
			ConnectorFactory connectorFactory = ConnectorFactoryImpl.getConnectorFactory(properties);
			Connector connector = connectorFactory.getConnector();
			byte[] echo = "echo".getBytes();
			Buffer buffer = new X_OCTET(echo.length);
			buffer.setData(echo);
			Response response = connector.tpcall("BAR", buffer, 0);
			Buffer responseBuffer = response.getResponse();
			byte[] responseData = responseBuffer.getData();
			return new String(responseData);
		} catch (Throwable t) {
			t.printStackTrace();
			return null;
		}
	}

	/**
	 * Currently a no-op
	 */
	public void setSessionContext(javax.ejb.SessionContext ctx) {
	}

	/**
	 * This will unadvertise the service
	 */
	public void ejbRemove() throws EJBException {
		try {
			ConnectorFactory cf = ConnectorFactoryImpl.getConnectorFactory();
			Connector connector = cf.getConnector();
			connector.tpunadvertise("BAR");
		} catch (ConnectorException e) {
			throw new EJBException(e);
		}
	}

	/**
	 * NO-OP
	 */
	public void ejbCreate() {

	}

	/**
	 * NO-OP
	 */
	public void ejbActivate() {
	}

	/**
	 * NO-OP
	 */
	public void ejbPassivate() {
	}
}
