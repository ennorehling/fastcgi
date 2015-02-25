#include "cgiapp.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <critbit.h>


#define MAXRESULTS 16

typedef struct payload {
    critbit_tree words;
} payload;

void signal_handler(int sig);

int init(void * self)
{
    const char * words[] = { "hodor", "houdini", "home", "hate", "foo", "foobar", 0 };
    payload *pl = (payload *)self;
    assert(self);
    signal(SIGINT, signal_handler);
    signal(SIGHUP, signal_handler);
    for (int i=0;words[i];++i) {
        cb_insert(&pl->words, words[i], 1+strlen(words[i]));
    }
    return 0;
}

const char * get_prefix(const char *path) {
    const char * result = strrchr(path, '/');
    return result ? result+1 : 0;
}

int process(void *self, FCGX_Request *req)
{
    const void * results[MAXRESULTS];
    const char *script, *prefix;
    int nresults;
    payload *pl = (payload *)self;
    assert(self && req);

    script = FCGX_GetParam("SCRIPT_NAME", req->envp);
    prefix = get_prefix(script);
    printf("request from %s for %s\n",
           FCGX_GetParam("REMOTE_ADDR", req->envp), prefix);
    
    nresults = cb_find_prefix(&pl->words, prefix, strlen(prefix), results, MAXRESULTS, 0);
    if (nresults) {
        FCGX_FPrintF(req->out,
                     "Status: 200 OK\r\n"
                     "Content-Type: text/plain\r\n"
                     "\r\n");
        for (int i=0; i!=nresults; ++i) {
            const char * str = (const char *)results[i];
            FCGX_PutS(str, req->out);
            FCGX_PutChar('\n', req->out);
        }
    } else {
        FCGX_FPrintF(req->out,
                     "Status: 404 Not Found\r\n\r\n\n");
    }
    return 0;
}

struct app myapp = {
    0, init, 0, process
};

void signal_handler(int sig) {
    if (sig==SIGINT) {
        printf("received SIGINT\n");
        // save_counters();
        abort();
    }
    if (sig==SIGHUP) {
        printf("received SIGHUP\n");
        // save_counters();
    }
}

struct app * create_app(void) {
    myapp.data = calloc(1, sizeof(payload));
    return &myapp;
}
