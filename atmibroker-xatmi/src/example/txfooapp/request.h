#ifndef __REQUEST_H
#define __REQUEST_H

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
	char op[1];
	enum TX_TYPE txtype;
} test_req_t;

#endif /* __REQUEST_H */
