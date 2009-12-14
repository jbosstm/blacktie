#include <stdio.h>
#include <string.h>
/*
 *	 TODO - work in progress. The DB2 examples hasn't been integrated yet.
 *
DB2PATH=$HOME/sqllib
LIB="lib32"
gcc -g -I$DB2PATH/include -L$DB2PATH/$LIB -ldb2 db2.c -o db2 && ./db2 1
#define DB2
 */

#ifdef DB2
#include <sqlcli.h>
#include "xa.h"
 
#include "tx/request.h"
/*
#define userlogc_warn   printf
#define userlogc_debug  printf
#define userlogc_snprintf   snprintf

typedef struct test_req {
	char db[16];
	char data[80];
	char op;
	int  id;	// request id
	int  expect;	// for testing null products (ie force success)
	int  prod;
	int txtype; //enum TX_TYPE
	int status;
} test_req_t;
*/

#ifdef DECLSPEC_DEFN
extern __declspec(dllimport) struct xa_switch_t db2xa_switch_std;
extern __declspec(dllimport) struct xa_switch_t * SQL_API_FN db2xacic_std();
#else
#define db2xa_switch_std (*db2xa_switch_std)
extern struct xa_switch_t db2xa_switch_std;
struct xa_switch_t * SQL_API_FN db2xacic_std();
#endif

int db2_access(test_req_t *req, test_req_t *resp);

long db2_xaflags()
{
	struct xa_switch_t *xasw = db2xacic_std();

	return xasw->flags; //db2xa_switch_std.flags;
}

static SQLCHAR userid[] = "";
static SQLCHAR passwd[] = "";
static SQLCHAR dbAlias[] = "BLACKTIE";   /* db instance name */
static SQLCHAR CTSQL[] = "CREATE TABLE XEMP (EMPNO integer NOT NULL PRIMARY KEY, ENAME varchar(32))";
/*static SQLCHAR DTSQL[] = "DROP TABLE XEMP";*/
static SQLCHAR ISQL[] = "INSERT INTO XEMP VALUES (?, 'Jim')";
static SQLCHAR USQL[] = "UPDATE XEMP SET ENAME='NEW_NAME' WHERE EMPNO=?";
static SQLCHAR DSQL[] = "DELETE FROM XEMP WHERE EMPNO >= ?";
static SQLCHAR SSQL[] = "select EMPNO, ENAME from XEMP WHERE EMPNO >= ?";

/*
gcc -g -I$DB2PATH/include -L$DB2PATH/$LIB -ldb2 db2.c -o db2 && ./db2 1
 */

static int check_error (int line, const char *msg, SQLCHAR sql[], SQLHDBC hdbc, SQLHSTMT hstmt) {
    SQLCHAR szSqlState[SQL_MAX_MESSAGE_LENGTH + 1];
    SQLINTEGER pfNativeError;
    SQLCHAR szErrorMsg[SQL_SQLSTATE_SIZE + 1];
    SQLSMALLINT cbErrorMsgMax = SQL_MAX_MESSAGE_LENGTH + 1;
    SQLSMALLINT pcbErrorMsg;
	SQLRETURN rc = SQLError(SQL_NULL_HENV, hdbc, hstmt, szSqlState, &pfNativeError, szErrorMsg, cbErrorMsgMax, &pcbErrorMsg);

    printf("ERROR: %s: %s (code=%d)\n", msg, sql, rc);
    if (rc != SQL_SUCCESS) {
        userlogc_warn("DB2 error: SQL state: %s db2 code: %ld (%s)\n", szSqlState, pfNativeError, szErrorMsg);
        printf("DB2 error: SQL state: %s db2 code: %ld (%s)\n", szSqlState, pfNativeError, szErrorMsg);
    }

	return(SQL_ERROR);
}
 
/* can't get SQLBindParameter to work - use string substitution for the time being */
const char *strsub(const char *src, char *dest, size_t sz, const char *match, char *rep) {
	char *s;
	size_t len;

	if ((s = strstr(src, match)) == NULL)
		return NULL;

	len = s - src;  /* length of the string upto the match */
	strncpy(dest, src, len);
	sprintf(dest+len, "%s%s", rep, s + strlen(match));

	return dest;
}

static int doSelect(SQLHENV henv, SQLHDBC hdbc, SQLHSTMT shdl, SQLCHAR sql[], int *rcnt) {
	SQLCHAR name[33];
	SQLINTEGER empno, col2sz, col1sz;

	if (SQLExecDirect (shdl, sql, SQL_NTS) != SQL_SUCCESS)
		return check_error (__LINE__, "SQLExecDirect", sql, hdbc, shdl);
 
	/* bind empno and name to columns 1 and 2 of the fetch */
	SQLBindCol(shdl, 1, SQL_C_LONG, (SQLPOINTER) &empno, (SQLINTEGER) sizeof (SQLINTEGER), (SQLINTEGER *) &col1sz);
	SQLBindCol(shdl, 2, SQL_C_CHAR, (SQLPOINTER) name, (SQLINTEGER) sizeof (name), &col2sz);
 
	*rcnt = 0;
	while (SQLFetch (shdl) == SQL_SUCCESS) {
		*rcnt += 1;
		printf("(%ld,%s)\n", empno, name);
	}

	return SQL_SUCCESS;
}

