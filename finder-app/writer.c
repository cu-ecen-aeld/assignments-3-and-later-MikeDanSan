//
#include <stdio.h>
#include <syslog.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char* argv[])
{
    openlog(NULL, LOG_PERROR, LOG_USER);

    if (argc != 3) {
        syslog(LOG_ERR, "Incorrect number of arguments\n");
        syslog(LOG_ERR, "Usage: %s <filename> <contents>\n", argv[0]);
        closelog();
        return 1;
    }
    const char* filename = argv[1];
    const char* contents = argv[2];

    syslog(LOG_DEBUG, "Writing %s to %s", contents, filename);
    int fd = creat(filename, S_IRUSR|S_IWUSR);
    int    ret_code = 0;

    if (fd < 0) {
        syslog(LOG_ERR, "Could not open %s for writing\n", filename);
        ret_code = 1;
    } else if (write(fd, contents, strlen(contents)) != strlen(contents)) {
        syslog(LOG_ERR, "Write to %s failed\n", filename);
        ret_code = 1;
    }

    if (fd >= 0)
        close(fd);
    closelog();
    return ret_code;
}