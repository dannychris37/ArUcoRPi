OPENCV = $(shell pkg-config --cflags --libs opencv4)
	
localize:
	g++ -std=c++11 -Wno-psabi -o localize localize.cpp $(OPENCV)
	
calibrate:
	g++ -std=c++11 -Wno-psabi -o calibrate calibrate.cpp $(OPENCV)
	
record:
	g++ -std=c++11 -Wno-psabi -o record record.cpp $(OPENCV)

receive:
	g++ -std=c++11 -Wno-psabi -o receive receive.cpp $(OPENCV)
	
.PHONY: localize calibrate record receive

