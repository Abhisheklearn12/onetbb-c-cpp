
# Compiler settings
CC      = gcc
CXX     = g++
CFLAGS  = -Wall -Wextra -O2
CXXFLAGS= -Wall -Wextra -O2
LDFLAGS = -ltbb -lstdc++

# Targets
TARGET  = app
OBJS    = main.o tbb_wrapper.o

# Default target
all: $(TARGET)

# Compile C source
main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o main.o

# Compile C++ wrapper (needs TBB)
tbb_wrapper.o: tbb_wrapper.cpp
	$(CXX) $(CXXFLAGS) -c tbb_wrapper.cpp -o tbb_wrapper.o

# Link everything
$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $(TARGET)

# Clean build artifacts
clean:
	rm -f $(OBJS) $(TARGET)
