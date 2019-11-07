OPENCV = $(shell pkg-config --cflags --libs opencv4)
	
localize:
	g++ -std=c++11 -o localize localize.cpp $(OPENCV)

