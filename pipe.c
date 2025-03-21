#include "pipe.h"
#include <stdlib.h>
#include <stdio.h>

void pipe_init(const char *path)
{
    if (mkfifo(path, S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP) == -1)
    {
        fprintf(stderr, "Failed to create named pipe %s\n", path);
        perror("Failed to create named pipe");
        exit(EXIT_FAILURE);
    }
}

void pipe_destroy(const char *path)
{
    if (unlink(path) == -1)
    {
        fprintf(stderr, "Failed to unlink named pipe %s\n", path);
        perror("Failed to unlink named pipe");
        exit(EXIT_FAILURE);
    }
}

static int open_pipe(const char *path, int flags)
{
    const int fd = open(path, flags);
    if (fd == -1)
    {
        fprintf(stderr, "Failed to open named pipe %s\n", path);
        perror("");
        exit(EXIT_FAILURE);
    }
    return fd;
}

int pipe_open_write(const char *path)
{
    return open_pipe(path, O_WRONLY);
}

int pipe_open_read(const char *path)
{
    return open_pipe(path, O_RDONLY);
}

void pipe_close(const int fd)
{
    if (close(fd) == -1)
    {
        perror("Failed to close pipe");
        exit(EXIT_FAILURE);
    }
}