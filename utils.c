#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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
            printf("%s", buffer);
   }

   pclose(pipe);
}