CC = gcc
LDFLAGS = -lncurses

SRCS = main.c pipe.c sll.c snake.c server.c client.c common.c syn_sll.c
OBJS = $(SRCS:.c=.o)

TARGET = snake

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)


%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


