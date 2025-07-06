CC = gcc
CFLAGS = -Wall -I.

SRCS = main.c insert.c delete.c scan.c print_table.c bitmap.c index.c

OBJS = $(SRCS:.c=.o)

TARGET = row_store

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
