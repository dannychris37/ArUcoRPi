#include "localize.h"
#include "udp.cpp"

static bool readCameraParameters(string filename, Mat &camMatrix, Mat &distCoeffs) {
    cv::FileStorage fs(filename,cv::FileStorage::READ);
    if(!fs.isOpened())
        return false;
    fs["camera_matrix"] >> camMatrix;
    fs["distortion_coefficients"] >> distCoeffs;
    return true;
}

void getEulerAngles(Mat &rotCamerMatrix, Vec3d &eulerAngles) {

    Mat cameraMatrix, rotMatrix, transVect, rotMatrixX,	rotMatrixY, rotMatrixZ;

    double* _r = rotCamerMatrix.ptr<double>();

    double projMatrix[12] = 
    {	_r[0], _r[1], _r[2], 0,
	_r[3], _r[4], _r[5], 0,
        _r[6], _r[7], _r[8], 0
    };

    decomposeProjectionMatrix( 
    	Mat(3,4,CV_64FC1,projMatrix),
        cameraMatrix,
        rotMatrix,
        transVect,
        rotMatrixX,
        rotMatrixY,
        rotMatrixZ,
        eulerAngles
    );
}

void makeSense(Vec3d tvec, Vec3d rvec, int markerID, int camera_no){
    
    /** Fixed marker IDs **/
    if(markerID < 50) {
        
        Mat rotMatrix;

        // converts a rotation matrix to a 
        // rotation vector or viceversa
	Rodrigues(rvec, rotMatrix);

	// transpose rotation matrix
        transpose(rotMatrix, f_rotMat);
        f_tvec = -1 * tvec;
        f_markerID = markerID;

    }

    else{
        
        // only moving markers
        if (markerID > 50){  
           
           // if all x pointing in same direction
            Matx33d rotMattoe0( 
            	0.0, -1.0, 0.0,
            	1.0, 0.0, 0.0,
            	0.0, 0.0, 1.0
            );

            // 
            Vec3d reading = 
            	(rotMattoe0 * (f_rotMat * (tvec + f_tvec))) + 
            	transtoe0[f_markerID];

            if(print_flag){
				cout << "\nCAM: using fixed marker ID:" << f_markerID << endl;
				cout << "CAM: origin to truck :" << markerID << "\t" << reading << endl;
		    }
            
            Mat rotationMatrix;
            Rodrigues(rvec, rotationMatrix);
            Vec3d angle_rot;

            
            getEulerAngles(rotationMatrix, angle_rot);
            if(print_flag) cout << "CAM: rotation angle(deg):" << "\t" << angle_rot << endl;
            
            if (sent_data[markerID-51] == 0){
				if(print_flag){
				    cout << "\nSEND: Cam "<<camera_no<<" first to find marker "<<markerID<<endl;
				    cout << "SEND: Coordinates to send:\t" << reading << endl;
				    cout << "SEND: Angles to send:\t\t" << angle_rot << endl;
				}
            	// angles in degreee and x,y,z 
                UDPFarewell(markerID, reading, angle_rot);
                sent_data[markerID-51] = 1;

            }
            
            else{

                if(print_flag) {
		    		cout << "\nSKIP: Cam "<<camera_no<<" skipped: " << markerID << endl;
				}

            }
        }
    }
}

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

			    makeSense(single_tvec[0], single_rvec[0], markerIds[i], CAMERA_NO);

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

			    makeSense(single_tvec[0], single_rvec[0], markerIds[i], CAMERA_NO);

		    }
		}
	}
}

int main(){

	UDPSet(senderIP);
	
	VideoCapture cap;
	
	if(USE_VIDEO_FILE){
	    cap.open(VIDEO_FILE);
	} else {
	    cap.open(0);
	}
	
	
	// Check if camera opened successfully
	if(!cap.isOpened()){
		cout << "Error opening video stream or file" << endl;
		return -1;
	}
	
	readCameraParameters("./calibration-files/cameraCalib.yml", camMatrix, distCoeffs);

	if(!STATIC_OUTPUT) print_flag = true;
	
	while(1){
	    
		if(print_flag) cout<<"\n---------- LOCALIZATION LOOP START ----------"<<endl;
		
		// assigns 0's to sent_data vector (vector of 2 bools inside image_proc.c)
		// 1st value is for marker id 51, 2nd is for marker id 52 etc. (markers used on trucks)
		// used to skip marker if seen again on another camera
		sent_data.assign(sent_data.size(), 0);
		
		Mat frame;
		
		//capture frame by frame
		cap >> frame;
		
		if(frame.empty())
			break;
			
		processFrame(frame);
			
		//display frame
		imshow("Frame", frame);
		
		if(print_flag) cout<<"\n----------  LOCALIZATION LOOP END  ----------"<<endl;
		
		if(STATIC_OUTPUT){
		    if(print_cnt == 10){
				system("clear");
				print_flag = true;
				print_cnt = 0;
		    } else{
				print_flag = false;
				print_cnt++;
		    }
		    
		}
		    
		
		char c=(char)waitKey(25);
		if(c==27)
			break;
		
	}
	
	cap.release();
	
	destroyAllWindows();
	
	return 0;
	
}
