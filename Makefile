all: clean build run

build: src/*.cpp include/*.hpp
	g++ -o server src/*.cpp -Iinclude

clean:
	rm -f server

run: build
	./server
