// Box Filter: all filter element are 1. average all neighber pixels.
//
// By Steven Chen

#include <iostream>
using namespace std;

#include <opencv2/opencv.hpp>
using namespace cv;
 
void BoxFilter(const Mat& src, Mat& dst)
{
  // Duplicate boundary for image
  Mat raster = Mat::zeros(src.rows+2, src.cols+2, src.type()); 
  src.copyTo(raster(Rect(1, 1, src.cols, src.rows)));
  raster.row(1).copyTo(raster.row(0));
  raster.col(1).copyTo(raster.col(0));
  raster.col(raster.cols-2).copyTo(raster.col(raster.cols-1));
  raster.row(raster.rows-2).copyTo(raster.row(raster.rows-1));

  // Average
  unsigned int average;
  int width = raster.cols;
  int height = raster.rows;
  for (int i = 1; i < height - 1; i++) {
    for (int j = 1; j < width - 1; j++) {
      average = 0;
      for (int m = -1; m <= 1; m++) {
        for (int n = -1; n <= 1; n++)
          average += raster.at<uchar>(i+m, j+n);
      }
//       dst.at<uchar>(i-1, j-1) = average/9;
      average -= raster.at<uchar>(i, j);
      dst.at<uchar>(i-1, j-1) = average/8;
    }
  }
}
 

