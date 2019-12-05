#include "localize.h"
#include "udp.cpp"

bool marker_found;

static bool readCameraParameters(string filename, Mat &camMatrix, Mat &distCoeffs) {
    FileStorage fs(filename,FileStorage::READ);
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

void makeSense(Vec3d tvec, Vec3d rvec, int markerID){
    
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
            
	    if(CAM_NO != 2)
			UDPSend(markerID, f_markerID, reading, angle_rot);
	    else{
			recv_data_aggr[CAM_NO-1][0] = CAM_NO;
			recv_data_aggr[CAM_NO-1][1] = markerID;
			recv_data_aggr[CAM_NO-1][2] = f_markerID;
			recv_data_aggr[CAM_NO-1][3] = reading[0];
			recv_data_aggr[CAM_NO-1][4] = reading[1];
			recv_data_aggr[CAM_NO-1][5] = reading[2];
			recv_data_aggr[CAM_NO-1][6] = angle_rot[0];
			recv_data_aggr[CAM_NO-1][7] = angle_rot[1];
			recv_data_aggr[CAM_NO-1][8] = angle_rot[2];
	    }
	    
	    marker_found = true;
	    
        }
    }
}

void processFrame(Mat frame){
    
	marker_found = false;
	
	vector<int> markerIds;
	vector<vector<Point2f>> markerCorners,rejectedCandidates;
	Ptr<aruco::Dictionary> dictionary = aruco::getPredefinedDictionary(aruco::DICT_6X6_250);
	Ptr<aruco::DetectorParameters> detectorParams= aruco::DetectorParameters::create();

	// only for fixed markers
	const float markerLength_fixed      = 0.0203;

	// only for moving markers
	const float markerLength_moving     = 0.013;//0.0097;
    
	// ArUco module function that performs marker detection
	aruco::detectMarkers(
		frame,			// image where markers are detected
		dictionary,		// dictionary used
		markerCorners,		// where to store marker corners
		markerIds,		// where to store marker ID's
		detectorParams,		// params that can be customized during detection
		rejectedCandidates 	// where to store rejected candidates
	);
	
	aruco::drawDetectedMarkers(frame, markerCorners, markerIds);
	
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

			    //cout<<"fixed markers:"<<markerIds[i]<<endl;

			    // draws X, Y, Z axes
			    aruco::drawAxis(
				frame, 
				camMatrix, 
				distCoeffs, 
				single_rvec[0], 
				single_tvec[0],
				    markerLength_fixed*0.5f
				);

			    makeSense(single_tvec[0], single_rvec[0], markerIds[i]);

			    found_fixedM = 1;

			}
		}
	}
		
	/** Send moving markers **/
	if(found_fixedM == 1){

		for(unsigned int i = 0; i < markerIds.size(); i++){

		    vector<vector<Point2f> > single_markerCorner;
		    vector<Vec3d> single_rvec,single_tvec;
		    single_markerCorner.resize(1);
		    single_markerCorner[0] = markerCorners[i];
		    
		    /** Moving marker ID **/
		    if(markerIds[i] > 50){

			    aruco::estimatePoseSingleMarkers( 
					single_markerCorner,       	// marker corners
					markerLength_moving,       	// size of marker
					camMatrix,      			// camera calibration parameter (known a priori)
					distCoeffs,     			// camera calibration parameter (known a priori)
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

			    makeSense(single_tvec[0], single_rvec[0], markerIds[i]);

		    }
		}
	}
	
	if(!marker_found && CAM_NO!=2){
	    if(print_flag) cout<<"Marker not found, sending 0's"<<endl;
	    UDPSend(0, 0, {0,0,0}, {0,0,0});
	} else if(!marker_found && CAM_NO == 2){
		recv_data_aggr[CAM_NO-1][0] = CAM_NO;
		recv_data_aggr[CAM_NO-1][1] = 0;
		recv_data_aggr[CAM_NO-1][2] = 0;
		recv_data_aggr[CAM_NO-1][3] = 0;
		recv_data_aggr[CAM_NO-1][4] = 0;
		recv_data_aggr[CAM_NO-1][5] = 0;
		recv_data_aggr[CAM_NO-1][6] = 0;
		recv_data_aggr[CAM_NO-1][7] = 0;
		recv_data_aggr[CAM_NO-1][8] = 0;
	}
	
}

