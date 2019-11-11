#include "localize.h"

void processFrame(Mat frame){
	
	vector<int> markerIds;
	vector<vector<Point2f>> markerCorners,rejectedCandidates;
	Ptr<aruco::DetectorParameters> detectorParams= aruco::DetectorParameters::create();
    
	// ArUco module function that performs marker detection
	aruco::detectMarkers(
		frame,			// image where markers are detected
		dictionary,		// dictionary used
		markerCorners,		// where to store marker corners
		markerIds,		// where to store marker ID's
		detectorParams,		// params that can be customized during detection
		rejectedCandidates 	// where to store rejected candidates
	);
	
	cv::aruco::drawDetectedMarkers(frame, markerCorners, markerIds);
	
	// if markers have same dictionary 
    if (markerIds.size() > 0) {

		// sending on fixed marker first
		found_fixedM = 0;

		for(unsigned int i = 0; i < markerIds.size(); i++){ 
			
			vector<vector<Point2f>> single_markerCorner;
			vector<Vec3d> single_rvec, single_tvec;
			single_markerCorner.resize(1);
			single_markerCorner[0] = markerCorners[i];

			/** Fixed marker ID **/
			if (markerIds[i] < 50){

				// estimate pose
			    aruco::estimatePoseSingleMarkers(
				single_markerCorner,       	// marker corners
				markerLength_fixed,        	// size of marker
				camMatrix,      		// camera calibration parameter (known a priori)
				distCoeffs,     		// camera calibration parameter (known a priori)
				single_rvec,               	// rotation vector of fixed marker
				single_tvec                	// translation vector of fixed marker
			    );

			    //std::cout<<"fixed markers:"<<markerIds[i]<<std::endl;

			    // draws X, Y, Z axes
			    aruco::drawAxis(
				frame, 
				camMatrix, 
				distCoeffs, 
				single_rvec[0], 
				single_tvec[0],
				    markerLength_fixed*0.5f
				);

			    //makeSense(single_tvec[0], single_rvec[0], markerIds[i], camera_no);

			    found_fixedM = 1;

			}
		}
	}
		
	/** Send moving markers **/
	if(found_fixedM == 1){

		for(unsigned int i = 0; i < markerIds.size(); i++){

		    vector<vector<cv::Point2f> > single_markerCorner;
		    vector<Vec3d> single_rvec,single_tvec;
		    single_markerCorner.resize(1);
		    single_markerCorner[0] = markerCorners[i];
		    
		    /** Moving marker ID **/
		    if(markerIds[i] > 50){

			    aruco::estimatePoseSingleMarkers( 
				single_markerCorner,       	// marker corners
				markerLength_moving,       	// size of marker
				camMatrix,      		// camera calibration parameter (known a priori)
				distCoeffs,     		// camera calibration parameter (known a priori)
				single_rvec,               	// rotation vector of moving marker
				single_tvec                	// translation vector of moving marker
			    );

			    aruco::drawAxis(
				frame, 
				camMatrix, 
				distCoeffs, 
				single_rvec[0], 
				single_tvec[0],
				markerLength_moving*0.5f
			    );

			    //makeSense(single_tvec[0], single_rvec[0], markerIds[i], camera_no);

		    }
		}
	}
}

int main(){
	
	VideoCapture cap("aruco-vid1.mp4");
	
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
			
		processFrame(frame);
			
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
