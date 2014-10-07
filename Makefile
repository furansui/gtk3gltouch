CC=g++
CFLAGS=-c -Wall `pkg-config --cflags gtk+-3.0`
LFLAGS=`pkg-config --libs gtk+-3.0`
SOURCES=main.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=hello

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) -o $@.$(shell uname -n) $(OBJECTS) $^ $(LFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -g -c -o $@ $<
	$(CC) -MM $(CFLAGS) $*.cpp -MF $(@:.o=.d) -MT $@
