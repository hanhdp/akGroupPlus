SHELL := /bin/bash

CXX := g++ -std=c++11
CXXFLAGS := -g -O3 -Wall -Wno-unused-result -fcilkplus -lcilkrts -msse4 #-fopenmp # -DNDEBUG
SOURCES := $(shell find . | grep -e ".cpp" | grep -v "main.cpp" | grep -v "convert" | grep -v "weighted" | grep -v "bench")
OBJECTS := $(SOURCES:%.cpp=%.o)

all: main

main: $(OBJECTS) main.cpp
	$(CXX) $(CXXFLAGS) $^ -o main32
run: main
	./harness init-file.txt workload-file.txt result-file.txt ./main

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf main $(OBJECTS)
