.PHONY: build run
IS_WIN=0
ifeq ($(OS),Windows_NT)
	IS_WIN=1
endif

build:
ifeq ($(IS_WIN),1)
ifeq ("$(wildcard ..\build)","")
	cmd.exe /c mkdir ..\build
endif
else
	mkdir -p ../build
endif
	gcc -g -Wall -Wpedantic -I.. -std=c99 example.c -o ../build/example


run: build
	../build/example
