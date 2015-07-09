#include "cgiapp.h"

#include <assert.h>
#include <stdlib.h>

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
    int *counter;
    assert(self && req);
    counter = (int *)self->data;
    ++*counter;
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
