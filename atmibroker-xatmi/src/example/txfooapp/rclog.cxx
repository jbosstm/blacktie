#include <stdio.h>

#include <XARecoveryLog.h>

#define IOR(rr) ((char *) (rr + 1))

/*
 * This example shows how to dump the transaction recovery log. Compile as follows:
 *
 * g++ -I$BLACKTIE_BIN_DIR/include -L$BLACKTIE_BIN_DIR/lib -latmibroker-tx rclog.cxx
 */

int
main(int argc, char* argv[])
{
    if (argc <= 1) {
        fprintf(stderr, "syntax %s <recovery log file path>\n", argv[0]);
        return -1;
    }

    XARecoveryLog log(argv[1]);

    for (rrec_t* rr = log.find_next(0); rr; rr = log.find_next(rr)) {
		XID &xid = rr->xid;

        fprintf(stdout, "XID=%ld:%ld:%ld:%s IOR=%s\n", xid.formatID, xid.gtrid_length, xid.bqual_length,
			(char *) (xid.data + xid.gtrid_length), IOR(rr));
	}

    return 0;
}
