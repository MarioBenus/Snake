#include "client.h"
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <curses.h>
#include <stdlib.h>

#include "common.h"
#include "pipe.h"

#define SLEEP_LENGTH 150000

void* client_input(void* args)
{
    client_input_thread_data* citd = args;
    bool* quit = citd->quit;
    char* pipe_name = add_suffix(citd->server_name, "INPUT");
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

    char* pipe_name = add_suffix(server_name, "RENDER");
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

        // for (size_t i = 0; i < game_width + 2; i++)
        // {
        //     printw("%s", board[i]);
        //     printw("\n");
        // }
        

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