#ifndef SNAKE_PIPE
#define SNAKE_PIPE

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void pipe_init(const char *path);
void pipe_destroy(const char *path);
int pipe_open_write(const char *path);
int pipe_open_read(const char *path);
void pipe_close(int fd);

#endif