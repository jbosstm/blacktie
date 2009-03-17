#ifndef _TESTRO_H
#define _TESTRO_H

#include "utilitiesMacro.h"

enum XA_OP {
        O_XA_OPEN,
        O_XA_CLOSE,
        O_XA_START,
        O_XA_END,
        O_XA_ROLLBACK,
        O_XA_PREPARE,
        O_XA_COMMIT,
        O_XA_RECOVER,
        O_XA_FORGET,
        O_XA_COMPLETE
};

enum X_FAULT {
        F_NONE,
        F_HALT,
        F_DELAY
};

/*
 * a definition of a fault in the XA protocol for testing purposes
 */
typedef struct UTILITIES_DLL fault {
        int id;                 // unique id for this fault specification
        int rmid;               // RM id
        enum XA_OP op;          // the XA method that this fault applies to
        int rc;                 // the value that the XA method should return
        enum X_FAULT xf;        // optional extra processing
        void *arg;              // optional arg for enum X_FAULT

        struct fault *next;
} fault_t;

#ifdef __cplusplus
extern "C" {
#endif
extern UTILITIES_DLL int dummy_rm_add_fault(fault_t *);
extern UTILITIES_DLL int dummy_rm_del_fault(int);
#ifdef __cplusplus
}
#endif

#endif /* _TESTRO_H */

