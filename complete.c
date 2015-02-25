#include "cgiapp.h"
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <critbit.h>

enum {
    FORMAT_PLAIN,
    FORMAT_JSON
};

#define MAXRESULTS 16
#define MAXWORDLEN 64

typedef struct payload {
    critbit_tree words;
} payload;

void signal_handler(int sig);


int init(void * self)
{
    const char *wordlist = "/usr/share/dict/words";
    payload *pl = (payload *)self;
    FILE *F;
    assert(self);
    signal(SIGINT, signal_handler);
    signal(SIGHUP, signal_handler);
    printf("reading from %s\n", wordlist);
    F = fopen(wordlist, "rt");
    if (F) {
        int i = 0;
        for (i=0; !feof(F); ++i) {
            char word[MAXWORDLEN];
            size_t len;
            fgets(word, sizeof(word), F);
            len = strlen(word);
            while (isspace(word[len-1])) {
                --len;
            }
            word[len]=0;
            cb_insert(&pl->words, word, len+1);
        }
        printf("read %d words from %s\n", i, wordlist);
    }
    return 0;
}

const char * get_prefix(const char *path) {
    const char * result = strrchr(path, '/');
    return result ? result+1 : 0;
}

int process(void *self, FCGX_Request *req)
{
    const char * types[] = { "text/plain", "application/json" };
    const void * results[MAXRESULTS];
    const char *script, *prefix, *accept;
    int nresults, format = FORMAT_PLAIN;
    payload *pl = (payload *)self;
    assert(self && req);

    script = FCGX_GetParam("SCRIPT_NAME", req->envp);
    prefix = get_prefix(script);
    printf("request from %s for %s\n",
           FCGX_GetParam("REMOTE_ADDR", req->envp), prefix);
    
    nresults = cb_find_prefix(&pl->words, prefix, strlen(prefix), results, MAXRESULTS, 0);

    /* really dumb conent negotiation: */
    accept = FCGX_GetParam("HTTP_ACCEPT", req->envp);
    if (accept && strstr(accept, "json")) {
        format = FORMAT_JSON;
    }
    
    FCGX_FPrintF(req->out,
                 "Status: 200 OK\r\n"
                 "Content-Type: %s\r\n"
                 "\r\n", types[format]);
    if (format==FORMAT_JSON) {
        FCGX_PutChar('[', req->out);
    }
    for (int i=0; i!=nresults; ++i) {
        const char * str = (const char *)results[i];
        if (format==FORMAT_JSON) {
            FCGX_PutChar('"', req->out);
        }
        FCGX_PutS(str, req->out);
        if (format==FORMAT_JSON) {
            if (nresults==i+1) {
                FCGX_PutChar('"', req->out);
            } else {
                FCGX_PutStr("\",", 2, req->out);
            }
        } else {
            FCGX_PutChar('\n', req->out);
        }
    }
    if (format==FORMAT_JSON) {
        FCGX_PutStr("]\n", 2, req->out);
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
