CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
LDFLAGS = -lncurses -lm

TARGET = roguelike
SRCS = roguelike.cpp entity.cpp
OBJS = $(SRCS:.cpp=.o)
HEADERS = entity.h

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
