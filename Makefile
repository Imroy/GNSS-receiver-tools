PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

# Libraries with pkg-config data
PKGS = sdl2

COMMON_FLAGS = -g -O0 -Wall -I. -fopenmp -finput-charset=UTF-8 `pkg-config --cflags $(PKGS)`
LIBS = -lm -lstdc++ -lgomp `pkg-config --libs $(PKGS)`
OBJS = $(patsubst %.cc,%.o, $(wildcard [A-Z]*.cc))

CFLAGS += $(COMMON_FLAGS)
CXXFLAGS += -std=c++11 $(COMMON_FLAGS)

PROGRAMS = gps_status

all: $(PROGRAMS)

clean:
	@rm -fv .depend *.o $(PROGRAMS)

install: $(PROGRAMS)
	install -t $(BINDIR) $(PROGRAMS)

$(PROGRAMS): %: $(OBJS)
	$(CXX) $(LDFLAGS) $^ $(LIBS) -o $@

parse: parse.o NMEA-0183.o
	$(CXX) $(LDFLAGS) $^ $(LIBS) -o $@

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

depend:
	touch .depend
	makedepend -f .depend -I . *.cc 2> /dev/null

ifneq ($(wildcard .depend),)
include .depend
endif
