/* Based on the code samples in the Oracle Streams Advanced Queuing User's Guide and Reference */

#include <oci.h>
#include <xa.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "userlogc.h"

/* 
 * If a session wont release a lock here's how to recover
 * 
 * find the lock (lock type should be TX):
 * SELECT sid,type,id1,lmode,request from v$lock;
 *
 * find the lock owner:
 * SELECT s.sid, s.serial#, s.osuser, s.program FROM v$session s where s.sid=140;
 * end the session that owns the lock:
 * ALTER SYSTEM KILL SESSION 'sid,serial#';
 *
 */
static text *dbname = (text *) "blacktie";

/* log a message */
static void logit(int debug, const char * format, ...) {
	char str[1024];
	va_list args;
	va_start(args, format);
	vsnprintf(str, sizeof (str), format, args);
	va_end(args);

	if (debug)
		userlogc_debug(str);
	else
		userlogc(str);
}

/* check if XA operation succeeded */
#define checkXAerr(action, funcname)                        \
    if ((action) != XA_OK)                                  \
    {                                                       \
        logit(0, "%s failed err %d!\n", funcname, action);    \
        exit(-1);                                           \
    } else

static void fatal(const char *msg) {
    logit(0, msg);
    exit (-1);
}

/* figure out the oracle error code and message corresponding to an Oracle error */
static void get_error(char **rbuf, long bufsz, dvoid *errhp, sword status) {
    if  (status == OCI_ERROR) {
        text buf[256];
        sb4 err = 0;
        (void) OCIErrorGet(errhp, (ub4) 1, (text *) NULL, &err, (text *) buf, (ub4) bufsz, OCI_HTYPE_ERROR);

		strncpy(*rbuf, (const char *) buf, (size_t) bufsz);
		*((*rbuf) + bufsz - 1) = 0;
    } else {
        sprintf(*rbuf, "OCI error: %d\n", (int) status);	// assume *buf is big enough
    }
}

static void show_error(dvoid *errhp, sword status) {
    if  (status == OCI_ERROR) {
        text buf[256];
        sb4 err = 0;
        (void) OCIErrorGet(errhp, (ub4) 1, (text *) NULL, &err, buf, (ub4) sizeof(buf), OCI_HTYPE_ERROR);
        logit(0, "OCI error %d: %s\n", (int) err, buf);
    } else {
        logit(0, "OCI error: %d\n", (int) status);
    }
}

#if 0
static void show_error(dvoid *errhp, sword status) {
    if  (status == OCI_ERROR) {
        text buf[256];
        sb4 err = 0;
        (void) OCIErrorGet(errhp, (ub4) 1, (text *) NULL, &err, buf, (ub4) sizeof(buf), OCI_HTYPE_ERROR);
        logit(0, "OCI error %d: %s\n", (int) err, buf);
    } else {
        logit(0, "OCI error: %d\n", (int) status);
    }
}
#endif

