# Compiler
CXX := g++

# Compiler flags
# -Wall -Wextra -pedantic
CXXFLAGS := -O3 -std=c++17

# Check if THREADS is defined
ifdef THREADS
    CXXFLAGS += -DTHREADS
endif

# Directories
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

# Source files
SRCS := $(SRC_DIR)/count.cpp $(SRC_DIR)/eval.cpp

MAIN_SRCS := $(SRC_DIR)/main.cpp $(SRCS)
TEST_SRCS := $(SRC_DIR)/test.cpp $(SRCS)

# Object files
MAIN_OBJS := $(MAIN_SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
TEST_OBJS := $(TEST_SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Executable name
EXEC := $(BIN_DIR)/connect4

EXEC_TEST := $(BIN_DIR)/test

# Main target
all: clean main test

main: dirs $(EXEC)

test: dirs $(EXEC_TEST)

# Create directories
dirs:
	mkdir -p $(OBJ_DIR) $(BIN_DIR)

# Link
$(EXEC): $(MAIN_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(EXEC_TEST): $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Compile
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

#trace
trace:
	xctrace record --template "Time Profiler" --launch $(EXEC) $(N)

clean_trace:
	rm -rf *.trace

# Clean
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all dirs clean
