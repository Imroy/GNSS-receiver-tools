PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

# Libraries with pkg-config data
PKGS = sdl2 SDL2_ttf

COMMON_FLAGS = -Wall -I. -Iinclude -fopenmp -finput-charset=UTF-8 `pkg-config --cflags $(PKGS)`
LIBS = -lm -lstdc++ -lgomp `pkg-config --libs $(PKGS)`
LIB_OBJS = $(patsubst %.cc,%.o, $(wildcard lib/*.cc))
APP_OBJS = $(patsubst %.cc,%.o, $(wildcard src/*.cc))

CFLAGS += -flto $(COMMON_FLAGS)
CXXFLAGS += -std=c++11 -flto $(COMMON_FLAGS)
LDFLAGS += -flto

PROGRAMS = $(patsubst %.cc,%, $(wildcard *.cc))

all: $(PROGRAMS)

clean:
	@rm -fv .depend *.o $(PROGRAMS)

install: $(PROGRAMS)
	install -t $(BINDIR) $(PROGRAMS)

# Ordinary programs that just need the libs
parse: parse.o $(LIB_OBJS)
	$(CXX) $(LDFLAGS) $^ $(LIBS) -o $@

bin: bin.o $(LIB_OBJS)
	$(CXX) $(LDFLAGS) $^ $(LIBS) -o $@

$(LIB_OBJS): %.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

# SDL app that also needs stuff in src/
gps_status: gps_status.o $(LIB_OBJS) $(APP_OBJS)
	$(CXX) $(LDFLAGS) $^ $(LIBS) -o $@

gps_status.o: gps_status.cc
	$(CXX) -Isrc $(CXXFLAGS) -c $< -o $@

$(APP_OBJS): %.o: %.cc
	$(CXX) -Isrc $(CXXFLAGS) -c $< -o $@

depend:
	touch .depend
	makedepend -f .depend -I include include/*.hh lib/*.cc src/*.hh src/*.cc *.cc 2> /dev/null

ifneq ($(wildcard .depend),)
include .depend
endif
# DO NOT DELETE
