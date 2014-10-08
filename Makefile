CC=g++
SOURCES=main.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=hello

#USEGTKMM=1
USEGTK3=1
USEGLX=1

ifdef USEGTKMM
CFLAGS=`pkg-config gtkmm-2.4 --cflags`
LFLAGS=`pkg-config gtkmm-2.4 --libs` -lgthread-2.0 
endif

ifdef USEGTK3
CFLAGS=-c -Wall `pkg-config --cflags gtk+-3.0`
LFLAGS=`pkg-config --libs gtk+-3.0`
endif

ifdef USEGLX
CFLAGS+=`pkg-config --cflags gdk-x11-3.0`
LFLAGS+=`pkg-config --libs gdk-x11-3.0` -lGL -lGLU -lX11 -lm 
endif

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) -o $@.$(shell uname -n) $^ $(LFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -g -c -o $@ $<
	$(CC) -MM $(CFLAGS) $*.cpp -MF $(@:.o=.d) -MT $@
