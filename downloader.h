#include <stdbool.h>

typedef struct {
    char *url;
    char *userppwd;
    char *outfilename;
} download_ctx;

bool download_file(download_ctx *dwnl_ctx);
