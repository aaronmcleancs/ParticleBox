CXX = g++
CXXSTD = -std=c++17

SRCS = $(wildcard *.cpp ./Engine/*.cpp ./UI/*.cpp)
OBJS = $(SRCS:.cpp=.o)
TARGET = ParticleSimulator

UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

# Common optimization flags
OPT_FLAGS = -O3 -DNDEBUG -ffast-math -funroll-loops -ftree-vectorize

ifeq ($(UNAME_S), Darwin)
    # Check for Apple Silicon (M1/M2/etc)
    ifeq ($(UNAME_M), arm64)
        # M1/M2-specific optimizations
        OPT_FLAGS += -mcpu=apple-m1 -DUSE_SIMD
        LDFLAGS_EXTRA = -framework Accelerate
    else
        # Intel Mac optimizations
        OPT_FLAGS += -march=native
        LDFLAGS_EXTRA =
    endif
    
    CXXFLAGS = -I./UI -I./Engine -I/opt/homebrew/include/SDL2 -D_THREAD_SAFE $(CXXSTD) $(OPT_FLAGS) -I/opt/homebrew/include
    LDFLAGS = -L/opt/homebrew/lib $(shell sdl2-config --libs) -lSDL2_ttf $(LDFLAGS_EXTRA)

else ifeq ($(UNAME_S), Linux)
    # Linux optimizations
    OPT_FLAGS += -march=native -fopenmp
    CXXFLAGS = -I./UI -I./Engine $(CXXSTD) $(OPT_FLAGS) $(shell pkg-config --cflags sdl2 SDL2_ttf)
    LDFLAGS = $(shell pkg-config --libs sdl2 SDL2_ttf) -lgomp

else
    # Windows optimizations
    OPT_FLAGS += -march=native
    CXXFLAGS = -I./UI -I./Engine -I$(MINGW_PREFIX)/include/SDL2 -Dmain=SDL_main $(CXXSTD) $(OPT_FLAGS)
    LDFLAGS = -L$(MINGW_PREFIX)/lib -lSDL2 -lSDL2_ttf
endif

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

# Add M1-specific optimized build
m1_optimize: clean
	$(MAKE) CXXFLAGS="$(CXXFLAGS) -Ofast -flto -fno-signed-zeros -fno-trapping-math -ffp-contract=fast" LDFLAGS="$(LDFLAGS) -flto"