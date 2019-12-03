#ifndef HEADER_SEEN
#define HEADER_SEEN

#include "opencv2/opencv.hpp"
#include "opencv2/aruco.hpp"
#include <iostream>
#include <sys/time.h>
#include <time.h>

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

//thread libs
#include <thread>
#include <mutex>         
#include <condition_variable>

#define USE_VIDEO_FILE		0
#define VIDEO_FILE			"aruco-vid1.mp4"
#define MILLION                    1000000L
#define RPI_NO                     6

using namespace std;
using namespace cv;

/** PROCESSING AND LOCALIZATION **/
vector<Vec3d> transtoe0 {   
                            {0.19,0.055,0.0},     //0 fixed marker in dM
                            {0.19,0.4113,0.0},    //1 fixed marker
                            {0.268,0.0549,0.0},   //2 fixed marker
                            {0.343,0.41,0.0},     //3 fixed marker
                            {0.5269,0.055,0.0},   //4 fixed marker 
                            {0.4865,0.4098,0.0},  //5 fixed marker
                            {0.6559,0.0549,0.0},  //6 fixed marker
                            {0.6544,0.41,0.0},    //7 fixed marker 
                            {0.19,0.4113,0.0}     //8 fixed marker
};


// struct used to store coordinates and angles of moving markers
typedef struct  {
	bool valuesStored;
	int fixedMarker;
	Vec3d coords;
	Vec3d angles;
} markerData;

markerData dataToProcess[100][6];

// camera matrix and distance coefficients
Mat camMatrix, distCoeffs;

vector<bool> received_data(100);

Matx33d f_rotMat;
Vec3d f_tvec;
int f_markerID;

bool found_fixedM = false;

//printing
bool print_flag = false;
int print_cnt = 0;

#endif