void fusion(int markerID){

	Vec3d avgAngles, avgCoords;

	for(int k = 0; k < 3; k++){
        avgCoords[k] = 0;
        avgAngles[k] = 0;
    }

    int cameraCount = 0;

    for(int j = 0; j < 6; j++){

        if(dataToProcess[markerID][j].valuesStored){

            if(print_flag){
                cout << "\nAVG: Marker " << markerID << " found by camera "<< j+1 << endl;
                cout << "AVG: Using fixed marker ID: " << dataToProcess[markerID][j].fixedMarker << endl;
                cout << "AVG: Coordinates:\t" << dataToProcess[markerID][j].coords << endl;
                cout << "AVG: Angle:\t\t" << dataToProcess[markerID][j].angles << endl << endl;
            }

            // For averaging
            for(int k = 0; k < 3; k++){
                avgCoords[k] += dataToProcess[markerID][j].coords[k];
                avgAngles[k] += dataToProcess[markerID][j].angles[k];
            }
            cameraCount++;

            // For diffs
            /*for(int di = j+1; di < 8; di++){
                if(dataToProcess[markerID][di].valuesStored){
                    diffs[j][di][0] = abs(dataToProcess[markerID][di].coords[0] - dataToProcess[markerID][j].coords[0]) * 1000;
                    diffs[di][j][0] = diffs[j][di][0];
                    diffs[j][di][1] = abs(dataToProcess[markerID][di].coords[1] - dataToProcess[markerID][j].coords[1]) * 1000;
                    diffs[di][j][1] = diffs[j][di][1];
                }
            }*/
        }

    } // for j

    for(int k = 0; k < 3; k++){
        avgCoords[k] /= cameraCount;
        avgAngles[k] /= cameraCount;
    }

    if(print_flag){
    	cout << "\nSEND: Sending final data data for marker " << markerID << endl;
	    cout << "SEND: Coordinates to send:\t" << avgCoords << endl;
	    cout << "SEND: Angles to send:\t\t" << avgAngles << endl;
	}

	UDPSendFinal(markerID, avgCoords, avgAngles);
}

int main(){

	UDPSet(false);
	
	VideoCapture cap;
	bool recvd_any;
	
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
		
		Mat frame;
		thread t[RPI_NO];

		// assigns 0's to received_data vector
		received_data.assign(received_data.size(), 0);

	    // reset valuesStored and markerFound flags
	    for(int i = 0; i < 100; i++){
	        for(int j = 0; j < 6; j++){
	            dataToProcess[i][j].valuesStored = false;
	        }
	    }
		
		//capture frame by frame
		cap >> frame;
		
		if(frame.empty())
			break;
			
		processFrame(frame);
		
		if(CAM_NO == 2){// receive all and send
		
			for(int i = 0; i<RPI_NO;i++){
			    if(i==CAM_NO-1) i++;
			    t[i] = thread(UDPRecAggr, i);
			}
			for(int i=0; i<RPI_NO; i++){
			    if(i==CAM_NO-1) i++;
			    t[i].join();
			}
			
			recvd_any = false;

			for(int i = 0; i < RPI_NO; i++){

				// get number of camera that sent
			    int camera_no = (int)recv_data_aggr[i][0];

			    int markerID = recv_data_aggr[i][1];

			    if( markerID != 0) {
			    	recvd_any = true;

			    	int camera_index = camera_no - 1;

				    dataToProcess[markerID][camera_index].fixedMarker = recv_data_aggr[i][2];
				    dataToProcess[markerID][camera_index].coords[0] = recv_data_aggr[i][3];
				    dataToProcess[markerID][camera_index].coords[1] = recv_data_aggr[i][4];
				    dataToProcess[markerID][camera_index].coords[2] = recv_data_aggr[i][5];
				    dataToProcess[markerID][camera_index].angles[0] = recv_data_aggr[i][6];
				    dataToProcess[markerID][camera_index].angles[1] = recv_data_aggr[i][7];
				    dataToProcess[markerID][camera_index].angles[2] = recv_data_aggr[i][8];
				    dataToProcess[markerID][camera_index].valuesStored = true;
				    received_data[markerID] = true;
			    }

			}
			
			if(recvd_any){
				// perform fusion on dataToProcess
				for(int i=50; i<100; i++){
					if(received_data[i]){
						fusion(i);
					}
				}
			}
			
		}
			
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
