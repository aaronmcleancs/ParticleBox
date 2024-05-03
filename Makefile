CXX = g++  # or clang++ if you're using Clang
CXXFLAGS = -I./UI -I./Engine -I/opt/homebrew/include/SDL2 -D_THREAD_SAFE -std=c++17
LDFLAGS = $(shell sdl2-config --libs) -lSDL2_ttf

SRCS = $(wildcard *.cpp ./Engine/*.cpp ./UI/*.cpp)
OBJS = $(SRCS:.cpp=.o)
TARGET = ParticleSimulator

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
