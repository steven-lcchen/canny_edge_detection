// Convert color into gray scale.
// Gray = R*0.299 + G*0.587 + B*0.114
// w/o use float point.
//
// By Steven Chen

#include "define.hpp"

#include <iostream>
using namespace std;

#include <opencv2/opencv.hpp>
using namespace cv;
 
void MyColorToGray(const Mat& src, Mat& img)
{
  if (src.channels() == 3) {
    /// Convert the image to grayscale
    #ifdef OCV_CVTCOLOR
      cvtColor( src, img, CV_BGR2GRAY );

    #else
      for (int y=0; y<img.rows; y++)
        for (int x=0; x<img.cols; x++) {
          uchar B = src.at<Vec3b>(y, x)[0];
          uchar G = src.at<Vec3b>(y, x)[1];
          uchar R = src.at<Vec3b>(y, x)[2];
          // img.at<uchar>(y, x) = R*0.299 + G*0.587 + B*0.114;
          img.at<uchar>(y, x) = ((uint)R*19595 + (uint)G*38469 + (uint)B*7472) >> 16;
        }
    #endif
  } else {
    img = src.clone();
  }
}


