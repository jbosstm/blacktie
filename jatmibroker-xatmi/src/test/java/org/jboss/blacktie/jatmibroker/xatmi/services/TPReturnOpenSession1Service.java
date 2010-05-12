package org.jboss.blacktie.jatmibroker.xatmi.services;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.RunServer;
import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionFactory;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.Service;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;

public class TPReturnOpenSession1Service implements Service {
	private static final Logger log = LogManager
			.getLogger(TPReturnOpenSession1Service.class);

	public Response tpservice(TPSVCINFO svcinfo) throws ConfigurationException,
			ConnectionException {
		log.info("testtpreturn_service_opensession1");

		ConnectionFactory connectionFactory = ConnectionFactory
				.getConnectionFactory();
		Connection connection = connectionFactory.getConnection();
		connection.tpacall(RunServer.getServiceNameTestTPReturn2(), svcinfo
				.getBuffer(), svcinfo.getLen(), svcinfo.getFlags());
		return new Response(Connection.TPSUCCESS, 0, svcinfo.getBuffer(),
				svcinfo.getLen(), 0);
	}
}
