#define CURL_STATICLIB
#include <curl/curl.h>
#include <curl/easy.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdlib.h>
#include <yaml.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

extern int errno;

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream);
unsigned valid_resp(CURLcode res);
void make_cmd(const char* cmd_p1, const char* cmd_p2, char * cmd);
void exec(const char* cmd, unsigned ignore_on_error);
void download_file(download_ctx *dwnl_ctx);
static int xferinfo(void *p, curl_off_t dltotal, curl_off_t dlnow,
                    curl_off_t ultotal, curl_off_t ulnow);

char *restart_db_container = "docker container restart 713dbe2ee0ed";
char *docker_cmd = "docker exec -i novakid_testing_postgres_1 /bin/bash -c \"psql --host \\\"localhost\\\" --port \\\"5432\\\" -U postgres --dbname \\\"postgres\\\" -c ";
char *drop_db = "\'DROP DATABASE working;\'\"";
char *create_db = "\'CREATE DATABASE working;\'\"";
char *alter_owner_db = "\'ALTER DATABASE working OWNER TO test;\'\"";
char *resore_db = "docker exec -i novakid_testing_postgres_1 /bin/bash -c \"pg_restore --host \"localhost\" --port \"5432\" --username \"test\" --no-password --role \"test\" --dbname \"working\" --no-owner --verbose\" < /Users/alekseysmaga/Projects/novakid-backend/var/backups/novakid-db.backup";

typedef struct {
  CURL *curl;
} curl_progress;

typedef struct {
    char *url;
    char *userppwd;
    char *outfilename;
} download_ctx;




int main(int argc, char **argv) {
    unsigned r_code = 0;
    CURL *curl_handle;
    FILE *fp;
    CURLcode res;

    download_ctx dwnl_ctx;
    dwnl_ctx.url = "https://nexus.novakidschool.com/repository/raw-group/qa/novakid-db.backup";
    dwnl_ctx.userppwd = "raw-user:mFfrbRTybzAIZOsskOQW";
    dwnl_ctx.outfilename = "/Users/alekseysmaga/Projects/novakid-backend/var/backups/novakid-db.backup";

    download_file(&dwnl_ctx);

    if (r_code) {
        FILE *p;
        char cmd[1024];

        exec(restart_db_container, 0);

        printf("Process DROP DB.\n");
        make_cmd(docker_cmd, drop_db, cmd);
        exec(cmd, 0);
        memset(cmd, 0, sizeof(cmd));

        make_cmd(docker_cmd, create_db, cmd);
        exec(cmd, 0);
        memset(cmd, 0, sizeof(cmd));

        make_cmd(docker_cmd, alter_owner_db, cmd);
        exec(cmd, 0);
        memset(cmd, 0, sizeof(cmd));

        exec(resore_db, 0);
    }

    exit(EXIT_SUCCESS);
}


size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written;
    written = fwrite(ptr, size, nmemb, stream);
    return written;
}

unsigned valid_resp(CURLcode res) {
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
        return false;
    }
    return true;
}

void make_cmd(const char* cmd_p1, const char* cmd_p2, char * cmd) {
    size_t size = strlen(cmd_p1) + strlen(cmd_p2) + 1;
    strcat(cmd, cmd_p1);
    strcat(cmd, cmd_p2);
    
    cmd[size] = '\0';
}

void exec(const char* cmd, unsigned ignore_on_error) {
   char buffer[128];

   // Open pipe to file
    FILE* pipe = popen(cmd, "r");
    if (!pipe) {
        fprintf(stderr, "ERROR on exec CMD: %s\n", cmd);
        if (!ignore_on_error)
            exit(EXIT_FAILURE);
   }

   printf("CMD output: ");
   // read till end of process:
   while (!feof(pipe)) {
        // use buffer to read
        if (fgets(buffer, 128, pipe) != NULL)
            printf(buffer);
   }

   pclose(pipe);
}

void download_file(download_ctx *dwnl_ctx)
{
    unsigned r_code = 0;

    CURL *curl_handle;
    FILE *fp;
    CURLcode res;

    curl_handle = curl_easy_init();
    if (curl_handle) {
        struct stat file_info;
        curl_progress prog;
        
        prog.curl = curl_handle;
        
        if ((fp = fopen(dwnl_ctx->outfilename, "wb") == NULL)) {
            fprintf(stderr, "Can't open file %s.", dwnl_ctx->outfilename);
            exit(EXIT_FAILURE);
        }
            
        curl_easy_setopt(curl_handle, CURLOPT_URL, dwnl_ctx->url);
        curl_easy_setopt(curl_handle, CURLOPT_USERPWD, dwnl_ctx->userppwd);
        curl_easy_setopt(curl_handle, CURLOPT_XFERINFOFUNCTION, xferinfo);
        curl_easy_setopt(curl_handle, CURLOPT_XFERINFODATA, &prog);
        curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl_handle);
        r_code = valid_resp(res);
        curl_easy_cleanup(curl_handle);
        fclose(fp);
    }
}


static int xferinfo(void *p, curl_off_t dltotal, curl_off_t dlnow,
                    curl_off_t ultotal, curl_off_t ulnow)
{
    curl_progress *myp = (struct curl_progress *)p;
    CURL *curl = myp->curl;
    double ulspeed_curl = 0;

    curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD, &ulspeed_curl);

    fprintf(stderr, "DOWN: %ld" CURL_FORMAT_CURL_OFF_T
                    " of %ld" CURL_FORMAT_CURL_OFF_T
                    "\r",
                    dlnow, dltotal);

  return 0;
}