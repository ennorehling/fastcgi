#include "cgiapp.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#define unused(a) (void)a;

static void signal_handler(int sig);
static void db_write(const char *filename);
static void db_read(const char *filename);

static const char * dbname = "counters.db";
static const char * version = "counters version 1.0 (c) Enno Rehling 2015";

static int init(struct app * self) {
    int * counter;
    assert(self);
    counter = (int *)malloc(sizeof(int));
    *counter = 0;
    self->data = counter;
    db_read(dbname);
    signal(SIGINT, signal_handler);
    signal(SIGHUP, signal_handler);
    return 0;
}

static void done(struct app *self) {
    assert(self);
    
    db_write(dbname);
    free(self->data);
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
    if ((*counter & 0xF)==0) {
        db_write(dbname);
    }
    return 0;
}

static struct app the_app = {
    0, init, done, process
};

static void db_write(const char *filename) {
    FILE *F = fopen(filename, "w");
    int *counter = (int *)the_app.data;
    if (F && counter) {
        fwrite(counter, sizeof(int), 1, F);
        fclose(F);
    }
}

static void db_read(const char *filename) {
    FILE *F = fopen(filename, "r");
    int *counter = (int *)the_app.data;
    if (F && counter) {
        fread(counter, sizeof(int), 1, F);
        fclose(F);
    }
}

static void signal_handler(int sig) {
    if (sig==SIGINT) {
        printf("received SIGINT\n");
        done(&the_app);
        abort();
    }
    if (sig==SIGHUP) {
        printf("received SIGHUP\n");
        db_write(dbname);
    }
}

struct app * create_app(int argc, char ** argv) {
    int i;
    assert(argc>=0);
    for(i=1;i!=argc;++i) {
        if (strcmp(argv[i], "-v")==0) {
            fputs(version, stderr);
        }
        else dbname = argv[i];
    }
    return &the_app;
}
