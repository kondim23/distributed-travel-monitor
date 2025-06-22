# Improved Makefile for Travels_Virus_Management_SysPro2_2020-2021

CC = gcc
CFLAGS = -g
SRC_DIR = src
BIN_DIR = bin
RUN_DIR = runtime
PIPE_DIR = $(RUN_DIR)/pipes
LOG_DIR = $(RUN_DIR)/logs
INCLUDE_DIR = include

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BIN_DIR)/%.o)

# List main executables
EXECUTABLES = travelMonitor Monitor

all: $(BIN_DIR) $(RUN_DIR) $(addprefix $(BIN_DIR)/,$(EXECUTABLES))

# Map main .c files to their binaries
$(BIN_DIR)/travelMonitor: $(filter-out $(BIN_DIR)/Monitor.o,$(OBJS))
	$(CC) $(CFLAGS) -o $@ $(filter-out $(BIN_DIR)/Monitor.o,$(OBJS))

$(BIN_DIR)/Monitor: $(filter-out $(BIN_DIR)/travelMonitor.o,$(OBJS))
	$(CC) $(CFLAGS) -o $@ $(filter-out $(BIN_DIR)/travelMonitor.o,$(OBJS))

# Pattern rule for object files
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create bin directory if it doesn't exist
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(RUN_DIR):
	mkdir -p $(RUN_DIR)
	mkdir -p $(LOG_DIR)
	mkdir -p $(PIPE_DIR)

clean:
	rm -rf $(BIN_DIR) $(RUN_DIR)

.PHONY: all clean