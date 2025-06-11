CC=g++
LDFLAGS=-std=c++17 -O3 -lm -Wall
SOURCES=src/MBFFOptimizer.cpp src/main.cpp 
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=mbff
INCLUDES=src/MBFFOptimizer.h src/BucketList.h src/CellLibrary.h src/DisSet.h src/Graph.h src/Instance.h src/Net.h src/Pin.h src/PlacementRow.h src/Point.h

all: $(SOURCES) bin/$(EXECUTABLE)

bin/$(EXECUTABLE): $(OBJECTS)
	mkdir -p bin
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

%.o:  %.c  ${INCLUDES}
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o bin/$(EXECUTABLE)