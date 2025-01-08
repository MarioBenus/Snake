#include <stdio.h>
#include <unistd.h>
#include <curses.h>
#include <pthread.h>
#include <stdlib.h>

#include "pipe.h"
#include "sll.h"
#include "snake.h"



void server(size_t game_width, size_t game_height, char* server_name)
{
    char board[game_width + 2][game_height + 3];
    for (size_t i = 0; i < game_height + 2; i++)
        for (size_t j = 0; j < game_width + 2; j++)
            board[i][j] = ' ';
    
    for (size_t i = 0; i < game_width + 2; i++)
    {
        board[i][game_height + 2] = '\0';
    }
    

    // borders
    for (size_t i = 1; i < game_width + 1; i++)
        board[i][0] = '-';
    for (size_t i = 1; i < game_width + 1; i++)
        board[i][game_height + 1] = '-';
    for (size_t i = 1; i < game_height + 1; i++)
        board[0][i] = '|';
    for (size_t i = 1; i < game_height + 1; i++)
        board[game_width + 1][i] = '|';
    board[0][0] = '/';
    board[game_width + 1][game_height + 1] = '/';
    board[0][game_height + 1] = '\\';
    board[game_width + 1][0] = '\\';

    const int fd_pipe = pipe_open_write(server_name);
    sll snake;
    snake_init(&snake);
    sll_for_each(&snake, snake_draw_node_on_board, board, NULL, NULL);


    // while (1) // GAME LOOP
    // {
        for (size_t i = 0; i < game_width + 2; i++)
        {
            write(fd_pipe, board[i], game_height + 3);
        }
    // }
    

    
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


    char board[game_width + 2][game_height + 3];

    const int fd_pipe = pipe_open_read(server_name);

    for (size_t i = 0; i < game_width + 2; i++)
    {
        read(fd_pipe, board[i], game_height + 3);
    }

    for (size_t i = 0; i < game_width + 2; i++)
    {
        for (size_t j = 0; j < game_height + 2; j++)
        {
            printw("%c", board[j][i]);
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
        //scanf("%s", &a); // disabled for testing
        if (a == '1' || 1 /* for testing*/) // CREATE GAME
        {
            while (1)
            {
                //printf("\nWrite a name for the server\n"); // Disabled for testing
                char b[11] = "SERVER";
                //scanf("%s", b);

                pipe_init(b);

                const pid_t pid = fork();
                if (pid == 0)
                    server(20, 20, b);
                else
                {
                    client_render(20, 20, b);
                    pipe_destroy(b);
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


