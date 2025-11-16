CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
LDFLAGS = -lncursesw -lm

# Detect macOS
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    # Check for Homebrew installation location
    HOMEBREW_PREFIX := $(shell brew --prefix 2>/dev/null)
    ifneq ($(HOMEBREW_PREFIX),)
        CXXFLAGS += -I$(HOMEBREW_PREFIX)/opt/ncurses/include
        LDFLAGS += -L$(HOMEBREW_PREFIX)/opt/ncurses/lib
    endif
endif

TARGET = roguelike
SRCS = roguelike.cpp entity.cpp mob.cpp combat.cpp pathfinding.cpp
OBJS = $(SRCS:.cpp=.o)
HEADERS = entity.h mob.h combat.h pathfinding.h

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
