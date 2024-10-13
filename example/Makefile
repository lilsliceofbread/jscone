.PHONY: build run

build:
	mkdir -p ../build
	gcc -g -Wall -Wpedantic -I.. example.c -o ../build/example

run: build
	../build/example