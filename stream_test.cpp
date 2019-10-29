#include "opencv2/opencv.hpp"
#include <iostream>

using namespace std;
using namespace cv;

int main(){
	
	VideoCapture cap("/home/pi/Videos/test2.avi");
	
	// Check if camera opened successfully
	if(!cap.isOpened()){
		cout << "Error opening video stream or file" << endl;
		return -1;
	}
	
	while(1){
		
		Mat frame;
		
		//capture frame by frame
		cap >> frame;
		
		if(frame.empty())
			break;
			
		//display frame
		imshow("Frame", frame);
		
		char c=(char)waitKey(25);
		if(c==27)
			break;
		
	}
	
	cap.release();
	
	destroyAllWindows();
	
	return 0;
	
}
