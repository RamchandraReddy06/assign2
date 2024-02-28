# Make File for linking and compiling

TARGET1 = test_assign2_1
TARGET2 = test_assign2_2

# Define source files and object files
SRCS = test_assign2_1.c test_assign2_2.c buffer_mgr_stat.c buffer_mgr.c dberror.c storage_mgr.c
OBJS = $(SRCS:.c=.o)

# Compiler and flags
CC = gcc
CFLAGS = -g -Wall

# Default target to compile both executables
all: $(TARGET1) $(TARGET2)

# Compile the first target executable
$(TARGET1): $(OBJS)
	$(CC) $(CFLAGS) -o $@ test_assign2_1.o buffer_mgr_stat.o buffer_mgr.o dberror.o storage_mgr.o

# Compile the second target executable
$(TARGET2): $(OBJS)
	$(CC) $(CFLAGS) -o $@ test_assign2_2.o buffer_mgr_stat.o buffer_mgr.o dberror.o storage_mgr.o

# Clean build artifacts, including the executables and .o files
clean:
	rm -f $(TARGET1) $(TARGET2) $(OBJS)


