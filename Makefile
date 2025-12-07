CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra
LDFLAGS := -lmd4c -lmd4c-html

SRC := $(wildcard src/*.cpp)
BIN := dist/md2html

all: $(BIN) styles static

$(BIN): $(SRC) | dist
	$(CXX) $(CXXFLAGS)  -o $@ $(SRC) $(LDFLAGS)

styles: | dist
	cp templates/styles.css dist/styles.css

.PHONY: static
static: | dist
	cp -r static/* dist/

dist:
	mkdir -p dist

clean:
	rm -rf dist/*

build: all
	./$(BIN)
