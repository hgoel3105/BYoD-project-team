# Compiler and flags
CC = gcc
CFLAGS = -Wall -I.

# Source files
SRCS = main.c insert.c delete.c scan.c print_table.c bitmap.c

# Object files
OBJS = $(SRCS:.c=.o)

# Output executable
TARGET = row_store

# Default target
all: $(TARGET)

# Link object files to create the final binary
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Compile .c files into .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -f $(OBJS) $(TARGET)
