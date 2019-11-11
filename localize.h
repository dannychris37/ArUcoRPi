#include "opencv2/opencv.hpp"
#include "opencv2/aruco.hpp"
#include <iostream>

using namespace std;
using namespace cv;

Ptr<aruco::Dictionary> dictionary = aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);

/** PROCESSING AND LOCALIZATION **/
vector<cv::Vec3d> transtoe0 {{0.19,0.055,0.0},     //0 fixed marker in dM
                                  {0.19,0.4113,0.0},    //1 fixed marker
                                  {0.268,0.0549,0.0},   //2 fixed marker
                                  {0.343,0.41,0.0},     //3 fixed marker
                                  {0.5269,0.055,0.0},   //4 fixed marker 
                                  {0.4865,0.4098,0.0},  //5 fixed marker
                                  {0.6559,0.0549,0.0},  //6 fixed marker
                                  {0.6544,0.41,0.0},    //7 fixed marker 
                                  {0.19,0.4113,0.0}     //8 fixed marker
};

// only for fixed markers
const float markerLength_fixed      = 0.0203;

// only for moving markers
const float markerLength_moving     = 0.013;//0.0097;


// struct used to store coordinates and angles of moving markers
typedef struct  {
	bool valuesStored;
	int fixedMarker;
	cv::Vec3d coords;
	cv::Vec3d angles;
} markerData;

markerData dataToSend[100];
bool markerFound[100] = {false};

// camera matrix and distance coefficients
vector<Mat> camMatrix, distCoeffs;

bool found_fixedM = false;
