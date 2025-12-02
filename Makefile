CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra
LDFLAGS := -lmd4c -lmd4c-html

SRC := $(wildcard src/*.cpp)
BIN := dist/md2html

all: $(BIN)

$(BIN): $(SRC) | dist
	$(CXX) $(CXXFLAGS)  -o $@ $(SRC) $(LDFLAGS)

dist:
	mkdir -p dist

clean:
	rm -rf dist/*
