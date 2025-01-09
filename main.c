#include <stdio.h>
#include <unistd.h>
#include <curses.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>


#include "pipe.h"
#include "sll.h"
#include "snake.h"

#define SLEEP_LENGTH 150000

coordinates server_place_apple(char* board, size_t game_width, size_t game_height)
{
    coordinates apple_pos;
    while (1)
    {
        int x = rand() % game_width + 1;
        int y = rand() % game_height + 1;

        if (board[x * game_height + y] == ' ')
        {
            apple_pos.pos_x = x;
            apple_pos.pos_y = y;
            return apple_pos;
        }
    }
}

void server(size_t game_width, size_t game_height, char* server_name)
{
    char board[game_width + 2][game_height + 3];
    
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

    const int fd_pipe_board = pipe_open_write(server_name);
    const int fd_pipe_input = pipe_open_read("SERVER-INPUT");
    sll snake;
    snake_init(&snake);

    //coordinates apple_pos = server_place_apple(board, game_width, game_height);

    char last_input = '\0';
    while (1) // GAME LOOP
    {
        char new_input;
        read(fd_pipe_input, &new_input, 1);
        if (new_input == 'q' || new_input == 'w' || new_input == 'a' || new_input == 's' || new_input == 'd')
            last_input = new_input;

        for (size_t i = 1; i < game_height + 1; i++)
            for (size_t j = 1; j < game_width + 1; j++)
                board[j][i] = ' ';
        
        //board[apple_pos.pos_x][apple_pos.pos_y] = '@';

        snake_node sn;
        sll_get(&snake, 0, &sn);
        coordinates snake_pos = sn.position;
        
        if (last_input == 'q')
            break;
        if (last_input == 'w')
            snake_pos.pos_y -= 1;
        if (last_input == 'a')
            snake_pos.pos_x -= 1;
        if (last_input == 's')
            snake_pos.pos_y += 1;
        if (last_input == 'd')
            snake_pos.pos_x += 1;

        if (board[snake_pos.pos_x][snake_pos.pos_y] != ' ') // COLLISION
        {
            // TODO
        }

        sll_for_each(&snake, snake_move, &snake_pos, NULL, NULL);
        sll_for_each(&snake, snake_draw_node_on_board, board, NULL, NULL);

        for (size_t i = 0; i < game_width + 2; i++)
        {
            write(fd_pipe_board, board[i], sizeof(char) * (game_height + 3));
        }

        usleep(SLEEP_LENGTH);
    }
    

    pipe_close(fd_pipe_input);
    pipe_close(fd_pipe_board);
    return;
}


void* client_input(void* args)
{
    bool* quit = args;
    const int fd_pipe = pipe_open_write("SERVER-INPUT");
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

    pipe_close(fd_pipe);
}

typedef struct client_render_thread_data
{
    size_t game_width; 
    size_t game_height; 
    char* server_name;
    bool* quit;
} client_render_thread_data;

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

    const int fd_pipe = pipe_open_read(server_name);

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
    
    pipe_close(fd_pipe);
    endwin();
    return NULL;

}

int main() {
    srand(time(NULL));
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
                pipe_init("SERVER-INPUT"); // TODO: CHANGE

                const pid_t pid = fork();
                if (pid == 0)
                    server(30, 20, b);
                else
                {
                    pthread_t render_thread;
                    pthread_t input_thread;
                    bool quit = false;
                    client_render_thread_data crtd = {30, 20, b, &quit};
                    pthread_create(&render_thread, NULL, client_render, &crtd);
                    pthread_create(&input_thread, NULL, client_input, &quit);

                    pthread_join(render_thread, NULL);
                    pthread_join(input_thread, NULL);
                    pipe_destroy(b);
                    pipe_destroy("SERVER-INPUT");
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


