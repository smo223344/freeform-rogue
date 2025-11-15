CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LDFLAGS = -lncurses -lm

TARGET = roguelike
SRC = roguelike.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
