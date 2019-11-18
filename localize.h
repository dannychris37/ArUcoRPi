#ifndef HEADER_SEEN
#define HEADER_SEEN

#include "opencv2/opencv.hpp"
#include "opencv2/aruco.hpp"
#include <iostream>

/** UDP libs **/

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <ctime>
#include <unistd.h>
#include <stdlib.h>

#define USE_VIDEO_FILE		0
#define VIDEO_FILE			"aruco-vid1.mp4"
#define CAMERA_NO			1
#define STATIC_OUTPUT		1

using namespace std;
using namespace cv;

#define PORT    "3000"
#define REC_IP  "192.168.0.4"

time_t t;
double now;
Vec<double, 8> loc_data;


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
Mat camMatrix, distCoeffs;

vector<bool> sent_data(100);

Matx33d f_rotMat;
Vec3d f_tvec;
int f_markerID;

bool found_fixedM = false;

//printing
bool print_flag = false;
int print_cnt = 0;

#endif
