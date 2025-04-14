#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

int main(int argc, char *argv[]) {
    // Open syslog for logging
    openlog("writer", LOG_PID | LOG_CONS, LOG_USER);

    // Check if the correct number of arguments is provided
    if (argc != 3) {
        syslog(LOG_ERR, "Error: Two arguments required - <writefile> <writestr>");
        fprintf(stderr, "Usage: %s <writefile> <writestr>\n", argv[0]);
        closelog();
        return 1;
    }

    const char *writefile = argv[1];
    const char *writestr = argv[2];

    // Open the file for writing
    FILE *file = fopen(writefile, "w");
    if (file == NULL) {
        syslog(LOG_ERR, "Error: Could not open file %s for writing", writefile);
        perror("Error");
        closelog();
        return 1;
    }

    // Write the string to the file
    if (fprintf(file, "%s", writestr) < 0) {
        syslog(LOG_ERR, "Error: Could not write to file %s", writefile);
        perror("Error");
        fclose(file);
        closelog();
        return 1;
    }

    // Log the successful write operation
    syslog(LOG_DEBUG, "Writing %s to %s", writestr, writefile);

    // Close the file
    fclose(file);

    // Close syslog
    closelog();

    return 0;
}