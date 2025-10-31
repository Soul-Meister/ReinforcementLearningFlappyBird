CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -I.    # -I. means: look in current dir + subfolders
LDFLAGS = `sdl2-config --libs` -lSDL2_image -lSDL2_ttf -lSDL2_mixer

# Find all cpp files recursively
SRC = $(shell find . -name "*.cpp")
OBJ = $(SRC:.cpp=.o)
TARGET = flappybird

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
