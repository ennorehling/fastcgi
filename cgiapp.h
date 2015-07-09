#pragma once
#include <fcgiapp.h>

typedef struct app {
    void *data;
    int (*init)(struct app *self);
    void (*done)(struct app *self);
    int (*process)(struct app *self, FCGX_Request* req);
} app;

struct app * create_app(int argc, char **argv);
