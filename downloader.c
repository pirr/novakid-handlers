#define CURL_STATICLIB
#include <curl/curl.h>
#include <curl/easy.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdbool.h>

#include "downloader.h"

typedef struct {
  CURL *curl;
} curl_progress;

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream);
bool valid_resp(CURLcode res);

bool download_file(download_ctx *dwnl_ctx)
{
    bool is_valid = false;

    CURL *curl_handle;
    FILE *fp;
    CURLcode res;

    curl_handle = curl_easy_init();
    if (curl_handle) {
        struct stat file_info;
        
        if ((fp = fopen(dwnl_ctx->outfilename, "wb")) == NULL) {
            fprintf(stderr, "Can't open file %s.", dwnl_ctx->outfilename);
            exit(EXIT_FAILURE);
        }

        curl_easy_setopt(curl_handle, CURLOPT_URL, dwnl_ctx->url);
        curl_easy_setopt(curl_handle, CURLOPT_USERPWD, dwnl_ctx->userppwd);
        curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, fp);

        res = curl_easy_perform(curl_handle);
        is_valid = valid_resp(res);
        curl_easy_cleanup(curl_handle);
        fclose(fp);
    }

    return is_valid;
}

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written;
    written = fwrite(ptr, size, nmemb, stream);
    return written;
}

bool valid_resp(CURLcode res) {
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
        return false;
    }
    return true;
}
