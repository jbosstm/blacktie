#ifndef __REQUEST_H
#define __REQUEST_H

enum DB_TYPE {
    ORACLE,
    BDB,
};

enum TX_TYPE {
	TX_TYPE_BEGIN,
	TX_TYPE_COMMIT,
	TX_TYPE_ABORT,
	TX_TYPE_BEGIN_COMMIT,
	TX_TYPE_BEGIN_ABORT,
	TX_TYPE_NONE,
};

typedef struct test_req {
	char db[16];
	char data[80];
	char op;
	enum DB_TYPE prod;
	enum TX_TYPE txtype;
	int status;
} test_req_t;

void fatal(const char *msg);
int fail(const char *reason, int ret);
void logit(int debug, const char * format, ...);
char * get_buf(const char *data, const char *dbfile, enum TX_TYPE txtype);
test_req_t * get_tbuf(const char *data, const char *dbfile, char op, enum DB_TYPE prod, enum TX_TYPE txtype);

#endif /* __REQUEST_H */
