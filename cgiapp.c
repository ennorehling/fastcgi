#include "cgiapp.h"

int main(void)
{
    FCGX_Request request;
    FCGX_Init();
    FCGX_InitRequest(&request, 0, 0);
    struct app* app = create_app();
    app->init(app->data);

    while(FCGX_Accept_r(&request) == 0) {
        app->process(app->data, &request);
    }
    app->done(app->data);
    return 0;
}
