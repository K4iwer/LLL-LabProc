CC = gcc

CFLAGS = -Wall -Wextra -std=c11

LDFLAGS = -lwiringPi

TARGET = teclado

SRCS = main.c keypad.c

OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

run: $(TARGET)
	sudo ./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean run