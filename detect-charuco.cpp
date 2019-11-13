#include "localize.h"
#include "opencv2/aruco/charuco.hpp"
#include "opencv2/imgcodecs.hpp"

int main(){
	
	Ptr<aruco::CharucoBoard> board = aruco::CharucoBoard::create(
		5, 
		7, 
		0.04, 
		0.02, 
		dictionary);
	Mat boardImage;
	board->draw({640, 480}, boardImage, 10, 1);
	imwrite("ArucoBoard.png", boardImage);
	

	// detection
	VideoCapture inputVideo(0);
	
	if(!inputVideo.isOpened())  // check if we succeeded
	{        
		cout << "Camera Init Fail\n";
		return -1;
	}
		
	Ptr<aruco::DetectorParameters> params = cv::aruco::DetectorParameters::create();
	params->cornerRefinementMethod = aruco::CORNER_REFINE_NONE;
	
	while (inputVideo.grab()) {
		
		Mat image, imageCopy;
		inputVideo.retrieve(image);
		image.copyTo(imageCopy);
		
		vector<int> ids;
		vector<vector<Point2f>> corners;
		aruco::detectMarkers(image, dictionary, corners, ids, params);
		
		// if at least one marker detected
		if (ids.size() > 0) {
			aruco::drawDetectedMarkers(imageCopy, corners, ids);
			vector<Point2f> charucoCorners;
			vector<int> charucoIds;
			aruco::interpolateCornersCharuco(corners, ids, image, board, charucoCorners, charucoIds);
			// if at least one charuco corner detected
			if(charucoIds.size() > 0){
				aruco::drawDetectedCornersCharuco(imageCopy, charucoCorners, charucoIds, Scalar(255, 0, 0));
				cv::Vec3d rvec, tvec;
				bool valid = cv::aruco::estimatePoseCharucoBoard(charucoCorners, charucoIds, board, cameraMatrix, distCoeffs, rvec, tvec);
				// if charuco pose is valid
				if(valid)
					cv::aruco::drawAxis(imageCopy, cameraMatrix, distCoeffs, rvec, tvec, 0.1);
			}
		}
		imshow("out", imageCopy);
		char key = (char) waitKey(25);
		if (key == 27)
			break;
	}

}




