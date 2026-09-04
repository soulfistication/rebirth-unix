CXX ?= c++
CXXFLAGS += -std=c++17 -O2 -Wall -Wextra -Isrc
CXXFLAGS += $(shell sdl2-config --cflags)
LDFLAGS += $(shell sdl2-config --libs)
UNAME_S := $(shell uname -s)
ifneq ($(UNAME_S),Darwin)
  LDFLAGS += -pthread -lm -ldl
endif

SRCS = src/main.cpp src/engine.cpp src/tb303.cpp src/drums.cpp src/fx.cpp src/sequencer.cpp src/ui.cpp
OBJS = $(SRCS:.cpp=.o)

.PHONY: all clean

all: rebirth

rebirth: $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) rebirth
