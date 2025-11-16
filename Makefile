CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LDFLAGS = -lncurses -lm

TARGET = roguelike
SRCS = roguelike.c entity.c
OBJS = $(SRCS:.c=.o)
HEADERS = entity.h

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
