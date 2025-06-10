CC=g++
LDFLAGS=-std=c++17 -O3 -lm -Wall
SOURCES=src/Pin.cpp src/MBFFOptimizer.cpp src/main.cpp 
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=mbff
INCLUDES=src/MBFFOptimizer.h 

all: $(SOURCES) bin/$(EXECUTABLE)

bin/$(EXECUTABLE): $(OBJECTS)
	mkdir -p bin
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

%.o:  %.c  ${INCLUDES}
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o bin/$(EXECUTABLE)