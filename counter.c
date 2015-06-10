#include <fcgiapp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <assert.h>

#define NUM_COUNTERS 8
static int counters[NUM_COUNTERS];
static int num_counters = NUM_COUNTERS;

#define MAX_PNG 10
static struct {
    char *bytes;
    size_t length;
} png[MAX_PNG];

static int get_png_image(int n, const char **data, size_t *len) {
    assert(n>=0);

    if (n>=MAX_PNG) return -1;
    if (!png[n].bytes) {
        char name[64];
        FILE * F;
        snprintf(name, sizeof(name), "numbers/%d.png", n);
        F = fopen(name, "rb");
        if (!F) {
            printf("no image file for %s\n", name);
            return -2;
        }
        fseek(F, 0, SEEK_END);
        png[n].length = (size_t)ftell(F);
        png[n].bytes = malloc(png[n].length);
        fseek(F, 0, SEEK_SET);
        fread(png[n].bytes, png[n].length, 1, F);
        printf("reading %u bytes from %s\n", (unsigned int)png[n].length, name);
        fclose(F);
    }
    *data = png[n].bytes;
    *len = png[n].length;
    return 0;
}

static int png_process(FCGX_Request *request, int value) {
    const char * data = 0;
    size_t len = 0;
    if (get_png_image(value, &data, &len)!=0) {
        FCGX_FPrintF(request->out,
                     "Status: 501 Not Implemented\r\n"
                     "Content-type: text/plain\r\n"
                     "\r\n");
        return 501;
    }
    FCGX_FPrintF(request->out,
                 "Status: 200 OK\r\n"
                 "Content-Type: image/png\r\n"
                 "Content-Length: %u\r\n"
                 "\r\n", (unsigned int)len);
    int bytes = FCGX_PutStr(data, (int)len, request->out);
    assert(bytes==(int)len);
    return 200;
}

static int counter_process(FCGX_Request *request, int n, const char * type) {
    int value = ++counters[n];
    assert(n>=0 && n<num_counters);
    assert(type);

    if (strcmp(type, "image/png")==0) {
        return png_process(request, value);
    }
    
    FCGX_FPrintF(request->out,
                 "Status: 200 OK\r\n"
                 "Content-Type: %s\r\n"
                 "\r\n"
                 "%d\n", type, value);
    return 200;
}

static void save_counters(void) {
    FILE * F = fopen("counters.dat", "wb");
    fwrite(&num_counters, sizeof(int), 1, F);
    fwrite(counters, sizeof(int), (size_t)num_counters, F);
    fclose(F);
}

static void load_counters(void) {
    int n;
    FILE * F = fopen("counters.dat", "rb");
    if (F) {
        fread(&n, sizeof(int), 1, F);
        if (n>num_counters) n = num_counters;
        fread(counters, sizeof(int), (size_t)n, F);
        fclose(F);
    }
}

static void signal_handler(int sig) {
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

static const char * filename(const char *path) {
    const char * result = strrchr(path, '/');
    return result ? result+1 : 0;
}

static const char * extension(const char *path) {
    const char * result = strrchr(path, '/');
    if (!result) result = path;
    result = strchr(result ? result : path, '.');
    return result ? result+1 : 0;
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
        const char *remote = FCGX_GetParam("REMOTE_ADDR", request.envp);
        const char *script = FCGX_GetParam("SCRIPT_NAME", request.envp);
        const char *name = filename(script);
        const char *ext = extension(script);
        int n = atoi(name);
        if (ext) {
            if (strcmp(ext, "png")==0) {
                printf("request from %s for %d.png\n", remote, n);
                png_process(&request, n);
            }
        }
        else {
            printf("request from %s for counter %d\n",
                   remote, n);
            if (n < 0 || n >= num_counters) {
                FCGX_FPrintF(request.out,
                             "Status: 404 Not Found\r\n\r\n\n");
            }
            else {
                const char *type = FCGX_GetParam("HTTP_ACCEPT", request.envp);
                if (strstr(type, "text/")) {
                    type = "text/plain";
                }
                else {
                    type = "image/png";
                }
                printf("returning %s\n", type);
                counter_process(&request, n, type);
            }
        }
    }
    return 0;
}
