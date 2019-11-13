/** Code based on 
 * https://github.com/opencv/opencv_contrib/blob/master/modules/aruco/samples/calibrate_camera_charuco.cpp
 * **/

#include "localize.h"
#include <opencv2/aruco/charuco.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace {
const char* about =
        "Calibration using a ChArUco board\n"
        "  To capture a frame for calibration, press 'c',\n"
        "  If input comes from video, press any key for next frame\n"
        "  To finish capturing, press 'ESC' key and calibration starts.\n";
const char* keys  =
        "{v        | ./calibration-files/calibration-saved.avi      | Input from video file, if ommited, input comes from camera }"
        "{ci       | 0     | Camera id if input doesnt come from video (-v) }"
        "{rs       | false | Apply refind strategy }"
        "{zt       | false | Assume zero tangential distortion }"
        "{pc       | false | Fix the principal point at the center }"
        "{sc       | false | Show detected chessboard corners after calibration }";
}

static bool saveCameraParams(const string &filename, Size imageSize, float aspectRatio, int flags,
                             const Mat &cameraMatrix, const Mat &distCoeffs, double totalAvgErr) {
    FileStorage fs(filename, FileStorage::WRITE);
    if(!fs.isOpened())
        return false;

    time_t tt;
    time(&tt);
    struct tm *t2 = localtime(&tt);
    char buf[1024];
    strftime(buf, sizeof(buf) - 1, "%c", t2);

    fs << "calibration_time" << buf;

    fs << "image_width" << imageSize.width;
    fs << "image_height" << imageSize.height;

    if(flags & CALIB_FIX_ASPECT_RATIO) fs << "aspectRatio" << aspectRatio;

    if(flags != 0) {
        sprintf(buf, "flags: %s%s%s%s",
                flags & CALIB_USE_INTRINSIC_GUESS ? "+use_intrinsic_guess" : "",
                flags & CALIB_FIX_ASPECT_RATIO ? "+fix_aspectRatio" : "",
                flags & CALIB_FIX_PRINCIPAL_POINT ? "+fix_principal_point" : "",
                flags & CALIB_ZERO_TANGENT_DIST ? "+zero_tangent_dist" : "");
    }

    fs << "flags" << flags;

    fs << "camera_matrix" << cameraMatrix;
    fs << "distortion_coefficients" << distCoeffs;

    fs << "avg_reprojection_error" << totalAvgErr;

    return true;
}

bool checkMat(Mat M, String name){
	
	if(M.dims < 0){
		cout<<"\nMatrix "<<name<<" has a negative dimension"<<endl;
		return false;
	}
	else if(M.dims > 32){
		cout<<"\nMatrix "<<name<<" has a dimension alrger than 32 "<<endl;
		return false;
	}
	
	return true;
}

