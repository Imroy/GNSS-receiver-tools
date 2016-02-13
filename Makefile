PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

# Libraries with pkg-config data
PKGS =

COMMON_FLAGS = -g -O0 -Wall -I. -Iinclude -fopenmp -finput-charset=UTF-8 #`pkg-config --cflags $(PKGS)`
LIBS = -lm -lstdc++ -lgomp #`pkg-config --libs $(PKGS)`
LIB_OBJS = $(patsubst %.cc,%.o, $(wildcard lib/*.cc))

CFLAGS += -flto $(COMMON_FLAGS)
CXXFLAGS += -std=c++11 -flto $(COMMON_FLAGS)
LDFLAGS += -flto

PROGRAMS = $(patsubst %.cc,%, $(wildcard *.cc))

all: $(PROGRAMS)

clean:
	@rm -fv .depend *.o $(LIB_OBJS) $(PROGRAMS)

install: $(PROGRAMS)
	install -t $(BINDIR) $(PROGRAMS)

parse-skytraq: parse-skytraq.o $(LIB_OBJS)
	$(CXX) $(LDFLAGS) $^ $(LIBS) -o $@

record-skytraq: record-skytraq.o $(LIB_OBJS)
	$(CXX) $(LDFLAHS) $^ $(LIBS) -lmongoclient -lboost_thread -lboost_filesystem -lboost_system -lpthread -lssl -lcrypto -o $@

$(LIB_OBJS): %.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

depend:
	touch .depend
	makedepend -f .depend -I include include/*.hh lib/*.cc *.cc 2> /dev/null

ifneq ($(wildcard .depend),)
include .depend
endif
# DO NOT DELETE
