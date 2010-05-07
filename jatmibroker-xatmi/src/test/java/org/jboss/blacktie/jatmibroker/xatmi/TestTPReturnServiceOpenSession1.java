package org.jboss.blacktie.jatmibroker.xatmi;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.RunServer;

public class TestTPReturnServiceOpenSession1 implements BlacktieService {
	private static final Logger log = LogManager
			.getLogger(TestTPReturnServiceOpenSession1.class);

	public Response tpservice(TPSVCINFO svcinfo) {
		log.info("testtpreturn_service_opensession1");

		try {
			ConnectionFactory connectionFactory = ConnectionFactory
					.getConnectionFactory();
			Connection connection = connectionFactory.getConnection();
			connection.tpacall(RunServer.getServiceNameTestTPReturn2(), svcinfo
					.getBuffer(), svcinfo.getLen(), svcinfo.getFlags());
			return new Response(Connection.TPSUCCESS, 0, svcinfo.getBuffer(),
					svcinfo.getLen(), 0);
		} catch (Throwable t) {
			return new Response(Connection.TPFAIL, -1, svcinfo.getBuffer(),
					svcinfo.getLen(), 0);
		}
	}
}
