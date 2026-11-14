# ==============================================================================
# Makefile for Blockchain Voting System (C++17 OOP Architecture)
# ==============================================================================

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pthread -I./include
SRCDIR = src
OBJDIR = obj
BINDIR = bin

SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SOURCES))
MAIN_OBJ = $(OBJDIR)/main.o

TARGET = $(BINDIR)/blockchain_voting
TEST_TARGET = $(BINDIR)/test_blockchain

.PHONY: all clean run test test_cpp help

all: $(TARGET)

$(TARGET): $(OBJECTS) $(MAIN_OBJ) | $(BINDIR)
	@echo "Linking executable: $@"
	$(CXX) $(CXXFLAGS) $^ -o $@
	@echo "Build successful! Run with: ./$(TARGET)"

test_cpp: $(OBJECTS) tests/test_blockchain.cpp | $(BINDIR)
	@echo "Building C++ unit tests..."
	$(CXX) $(CXXFLAGS) $(filter-out $(OBJDIR)/HttpServer.o, $(OBJECTS)) tests/test_blockchain.cpp -o $(TEST_TARGET)
	@echo "Running C++ tests..."
	./$(TEST_TARGET)

test:
	node tests/test_blockchain.js

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	@echo "Compiling: $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/main.o: main.cpp | $(OBJDIR)
	@echo "Compiling main entry point: $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(OBJDIR) $(BINDIR)
	@echo "Cleaned build artifacts."

help:
	@echo "Available targets:"
	@echo "  make          - Build the C++ blockchain voting system"
	@echo "  make run      - Build and launch native C++ server"
	@echo "  make test_cpp - Build and run C++ OOP test suite"
	@echo "  make test     - Run automated test suite"
	@echo "  make clean    - Remove object and binary files"
