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
SRCS := $(wildcard $(SRC_DIR)/*.cpp)

# Object files
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Executable name
EXEC := $(BIN_DIR)/count

# Main target
all: dirs $(EXEC)

# Create directories
dirs:
	mkdir -p $(OBJ_DIR) $(BIN_DIR)

# Link
$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Compile
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all dirs clean