static int doSql(SQLHENV henv, SQLHDBC hdbc, SQLHSTMT shdl, SQLCHAR sql[]) {

	userlogc_debug("doSql %s\n", sql);
	if (SQLPrepare(shdl, sql, SQL_NTS) != SQL_SUCCESS)
		return check_error (__LINE__, "SQLPrepare", sql, hdbc, shdl);
	
	if (SQLExecDirect(shdl, sql, SQL_NTS) != SQL_SUCCESS)
		return check_error (__LINE__, "SQLExecDirect", sql, hdbc, shdl);

	return SQL_SUCCESS;
}

static void fini(SQLHENV henv, SQLHDBC hdbc) {
	/* clean up - free the connection and environment handles */
	SQLDisconnect (hdbc);
	SQLFreeConnect (hdbc);
	SQLFreeEnv (henv);
}

static void init(SQLHENV *henv, SQLHDBC *hdbc) {
	SQLHSTMT hstmt;	/* statement handle */

	SQLAllocEnv(henv);
	SQLAllocConnect(*henv, hdbc);

	(void) SQLConnect(*hdbc, (SQLCHAR *) dbAlias, SQL_NTS, userid, SQL_NTS, passwd, SQL_NTS);

	SQLAllocHandle(SQL_HANDLE_STMT, *hdbc, &hstmt);

/*	if (SQLExecDirect (hstmt, DTSQL, SQL_NTS) != SQL_SUCCESS)
		check_error (__LINE__, "SQLExecDirect", DTSQL, *hdbc, hstmt);
*/
	if (SQLExecDirect (hstmt, CTSQL, SQL_NTS) != SQL_SUCCESS)
		; /* check_error (__LINE__, "SQLExecDirect", CTSQL, *hdbc, hstmt);*/
 
/*	SQLTransact (*henv, *hdbc, SQL_COMMIT);*/		   /* commit create table */
	SQLFreeStmt (hstmt, SQL_DROP);
}

static SQLRETURN doWork(char op, char *arg, SQLHENV henv, SQLHDBC hdbc, SQLHSTMT shdl, test_req_t *resp)  {
	SQLRETURN status = SQL_ERROR;
	int empno = (*arg ? atoi(arg) : 8000);
	char buf1[512];
	char buf2[512];
	int rcnt = 0;   // no of matching records

	sprintf(buf2, "%d", empno);
	(resp->data)[0] = 0;

	if (op == '0') {
		(void) strsub((const char*) ISQL, buf1, sizeof (buf1), "?", buf2);
		status = doSql(henv, hdbc, shdl, (SQLCHAR *) buf1);
	} else if (op == '1') {
		(void) strsub((const char*) SSQL, buf1, sizeof (buf1), "?", buf2);
		status = doSelect(henv, hdbc, shdl, (SQLCHAR *) buf1, &rcnt);
		userlogc_snprintf(resp->data, sizeof (resp->data), "%d", rcnt);
	} else if (op == '2') {
		(void) strsub((const char*) USQL, buf1, sizeof (buf1), "?", buf2);
		status = doSql(henv, hdbc, shdl, (SQLCHAR *) buf1);
	} else if (op == '3') {
		(void) strsub((const char*) DSQL, buf1, sizeof (buf1), "?", buf2);
		status = doSql(henv, hdbc, shdl, (SQLCHAR *) buf1);
	}

/*	if (status == SQL_SUCCESS)
		SQLTransact (henv, hdbc, SQL_COMMIT);
*/

	return status;
}

int db2_access(test_req_t *req, test_req_t *resp)
{
	SQLHENV henv;	/* environment handle */
	SQLHDBC hdbc;	/* connection handle */
	SQLRETURN status;
	SQLHSTMT shdl;

	userlogc_debug("op=%c data=%s db=%s\n", req->op, req->data, req->db);

	init(&henv, &hdbc);

	SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &shdl);
	status = doWork(req->op, req->data, henv, hdbc, shdl, resp);
	SQLFreeStmt (shdl, SQL_DROP);

	fini(henv, hdbc);

	return (int) status;
}

static int xmain(int argc, char *argv[]) {
	test_req_t res = {"blacktie", "0", '1'};
	test_req_t req = {"blacktie", "0", '1'};
	const char *empno = (argc > 2 ? argv[2] : "0");

	req.op = argv[1][0];
	strcpy(req.data, empno);

	(void) db2_access(&req, &res);

	return 0;
}
#endif
