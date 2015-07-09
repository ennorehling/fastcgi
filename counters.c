#include "cgiapp.h"

#include <iniparser.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define unused(a) (void)a;

typedef struct app_data {
    int counter;
    char *dbname;
} app_data;

static const char *inifile;

static void reload_config(app *self) {
    dictionary *ini = iniparser_new(inifile);
    app_data *data;
    assert(self && self->data);

    data = (app_data *)self->data;
    if (ini) {
        const char *str;
        str = iniparser_getstring(ini, "counters:database", 0);
        if (str) {
            data->dbname = strdup(str);
        }
        iniparser_free(ini);
    }
}

static int init(app * self) {
    app_data *data;
    assert(self);
    data = (app_data *)malloc(sizeof(app_data));
    data->counter = 0;
    data->dbname = 0;
    self->data = data;
    if (inifile) {
        reload_config(self);
    }
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
    if (argc>1) {
        inifile = argv[1];
    }
    return &the_app;
}
