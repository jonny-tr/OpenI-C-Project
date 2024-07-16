# Compiler
CC = gcc
CFLAGS = -Wall -ansi -pedantic -ggdb3 -std=c89
LDFLAGS = -lm
SRCS = ${shell ls *.c}
OBJS = $(SRCS:.c=.o)
LIB = ${shell ls *.h} 

# Executable
TARGET = executable

# Default target
all: $(TARGET)

# Compile source files into object files
%.o: %.c $(LIB)
	$(CC) $(CFLAGS) $ -c $< -o $@

# Link object files into executable
$(TARGET): $(OBJS) $(LIB)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

# Remove up object files and executable
clean:
	rm -f $(OBJS) $(TARGET)
