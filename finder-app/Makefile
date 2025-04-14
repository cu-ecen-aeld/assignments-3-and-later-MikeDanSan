# Compiler and flags
CC := $(CROSS_COMPILE)gcc
CFLAGS := -Wall -Werror -g
TARGET := writer

# Default target
all: $(TARGET)

# Build the writer application
$(TARGET): writer.o
	$(CC) $(CFLAGS) -o $(TARGET) writer.o

# Compile the object file
writer.o: writer.c
	$(CC) $(CFLAGS) -c writer.c

# Clean target
clean:
	rm -f $(TARGET) *.o