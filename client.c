#include "client.h"
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <curses.h>
#include <stdlib.h>
#include <pthread.h>

#include "common.h"
#include "pipe.h"

#define SLEEP_LENGTH 150000

typedef struct client_input_thread_data
{
    bool* quit;
    char* server_name;
    char* pid;
} client_input_thread_data;

typedef struct client_render_thread_data
{
    size_t game_width; 
    size_t game_height; 
    char* server_name;
    bool* quit;
    char* pid;
} client_render_thread_data;

void* client_input(void* args)
{
    client_input_thread_data* citd = args;
    bool* quit = citd->quit;
    char* pipe_name_temp = add_suffix(citd->server_name, "INPUT");
    char* pipe_name = add_suffix(pipe_name_temp, citd->pid);
    free(pipe_name_temp);
    const int fd_pipe = pipe_open_write(pipe_name);
    while (1)
    {
        char ch = getch();
        flushinp();

        write(fd_pipe, &ch, 1);

        if (ch == 'q')
        {
            *quit = true;
            break;
        }
        usleep(SLEEP_LENGTH);
    }

    free(pipe_name);
    pipe_close(fd_pipe);
}

void* client_render(void* args)
{
    initscr();                // Initialize ncurses
    nodelay(stdscr, TRUE);    // Set non-blocking input mode
    noecho();                 // Disable echoing of typed characters
    curs_set(FALSE);          // Hide the cursor
    client_render_thread_data* td = args;
    size_t game_width = td->game_width;
    size_t game_height = td->game_height;
    char* server_name = td->server_name;
    bool* quit = td->quit;

    char board[game_width + 2][game_height + 3];

    char* pipe_name_temp = add_suffix(server_name, "RENDER");
    char* pipe_name = add_suffix(pipe_name_temp, td->pid);
    free(pipe_name_temp);
    const int fd_pipe = pipe_open_read(pipe_name);

    while (1)
    {
        if (*quit)
            break;

        for (size_t i = 0; i < game_width + 2; i++)
        {
            read(fd_pipe, board[i], sizeof(char) * (game_height + 3));
        }

        clear();

        for (size_t j = 0; j < game_height + 2; j++)
        {
            for (size_t i = 0; i < game_width + 2; i++)
            {
                printw("%c", board[i][j]);
            }
            printw("\n");
        }

        refresh();
        usleep(SLEEP_LENGTH);
    }
    
    free(pipe_name);
    pipe_close(fd_pipe);
    endwin();
    return NULL;

}

void client_join(char* server_name)
{
    char buf[20];
    sprintf(buf, "%d", getpid());
    char* pipe_name = add_suffix(server_name, "JOIN");
    int fd_pipe = pipe_open_write(pipe_name);
    write(fd_pipe, buf, 20);
    pipe_close(fd_pipe);
    free(pipe_name);

    sleep(1); // give time to server to create pipes
    char* pipe_name_render_temp = add_suffix(server_name, "RENDER");
    char* pipe_name_render = add_suffix(pipe_name_render_temp, buf);
    char* pipe_name_input_temp = add_suffix(server_name, "INPUT");
    char* pipe_name_input = add_suffix(pipe_name_input_temp, buf);

    free(pipe_name_input_temp);
    free(pipe_name_render_temp);

    pthread_t render_thread;
    pthread_t input_thread;
    bool quit = false;
    client_input_thread_data citd = {&quit, server_name, buf};
    pthread_create(&input_thread, NULL, client_input, &citd);
    client_render_thread_data crtd = {30, 20, server_name, &quit, buf};
    pthread_create(&render_thread, NULL, client_render, &crtd);

    pthread_join(render_thread, NULL);
    pthread_join(input_thread, NULL);

    

    // pipe_destroy(pipe_name_render);
    // pipe_destroy(pipe_name_input);
    free(pipe_name_input);
    free(pipe_name_render);
}