CC = g++
CXXFLAGS = -std=c++17 -g -O3 -Wall `pkg-config --cflags cairo`
LDFLAGS = `pkg-config --libs cairo`
SOURCES = src/MBFFOptimizer.cpp src/main.cpp src/visualizer.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = mbff
INCLUDES = src/MBFFOptimizer.h

all: bin/$(EXECUTABLE)

bin/$(EXECUTABLE): $(OBJECTS)
	mkdir -p bin
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

%.o: %.cpp $(INCLUDES)
	$(CC) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f src/*.o bin/$(EXECUTABLE)
