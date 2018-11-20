// By Steven Chen
// MedianFilter: Remove extreme pixel value (noise) and replace it by medium value of neighbers.

#include <iostream>
using namespace std;

#include <opencv2/opencv.hpp>
using namespace cv;
 
// Sort array from minimum to maximum
void Sort9(unsigned char data[9])
{
  int i;
  int j;
  int min_val;
  int index = 0;
  for (i = 0; i < 9; i++) {
    min_val = data[i];
    index = i;
    for (j = i+1; j < 9; j++) {       
      if (min_val > data[j]) {       
         min_val = data[j];
         index = j;
      }
    }
    if (i != index) {
      data[index] = data[i];
      data[i] = min_val;
    }
  }
}

// MedianFilter: Remove extreme pixel value (noise) and replace it by medium value of neighbers.
void MedianFilter(const Mat& src, Mat& dst)
{
  Mat raster = Mat::zeros(src.rows+2, src.cols+2, src.type()); 
  // Copy & Duplicate boundary for image
  for (int y=0; y<raster.rows; y++) {
    for (int x=0; x<raster.cols; x++) {
      if (y==0)
        if (x==0)
          raster.at<uchar>(y, x) = src.at<uchar>(y, x);
        else if (x==raster.cols-1)
          raster.at<uchar>(y, x) = src.at<uchar>(y, x-2);
        else
          raster.at<uchar>(y, x) = src.at<uchar>(y, x-1);
      else if (y==raster.rows-1) 
        if (x==0)
          raster.at<uchar>(y, x) = src.at<uchar>(y-2, x);
        else if (x==raster.cols-1)
          raster.at<uchar>(y, x) = src.at<uchar>(y-2, x-2);
        else
          raster.at<uchar>(y, x) = src.at<uchar>(y-2, x-1);
      else
        if (x==0)
          raster.at<uchar>(y, x) = src.at<uchar>(y-1, x);
        else if (x==raster.cols-1)
          raster.at<uchar>(y, x) = src.at<uchar>(y-1, x-2);
        else
          raster.at<uchar>(y, x) = src.at<uchar>(y-1, x-1);
    }
  }

  int width = raster.cols;
  int height = raster.rows;
  int d;
  unsigned char data[9];

  // Execute medium filtering
  for (int y = 1; y < height - 1; y++) {
     for (int x = 1; x < width - 1; x++) {
        d = 0;
        for (int m = -1; m <= 1; m++) {
           for (int n = -1; n <= 1; n++) {
              data[d] = raster.at<uchar>(y+m, x+n);
              d++;
           }
         }
         Sort9(data);
         dst.at<uchar>(y-1, x-1) = data[4];
      }
   }
}

