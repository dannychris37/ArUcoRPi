/**
 * ArUco uses the class Marker, representing a marker observed in the 
 image. Each marker is a vector of 4 points (representing the corners 
 in the image), a unique id, its size (in meters), and the translation 
 and rotation that relates the center of the marker and the camera 
 location. In the above example, you can see that the task of detecting 
 the markers is the class MarkerDetector. It is prepared to detect 
 markers of any of the Dictionaries allowed. By default, the 
 MarkerDetector will look for squares and then will analyze the binary 
 code inside. For the code extracted, it will compare against all the 
 markers in all the available dictionaries. This is however not the 
 best approach. Ideally, you should explicitly indicate the type of 
 dictionary you are using. The list of possible dictionaries is in 
 Dictionary::DICT_TYPES. 
 */

#include <iostream>
#include "opencv2/aruco.hpp"
#include "opencv2/highgui/highgui.hpp"
 int main(int argc,char **argv){
   if (argc != 2 ){ std::cerr<<"Usage: inimage"<<std::endl;return -1;}
   cv::Mat image=cv::imread(argv[1]);
   aruco::MarkerDetector MDetector;
   //detect
   std::vector<aruco::Marker> markers=MDetector.detect(image);
   //print info to console
   for(size_t i=0;i<markers.size();i++){
       std::cout<<markers[i]<<std::endl;
      //draw in the image
       markers[i].draw(image);
   }
   cv::imshow("image",image);
   cv::waitKey(0);
}
