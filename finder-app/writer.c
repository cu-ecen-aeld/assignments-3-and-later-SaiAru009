#include<stdio.h>
#include<syslog.h>

int main(int argc, char* argv[]) {
    openlog(NULL,0,LOG_USER);
    if(argc!=3) {
        syslog(LOG_ERR, "Required no of args: 3, given: %d", argc);
        exit(1);
    }

    FILE* fptr;
    fptr = fopen(argv[1], "w");

    if(fptr==NULL) {
        syslog(LOG_ERR, "Unable to create file: %s", argv[1]);
        exit(1);
    }

    fprintf(fptr, argv[2]);
    syslog(LOG_DEBUG, "Writing %s to %s", argv[2], argv[1]);

    fclose(fptr);
}