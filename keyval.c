#include "cgiapp.h"
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <critbit.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct payload {
    critbit_tree data;
    FILE * binlog;
} payload;

void signal_handler(int sig);

const char * get_prefix(const char *path) {
    const char * result = strrchr(path, '/');
    return result ? result+1 : 0;
}

int http_success(FCGX_Stream *out, const char *body) {
    size_t len = strlen(body);
    FCGX_FPrintF(out,
                 "Status: 200 OK\r\n"
                 "Content-Type: text/plain\r\n"
                 "Content-Length: %u\r\n"
                 "\r\n"
                 "%s", (unsigned int)len, body);
    return 200;
}

int http_not_found(FCGX_Stream *out, const char *body) {
    int http_result = 404;
    size_t len = body ? strlen(body) : 0;

    FCGX_FPrintF(out,
                 "Status: %d Not Found\r\n"
                 "Content-Type: text/plain\r\n"
                 "Content-Length: %u\r\n"
                 "\r\n"
                 "%s", http_result, (unsigned int)len, body ? body : "");
    return http_result;
}

void get_key(FCGX_Request *req, payload *pl, const char *key) {
    const void *matches[2];
    int result;

    result = cb_find_prefix(&pl->data, key, strlen(key)+1, matches, 2, 0);
    printf("found %d matches for %s\n", result, key);
    if (result==1) {
        char value[1024];
        cb_get_kv(matches[0], value, sizeof(value));
        http_success(req->out, value);
    }
    else {
        http_not_found(req->out, "");
    }
}

void set_key(FCGX_Request *req, payload *pl, const char *key) {
    char data[2048], *buffer = data+1024;
    size_t size = 1024, len = 0;

    for (char * b=buffer; size; ) {
        int bytes;
        bytes = FCGX_GetStr(b, size, req->in);
        if (bytes > 0) {
            b+=bytes;
            size-=bytes;
            len+=bytes;
        } else {
            break;
        }
    }
    len = cb_new_kv(key, strlen(key), buffer, len, data);
    fwrite(&len, sizeof(len), 1, pl->binlog);
    fwrite(data, len, 1, pl->binlog);
    fflush(pl->binlog);
    cb_insert(&pl->data, data, len);
    http_success(req->out, "");
}

int init(void * self)
{
    payload *pl = (payload *)self;
    int fd = open("binlog", O_RDONLY);
    if (fd>0) {
        off_t fsize;
        void *logdata;
        fsize = lseek(fd, 0, SEEK_END);
        logdata = mmap(NULL, fsize, PROT_READ, MAP_PRIVATE, fd, 0);
        if (logdata) {
            const char *data = (const char *)logdata;
            printf("reading %u bytes from binlogs\n", (unsigned)fsize);
            while(data-fsize<(const char *)logdata) {
                size_t len = *(size_t *)data;
                data+=sizeof(size_t);
                cb_insert(&pl->data, data, len);
                data+=len;
            }
            munmap(logdata, fsize);
        }
    }
    pl->binlog = fopen("binlog", "a+");
    signal(SIGINT, signal_handler);
    signal(SIGHUP, signal_handler);
    return 0;
}

void done(void *self) {
    payload *pl = (payload *)self;
    fclose(pl->binlog);
    free(self);
}

int process(void *self, FCGX_Request *req)
{
    const char *script, *prefix, *method;
    payload *pl = (payload *)self;
    assert(self && req);

    method = FCGX_GetParam("REQUEST_METHOD", req->envp);
    script = FCGX_GetParam("SCRIPT_NAME", req->envp);
    prefix = get_prefix(script);

    printf("%s request for %s\n", method, prefix);
    if (strcmp(method, "GET")==0) {
        get_key(req, pl, prefix);
    }
    else if (strcmp(method, "POST")==0) {
        set_key(req, pl, prefix);
    }
    else {
        // invalid method
        http_not_found(req->out, NULL);
    }
    return 0;
}

struct app myapp = {
    0, init, done, process
};

void signal_handler(int sig) {
    if (sig==SIGINT) {
        printf("received SIGINT\n");
        done(myapp.data);
        abort();
    }
    if (sig==SIGHUP) {
        printf("received SIGHUP\n");
        fflush(((payload *)myapp.data)->binlog);
    }
}

struct app * create_app(void) {
    myapp.data = calloc(1, sizeof(payload));
    return &myapp;
}
