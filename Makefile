CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra
LDFLAGS := -lmd4c -lmd4c-html

SRC := $(wildcard src/*.cpp)
BIN := dist/md2html

all: $(BIN) styles

$(BIN): $(SRC) | dist
	$(CXX) $(CXXFLAGS)  -o $@ $(SRC) $(LDFLAGS)

styles: | dist
	cp templates/styles.css dist/styles.css

dist:
	mkdir -p dist

clean:
	rm -rf dist/*

build: all
	./$(BIN)
