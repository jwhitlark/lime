CXXFLAGS += -Iinclude -std=c++11

all: bin/lime

bin/lime: src/lime.o src/interpreter.o src/core.o src/builtins.o src/parse.o src/eval.o src/expand.o
	g++ -o bin/lime src/lime.o src/interpreter.o src/core.o src/builtins.o src/parse.o src/eval.o src/expand.o

clean:
	rm -f src/*.o