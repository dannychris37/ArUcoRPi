OPENCV = $(shell pkg-config --cflags --libs opencv4)

#all: aruco_test
	
aruco_test:
	g++ -std=c++11 -o aruco_test aruco_test.cpp $(OPENCV)
	
stream_test:
	g++ -std=c++11 -o stream_test stream_test.cpp $(OPENCV)

