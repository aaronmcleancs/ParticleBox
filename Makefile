CXX = g++
CXXSTD = -std=c++17

SRCS = $(wildcard *.cpp ./Engine/*.cpp ./UI/*.cpp)
OBJS = $(SRCS:.cpp=.o)
TARGET = ParticleSimulator

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S), Darwin)
    CXXFLAGS = -I./UI -I./Engine -I/opt/homebrew/include/SDL2 -D_THREAD_SAFE $(CXXSTD)
    LDFLAGS = -L/opt/homebrew/lib $(shell sdl2-config --libs) -lSDL2_ttf

else ifeq ($(UNAME_S), Linux)
    CXXFLAGS = -I./UI -I./Engine $(CXXSTD) $(shell pkg-config --cflags sdl2 SDL2_ttf)
    LDFLAGS = $(shell pkg-config --libs sdl2 SDL2_ttf)

else
    CXXFLAGS = -I./UI -I./Engine -I$(MINGW_PREFIX)/include/SDL2 -Dmain=SDL_main $(CXXSTD)
    LDFLAGS = -L$(MINGW_PREFIX)/lib -lSDL2 -lSDL2_ttf
endif

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)