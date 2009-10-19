package org.jboss.blacktie.jatmibroker.tx;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.xatmi.BlacktieService;
import org.jboss.blacktie.jatmibroker.xatmi.Buffer;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;

public class TestRollbackOnlyTprecvTPEVDISCONIMMService implements
		BlacktieService {
	private static final Logger log = LogManager
			.getLogger(TestRollbackOnlyTprecvTPEVDISCONIMMService.class);

	public Response tpservice(TPSVCINFO svcinfo) {
		try {
			log.info("test_tprecv_TPEV_DISCONIMM_service");
			Buffer status = svcinfo.getSession().tprecv(0);
			TXINFO txinfo = new TXINFO();
			int inTx = TX.tx_info(txinfo);
			boolean rbkOnly = (txinfo.transaction_state == TX.TX_ROLLBACK_ONLY);
			log.info("status=%d, inTx=%d, rbkOnly=%d" + status + " " + inTx
					+ " " + rbkOnly);
			return null;
		} catch (ConnectionException e) {
			return new Response(Connection.TPFAIL, Connection.TPEITYPE, null,
					0, 0);
		}
	}
}
