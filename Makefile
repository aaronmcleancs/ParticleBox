# Makefile for ParticleSimulation
# Targets:
#   make             - build the simulator with optimisations
#   make debug       - build with debug symbols and no optimisation
#   make test        - build with optimisations and run the headless benchmark
#   make clean       - remove all build artefacts

CXX     = g++
CXXSTD  = -std=c++17
TARGET  = ParticleSimulator

SRCS    = $(wildcard *.cpp ./Engine/*.cpp ./UI/*.cpp)
OBJS    = $(SRCS:.cpp=.o)
DEPS    = $(OBJS:.o=.d)

UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

# ---- Optimisation defaults ---------------------------------------------------
OPT_FLAGS = -O3 -DNDEBUG -ffast-math -funroll-loops -ftree-vectorize
WARN_FLAGS = -Wall -Wextra -Wno-unused-parameter -Wno-unused-but-set-variable

# ---- Platform-specific settings ---------------------------------------------
ifeq ($(UNAME_S), Darwin)
    # macOS (Intel or Apple Silicon)
    ifeq ($(UNAME_M), arm64)
        OPT_FLAGS += -mcpu=apple-m1
        SDL_INC = -I/opt/homebrew/include -I/opt/homebrew/include/SDL2
        SDL_LIB = -L/opt/homebrew/lib
    else
        OPT_FLAGS += -march=native
        SDL_INC = -I/usr/local/include -I/usr/local/include/SDL2
        SDL_LIB = -L/usr/local/lib
    endif
    CXXFLAGS = -I./UI -I./Engine $(SDL_INC) -D_THREAD_SAFE $(CXXSTD) $(OPT_FLAGS) $(WARN_FLAGS)
    LDFLAGS  = $(SDL_LIB) $(shell sdl2-config --libs) -lSDL2_ttf -pthread

else ifeq ($(UNAME_S), Linux)
    OPT_FLAGS += -march=native
    CXXFLAGS = -I./UI -I./Engine $(CXXSTD) $(OPT_FLAGS) $(WARN_FLAGS) $(shell pkg-config --cflags sdl2 SDL2_ttf)
    LDFLAGS  = $(shell pkg-config --libs sdl2 SDL2_ttf) -pthread

else
    # Windows / MSYS2 / MinGW fallback
    OPT_FLAGS += -march=native
    CXXFLAGS = -I./UI -I./Engine -I$(MINGW_PREFIX)/include/SDL2 -Dmain=SDL_main $(CXXSTD) $(OPT_FLAGS) $(WARN_FLAGS)
    LDFLAGS  = -L$(MINGW_PREFIX)/lib -lSDL2main -lSDL2 -lSDL2_ttf -pthread
endif

# ---- Build rules -------------------------------------------------------------
.PHONY: all clean debug test help

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# Emit dependency files alongside object files so header changes trigger rebuilds.
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

debug: OPT_FLAGS = -O0 -g -DDEBUG
debug: clean all

test: all
	./$(TARGET) -test

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET)

help:
	@echo "ParticleSimulation Makefile"
	@echo "  make         - build with optimisations"
	@echo "  make debug   - build with -O0 -g"
	@echo "  make test    - build and run headless benchmark"
	@echo "  make clean   - remove build artefacts"
	@echo ""
	@echo "Environment:"
	@echo "  PARTICLE_FONT=/path/to/font.ttf  (overrides built-in font search)"
