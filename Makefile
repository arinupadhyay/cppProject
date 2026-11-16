# ==============================================================================
# Simple Makefile for Blockchain Voting System
# ==============================================================================

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET = voting

.PHONY: all clean run help

all: $(TARGET)

$(TARGET): main.cpp blockchain.hpp
	@echo "Compiling Blockchain Voting System..."
	$(CXX) $(CXXFLAGS) main.cpp -o $(TARGET)
	@echo "Build successful! Run with: ./$(TARGET)"

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
	@echo "Cleaned build binary."

help:
	@echo "Commands:"
	@echo "  make      - Build the C++ binary (./voting)"
	@echo "  make run  - Build and run the interactive program"
	@echo "  make clean- Remove binary"
