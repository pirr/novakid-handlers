#include <string.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "utils.h"
#include "downloader.h"

char *restart_db_container = "docker container restart novakid_testing-136_postgres_1";
char *docker_cmd = "docker exec -i novakid_testing-136_postgres_1 /bin/bash -c \"psql --host \\\"localhost\\\" --port \\\"5432\\\" -U postgres --dbname \\\"postgres\\\" -c ";
char *drop_db = "\'DROP DATABASE working;\'\"";
char *create_db = "\'CREATE DATABASE working;\'\"";
char *alter_owner_db = "\'ALTER DATABASE working OWNER TO test;\'\"";
char *resore_db = "docker exec -i novakid_testing-136_postgres_1 /bin/bash -c \"pg_restore --host \\\"localhost\\\" --port \\\"5432\\\" --username \\\"test\\\" --no-password --role \\\"test\\\" --dbname \\\"working\\\" --no-owner --verbose\" < /Users/alekseysmaga/Projects/novakid-backend/var/backups/novakid-db.backup";


int main(int argc, char **argv) {
    
    download_ctx dwnl_ctx;
    bool download = false;

    int count;

    for (count=1; count < argc; count++) {
        char *opt = argv[count];

        if (strcmp(opt, "-d") == 0)
            download = true;
        else if (strcmp(opt, "-u") == 0)
            dwnl_ctx.url = argv[++count];
        else if (strcmp(opt, "-p") == 0)
            dwnl_ctx.userppwd = argv[++count];
        else if (strcmp(opt, "-o") == 0)
            dwnl_ctx.outfilename = argv[++count];
        else {
            fprintf(stderr, "Invalid option %s.\n", argv[count]);
            exit(EXIT_FAILURE);
        }
    
    }

    bool is_valid_response = false;

    if (download) {
        char *err = NULL;
        if (dwnl_ctx.outfilename == NULL)
            err = "outfilename [-o] is required";
        else if (dwnl_ctx.url == NULL)
            err = "url [-u] is required";
        else if (dwnl_ctx.userppwd == NULL)
            err = "userppwd [-p] is required";

        if (err != NULL) {
            fprintf(stderr, "%s", err);
            exit(EXIT_FAILURE);
        }

        is_valid_response = download_file(&dwnl_ctx);
    }

    if (is_valid_response | !download) {
        FILE *p;
        char cmd[1024];

        exec(restart_db_container, 0);

        printf("Process Recreate DB.\n");
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
