CC = gcc

CFLAGS = -Wall -Wextra -std=c11 -g

LDFLAGS = -lgpiod

TARGET = teclado

SRCS = main.c keypad.c

OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	sudo ./$(TARGET)

.PHONY: all clean run