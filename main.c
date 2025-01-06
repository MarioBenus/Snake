#include <stdio.h>
#include <unistd.h>
#include <curses.h>
#include <pthread.h>
#include <stdlib.h>

#include "pipe.h"


void server(size_t game_width, size_t game_height, char* server_name)
{
    char asd[game_width + 2][game_height + 3];
    for (size_t i = 0; i < game_height + 2; i++)
        for (size_t j = 0; j < game_width + 2; j++)
            asd[i][j] = j + 48;
    
    for (size_t i = 0; i < game_width + 2; i++)
    {
        asd[i][game_height + 2] = '\0';
    }
    

    // borders
    for (size_t i = 1; i < game_width + 1; i++)
        asd[i][0] = '-';
    for (size_t i = 1; i < game_width + 1; i++)
        asd[i][game_height + 1] = '-';
    for (size_t i = 1; i < game_height + 1; i++)
        asd[0][i] = '|';
    for (size_t i = 1; i < game_height + 1; i++)
        asd[game_width + 1][i] = '|';
    asd[0][0] = '/';
    asd[game_width + 1][game_height + 1] = '/';
    asd[0][game_height + 1] = '\\';
    asd[game_width + 1][0] = '\\';

    const int fd_pipe = pipe_open_write(server_name);

    for (size_t i = 0; i < game_width + 2; i++)
    {
        write(fd_pipe, asd[i], game_height + 3);
    }
    pipe_close(fd_pipe);

    return;
}

void* client_input()
{
    return NULL;
}

void* client_render(size_t game_width, size_t game_height, char* server_name)
{
    initscr();                // Initialize ncurses
    nodelay(stdscr, TRUE);    // Set non-blocking input mode
    noecho();                 // Disable echoing of typed characters
    curs_set(FALSE);          // Hide the cursor


    char asd[game_width + 2][game_height + 3];

    const int fd_pipe = pipe_open_read(server_name);

    for (size_t i = 0; i < game_width + 2; i++)
    {
        read(fd_pipe, asd[i], game_height + 3);
    }

    for (size_t i = 0; i < game_width + 2; i++)
    {
        for (size_t j = 0; j < game_height + 2; j++)
        {
            printw("%c", asd[j][i]);
        }
        printw("\n");
    }
    refresh();
    sleep(5);
    
    pipe_close(fd_pipe);
    curs_set(TRUE);
    endwin();
    return NULL;

}

int main() {
    while (1)
    {
        printf("1) Create a new game\n");
        printf("2) Join game\n");
        printf("3) Exit\n\n");

        char a;
        scanf("%s", &a);
        if (a == '1') // CREATE GAME
        {
            while (1)
            {
                printf("\nWrite a name for the server\n");
                char* b = calloc(11, sizeof(char));
                scanf("%s", b);

                pipe_init(b);

                const pid_t pid = fork();
                if (pid == 0)
                    server(20, 20, b);
                else
                {
                    client_render(20, 20, b);
                    pipe_destroy(b);
                    free(b);
                }
                return 0;
            }
        }
        else if (a == '2') // JOIN GAME
        {
            break; // TODO
        }
        else if (a == '3') // EXIT
            break;
    }
    

    
    return 0;
}


