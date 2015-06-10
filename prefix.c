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

static void signal_handler(int sig) {
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

static const char *wordlist = "/usr/share/dict/words";

static int init(void * self)
{
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

static const char * get_prefix(const char *path) {
    const char * result = strrchr(path, '/');
    return result ? result+1 : 0;
}

static int process(void *self, FCGX_Request *req)
{
    const char * types[] = { "text/plain", "application/json" };
    const void * results[MAXRESULTS];
    const char *script, *prefix, *accept, *type;
    int nresults, format = FORMAT_PLAIN;
    payload *pl = (payload *)self;
    assert(self && req);

    script = FCGX_GetParam("PATH_INFO", req->envp);
    prefix = get_prefix(script);
    nresults = prefix ? cb_find_prefix(&pl->words, prefix, strlen(prefix), results, MAXRESULTS, 0) : 0;

    /* really dumb conent negotiation: */
    accept = FCGX_GetParam("HTTP_ACCEPT", req->envp);
    if (accept && (strstr(accept, "json") || strstr(accept, "javascript"))) {
        format = FORMAT_JSON;
    }
    type = types[format];
    
    printf("request from %s for %s as %s, %d results\n",
           FCGX_GetParam("REMOTE_ADDR", req->envp), prefix, accept, nresults);

    FCGX_FPrintF(req->out,
                 "Status: 200 OK\r\n"
                 "Content-Type: %s\r\n"
                 "\r\n", type);
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

static struct app myapp = {
    0, init, 0, process
};

struct app * create_app(int argc, char **argv) {
    if (argc>1) {
        wordlist = argv[1];
    }
    myapp.data = calloc(1, sizeof(payload));
    return &myapp;
}
