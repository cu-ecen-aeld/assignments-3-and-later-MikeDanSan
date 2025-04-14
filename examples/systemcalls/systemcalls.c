#include "systemcalls.h"
#include <stdlib.h>  // For exit(), EXIT_FAILURE
#include <unistd.h>  // For fork(), execv(), dup2(), STDOUT_FILENO
#include <sys/types.h>  // For pid_t
#include <sys/wait.h>  // For waitpid(), WIFEXITED(), WEXITSTATUS()
#include <fcntl.h>  // For open(), O_WRONLY, O_CREAT, O_TRUNC

bool do_system(const char *cmd)
{
    if (cmd == NULL) {
        return false; // Return false if the command is NULL
    }

    int ret = system(cmd); // Call the system() function with the command

    // Check if the system() call succeeded
    if (ret == -1) {
        return false; // system() call failed
    }

    // Check the exit status of the command
    if (WIFEXITED(ret) && WEXITSTATUS(ret) == 0) {
        return true; // Command executed successfully
    }

    return false; // Command failed
}

bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char *command[count + 1];
    int i;
    for (i = 0; i < count; i++) {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL; // Null-terminate the command array

    pid_t pid = fork(); // Create a new process
    if (pid == -1) {
        // Fork failed
        va_end(args);
        return false;
    } else if (pid == 0) {
        // Child process
        execv(command[0], command); // Execute the command
        // If execv returns, it means there was an error
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            // waitpid failed
            va_end(args);
            return false;
        }

        // Check if the child process exited successfully
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            va_end(args);
            return true;
        }
    }

    va_end(args);
    return false; // Command execution failed
}

bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char *command[count + 1];
    int i;
    for (i = 0; i < count; i++) {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL; // Null-terminate the command array

    fflush(stdout); // Flush stdout to avoid duplicate prints after fork()

    pid_t pid = fork(); // Create a new process
    if (pid == -1) {
        // Fork failed
        va_end(args);
        return false;
    } else if (pid == 0) {
        // Child process
        int fd = open(outputfile, O_WRONLY | O_CREAT | O_TRUNC, 0644); // Open the output file
        if (fd == -1) {
            // Failed to open the file
            exit(EXIT_FAILURE);
        }

        // Redirect standard output to the file
        if (dup2(fd, STDOUT_FILENO) == -1) {
            // Failed to redirect stdout
            close(fd);
            exit(EXIT_FAILURE);
        }

        close(fd); // Close the file descriptor as it's no longer needed

        execv(command[0], command); // Execute the command
        // If execv returns, it means there was an error
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            // waitpid failed
            va_end(args);
            return false;
        }

        // Check if the child process exited successfully
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            va_end(args);
            return true;
        }
    }

    va_end(args);
    return false; // Command execution failed
}