/* execute an SQL statement for the given service context */
static int doSql(OCISvcCtx *svcCtx, OCIStmt *stmthp, OCIError *errhp, text *sql) {
    sword status = OCIStmtPrepare(stmthp, errhp, (text *) sql, (ub4) strlen((char *)sql),
        (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);

	logit(1, "executing statement: %s\n", sql);

    if (status == OCI_SUCCESS)
        status = OCIStmtExecute(svcCtx, stmthp, errhp, (ub4) 1, (ub4) 0,
                                (CONST OCISnapshot *) NULL,
                                (OCISnapshot *) NULL, OCI_DEFAULT);

    if (status != OCI_SUCCESS)
        show_error(errhp, status);

    return status;

}

/* update test */
static int doUpdate(OCISvcCtx *svcCtx, OCIStmt *stmthp, OCIError *errhp) {
    text *sql = (text *) "UPDATE EMP SET JOB='DIRECTOR' WHERE EMPNO=8000" ;
    return doSql(svcCtx, stmthp, errhp, sql);
}
/* insert test */
static int doInsert(OCISvcCtx *svcCtx, OCIStmt *stmthp, OCIError *errhp) {
    text *sql = (text *) "INSERT INTO EMP (EMPNO,ENAME,JOB,MGR,HIREDATE,SAL,COMM,DEPTNO)"
        " VALUES ('8000','Jim','Janitor','7902','17-DEC-80','900','0','20')";
    return doSql(svcCtx, stmthp, errhp, sql);
}
/* delete test */
static int doDelete(OCISvcCtx *svcCtx, OCIStmt *stmthp, OCIError *errhp) {
    text *sql = (text *) "DELETE FROM EMP WHERE EMPNO = 8000" ;
    return doSql(svcCtx, stmthp, errhp, sql);
}

/* select test */
static int doSelect(OCISvcCtx *svcCtx, OCIStmt *stmthp, OCIError *errhp, int expectedRcnt) {
    text *sql = (text *) "SELECT ENAME,JOB FROM EMP WHERE EMPNO = 8000" ;
    char p_data [30];
    int rcnt = 0;

    OCIDefine *stmtdef = (OCIDefine *) 0;

    sword status = OCIStmtPrepare(stmthp, errhp, (text *) sql, (ub4) strlen((char *)sql),
        (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);

    if (status == OCI_SUCCESS) {
        status = OCIDefineByPos(stmthp, &stmtdef, errhp, 1, (dvoid *)&p_data,(sword) 20, SQLT_STR,
            (dvoid *) 0, (ub2 *)0, (ub2 *)0, (ub4) OCI_DEFAULT);

        status = OCIStmtExecute(svcCtx, stmthp, errhp, (ub4) 1,(ub4) 0, (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL, OCI_DEFAULT);
    }

    if (status != OCI_SUCCESS && status != OCI_NO_DATA) {
        show_error(errhp, status);
    } else {
        while ((status = OCIStmtFetch(stmthp, errhp, (ub4) 1, (ub4) OCI_FETCH_NEXT,
                                  (ub4) OCI_DEFAULT)) == OCI_SUCCESS || status == OCI_SUCCESS_WITH_INFO) {
            if (status == OCI_NO_DATA)
                break;
            logit(1, "%s\n",p_data);
            rcnt += 1;
        }
    }

    if (rcnt != expectedRcnt)
        logit(1, "select returned %d records - should have returned %d\n", rcnt, expectedRcnt);

    return 0;
}

static int toInt(char *arg) {
	return atoi(arg);
}

/* the test: insert some data, update it and finally delete it */
static sword doWork(char op, char *arg, OCISvcCtx *svcCtx, OCIStmt *stmthp, OCIError *errhp, char **rdata, long rdata_sz) {
	sword status;

	if (op == '0') {
    	status = doInsert(svcCtx, stmthp, errhp);
	} else if (op == '1') {
		status = doSelect(svcCtx, stmthp, errhp, toInt(arg));
	} else if (op == '2') {
    	status = doUpdate(svcCtx, stmthp, errhp);
	} else if (op == '3') {
    	status = doDelete(svcCtx, stmthp, errhp);
	}

	get_error(rdata, rdata_sz, errhp, status);

	return status;
}

/**
 * test that blacktie correctly drives oracle xa
 * Pre-requisite: - there is an active transaction (meaning that that there is open XA connection)
 */
int ora_test(char op, char *arg, char **rdata, long rdata_sz)
{
    OCIStmt *stmthp;
    OCIError *errhp;
    OCIEnv *xaEnv;
    OCISvcCtx *svcCtx;
	sword status;

    /* opening an XA connection creates an environment and service context */
    xaEnv = (struct OCIEnv *) xaoEnv(dbname) ;
    svcCtx = (struct OCISvcCtx *) xaoSvcCtx(dbname);

    if (!xaEnv || !svcCtx)
        fatal("Unable to obtain env and/or service context!\n");

    /* initialise OCI handles */
    if (OCI_SUCCESS != OCIHandleAlloc((dvoid *)xaEnv, (dvoid **)&errhp,
        OCI_HTYPE_ERROR, 0, (dvoid **)0))
        fatal("Unable to allocate statement handle\n");

    if (OCI_SUCCESS != OCIHandleAlloc((dvoid *)xaEnv, (dvoid **)&stmthp,
        OCI_HTYPE_STMT, 0, (dvoid **)0))
        fatal("Unable to allocate error handle\n");

    status = doWork(op, arg, svcCtx, stmthp, errhp, rdata, rdata_sz);

    return (status == OCI_SUCCESS);
}