int main(int argc, char *argv[]){
	
	// board
	Ptr<aruco::CharucoBoard> charucoBoard = aruco::CharucoBoard::create(
		5, 
		7, 
		0.04, 
		0.02, 
		dictionary
	);
	Mat boardImage;
	charucoBoard->draw({640, 480}, boardImage, 10, 1);
	imwrite("ArucoBoard.png", boardImage);
	Ptr<aruco::Board> board = charucoBoard.staticCast<aruco::Board>();
	
	// detector parameters same as in detect charuco tutorial
	Ptr<aruco::DetectorParameters> params = cv::aruco::DetectorParameters::create();
	params->cornerRefinementMethod = aruco::CORNER_REFINE_NONE;
	
	// parser
	CommandLineParser parser(argc, argv, keys);
    parser.about(about);
    
	float aspectRatio = 1;
    int calibrationFlags = 0;
    
    // show corners after calib
    bool showChessboardCorners = parser.get<bool>("sc");
    
	// cam id default 0
    int camId = parser.get<int>("ci");
    String video;

    if(parser.has("v")) {
        video = parser.get<String>("v");
    }

    if(!parser.check()) {
        parser.printErrors();
        return 0;
    }

	VideoCapture inputVideo;
	
	int waitTime;
    if(!video.empty()) {
        inputVideo.open(video);
        waitTime = 0;
    } else {
        inputVideo.open(camId);
        waitTime = 10;
    }
	
	if(!inputVideo.isOpened())  // check if we succeeded
	{        
		cout << "Camera Init Fail\n";
		return -1;
	}
	
	vector<int> ids;
	vector<vector<Point2f>> corners, rejected;
	
	// collect data from each frame
    vector< vector< vector< Point2f > > > allCorners;
    vector< vector< int > > allIds;
    vector< Mat > allImgs;
    Size imgSize;
	
	while (inputVideo.grab()) {
		
		Mat image, imageCopy;
		inputVideo.retrieve(image);
		
		aruco::detectMarkers(image, dictionary, corners, ids, params, rejected);
		
		Mat currentCharucoCorners, currentCharucoIds;
		
		//value 10 arbitrarily chosen (my board has 17 markers)
		if(ids.size() > 10){
			aruco::interpolateCornersCharuco(corners, ids, image, charucoBoard, currentCharucoCorners,
                                             currentCharucoIds);
		}
		
        image.copyTo(imageCopy);
		putText(imageCopy, "Press 'c' to add current frame. 'ESC' to finish and calibrate",
			Point(10, 20), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 0, 0), 2);
        if(ids.size() > 10){ 
			
			aruco::drawDetectedMarkers(imageCopy, corners);
        
			if(currentCharucoCorners.total() > 0){
				aruco::drawDetectedCornersCharuco(imageCopy, currentCharucoCorners, currentCharucoIds);
			
				char key = (char) waitKey(25);
				if (key == 27) break;
				
				//uncomment if you want to capture frames yourself
				/*if(key == 'c' && ids.size() > 0) {
					cout << "Frame captured" << endl;
					allCorners.push_back(corners);
					allIds.push_back(ids);
					allImgs.push_back(image);
					imgSize = image.size();
				}*/
				
				cout << "Frame captured" << endl;
				allCorners.push_back(corners);
				allIds.push_back(ids);
				allImgs.push_back(image);
				imgSize = image.size();
				
			}// if corners
			
		}// if id size
		
		imshow("out", imageCopy);	
        
	}
        
	if(allIds.size() < 1) {
		cerr << "Not enough captures for calibration" << endl;
		return 0;
	}
	
	Mat cameraMatrix, distCoeffs;
	vector< Mat > rvecs, tvecs;
	double repError;
	
	// prepare data for calibration
	vector< vector< Point2f > > allCornersConcatenated;
	vector< int > allIdsConcatenated;
	vector< int > markerCounterPerFrame;
	markerCounterPerFrame.reserve(allCorners.size());
	for(unsigned int i = 0; i < allCorners.size(); i++) {
		markerCounterPerFrame.push_back((int)allCorners[i].size());
		for(unsigned int j = 0; j < allCorners[i].size(); j++) {
			allCornersConcatenated.push_back(allCorners[i][j]);
			allIdsConcatenated.push_back(allIds[i][j]);
		}
	}
	
	// calibrate camera using aruco markers
    double arucoRepErr;
    
    arucoRepErr = aruco::calibrateCameraAruco(allCornersConcatenated, allIdsConcatenated,
                                              markerCounterPerFrame, board, imgSize, cameraMatrix,
                                              distCoeffs, noArray(), noArray(), calibrationFlags);
	
	// prepare data for charuco calibration
	int nFrames = (int)allCorners.size();
	cout<<"\nNumber of frames is "<<nFrames<<endl;
	vector< Mat > allCharucoCorners;
	vector< Mat > allCharucoIds;
	vector< Mat > filteredImages;
	allCharucoCorners.reserve(nFrames);
	allCharucoIds.reserve(nFrames);
        
    for(int i = 0; i < nFrames; i++) {
		
        // interpolate using camera parameters
        Mat currentCharucoCorners, currentCharucoIds;  
              		
		aruco::interpolateCornersCharuco(allCorners[i], allIds[i], allImgs[i], charucoBoard,
										 currentCharucoCorners, currentCharucoIds, cameraMatrix,
										 distCoeffs);

		allCharucoCorners.push_back(currentCharucoCorners);
		allCharucoIds.push_back(currentCharucoIds);
		filteredImages.push_back(allImgs[i]);
		
		cout<<"Interpolated corners frame "<<i<<endl;
		
    }
    
    if(allCharucoCorners.size() < 4) {
        cerr << "Not enough corners for calibration" << endl;
        return 0;
    }

    // calibrate camera using charuco
    repError =
        aruco::calibrateCameraCharuco(allCharucoCorners, allCharucoIds, charucoBoard, imgSize,
                                      cameraMatrix, distCoeffs, rvecs, tvecs, calibrationFlags);

	string outputFile = "./calibration-files/cameraCalib.yml";
    bool saveOk =  saveCameraParams(outputFile, imgSize, aspectRatio, calibrationFlags,
                                    cameraMatrix, distCoeffs, repError);
    if(!saveOk) {
        cerr << "Cannot save output file" << endl;
        return 0;
    }
    
    cout << "Rep Error: " << repError << endl;
    cout << "Rep Error Aruco: " << arucoRepErr << endl;
    cout << "Calibration saved to " << outputFile << endl;

    // show interpolated charuco corners for debugging
    if(showChessboardCorners) {
        for(unsigned int frame = 0; frame < filteredImages.size(); frame++) {
            Mat imageCopy = filteredImages[frame].clone();
            if(allIds[frame].size() > 0) {

                if(allCharucoCorners[frame].total() > 0) {
                    aruco::drawDetectedCornersCharuco( imageCopy, allCharucoCorners[frame],
                                                       allCharucoIds[frame]);
                }
            }

            imshow("out", imageCopy);
            char key = (char)waitKey(0);
            if(key == 27) break;
        }
    }

    return 0;

}




