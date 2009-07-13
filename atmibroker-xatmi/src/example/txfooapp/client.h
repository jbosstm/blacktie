#ifndef __CLIENT_H
#define __CLIENT_H

#include "request.h"

extern int null_access(test_req_t *req, test_req_t *resp);
extern int ora_access(test_req_t *req, test_req_t *resp);
extern int bdb_access(test_req_t *req, test_req_t *resp);

typedef struct product {
    int id;
    int (*access)(test_req_t *req, test_req_t *resp);
} product_t;

product_t products[] = {
    {0, null_access},
    {1, ora_access},
    {2, bdb_access},
    {-1, 0},
};

#endif /* __CLIENT_H */
