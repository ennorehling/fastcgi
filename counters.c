#include "cgiapp.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define unused(a) (void)a;

static int init(struct app * self) {
    int * counter;
    assert(self);
    counter = (int *)malloc(sizeof(int));
    *counter = 0;
    self->data = counter;
    return 0;
}

static void done(struct app *self) {
    int *counter;
    assert(self);
    counter = (int *)self->data;
    free(counter);
}

static int process(struct app *self, FCGX_Request *req) {
    const char *method, *path, *message = "OK";
    int *counter;
    int status;

    method = FCGX_GetParam("REQUEST_METHOD", req->envp);
    path = FCGX_GetParam("PATH_INFO", req->envp);
    assert(self && req);
    counter = (int *)self->data;
    if (method[0]=='G') {
        if (strstr(path, "death.php")) {
            // special-case hack
            ++*counter;
        }
        status = 200;
    } else if (method[0]=='P') {
        ++*counter;
        status = 200;
    } else {
        message = "Method Not Allowed";
        status = 405;
    }
    FCGX_FPrintF(req->out,
                 "Status: %d %s\r\n"
                 "Content-Type: text/plain\r\n"
                 "\r\n%d\n", status, message, *counter);
    return 0;
}

static struct app the_app = {
    0, init, done, process
};

struct app * create_app(int argc, char ** argv) {
    assert(argc>=0);
    unused(argv);
    return &the_app;
}
