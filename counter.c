#include <fcgiapp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <assert.h>

#define NUM_COUNTERS 8
static int counters[NUM_COUNTERS];
static int num_counters = NUM_COUNTERS;

int counter_process(FCGX_Request *request, int n) {
    assert(n>=0 && n<num_counters);
    FCGX_FPrintF(request->out,
                 "Status: 200 OK\r\n"
                 "Content-type: text/plain\r\n"
                 "\r\n"
                 "%d\n", ++counters[n]);
    return 0;
}

const char * filename(const char *path) {
    const char * result = strrchr(path, '/');
    return result ? result+1 : 0;
}

void save_counters(void) {
    FILE * F = fopen("counters.dat", "wb");
    fwrite(&num_counters, sizeof(int), 1, F);
    fwrite(counters, sizeof(int), num_counters, F);
    fclose(F);
}

void load_counters(void) {
    int n;
    FILE * F = fopen("counters.dat", "rb");
    if (F) {
        fread(&n, sizeof(int), 1, F);
        if (n>num_counters) n = num_counters;
        fread(counters, sizeof(int), n, F);
        fclose(F);
    }
}

void signal_handler(int sig) {
    if (sig==SIGINT) {
        printf("received SIGINT\n");
        save_counters();
        abort();
    }
    if (sig==SIGHUP) {
        printf("received SIGHUP\n");
        save_counters();
    }
}

int main(void)
{
    FCGX_Request request;

    signal(SIGINT, signal_handler);
    signal(SIGHUP, signal_handler);
    
    load_counters();
    
    FCGX_Init();
    FCGX_InitRequest(&request, 0, 0);

    while(FCGX_Accept_r(&request) == 0) {
        const char *script = FCGX_GetParam("SCRIPT_NAME", request.envp);
        const char *name = filename(script);
        int n = atoi(name);
        printf("request from %s for %d\n",
              FCGX_GetParam("REMOTE_ADDR", request.envp), n);
        if (n < 0 || n >= num_counters) {
            FCGX_FPrintF(request.out,
                         "Status: 404 Not Found\r\n\r\n\n");
        } else {
            counter_process(&request, n);
        }
    }
    return 0;
}
