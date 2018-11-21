// Otsu's method for image thresholding, or,
// the reduction of a graylevel image to a binary image. 
// 
// By Steven Chen
//

#include <iostream>
using namespace std;

#include <opencv2/opencv.hpp>
using namespace cv;
 
// src: input,  8-bits 1-channel gray image
// dst: output, OTSU binary image, 8-bits 1-channel binary image
// inv: output type, 0: P=(P>TH)?255:0;  1: P=(P>TH)?0:255;
int otsu_threshold (const Mat& src, Mat& dst, int inv=0)
{
  int threshold = 0;
  const int GrayScale = 256;
  int historgram[GrayScale] = {0};
  float LevelWeight[GrayScale] = {0};

  int width = src.cols;
  int height = src.rows;
  long TotalPix = width * height; 
  uchar* data = (uchar*)src.data;

  // Count pixels at each gray level
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      historgram[src.at<uchar>(i, j)]++;
    }
  }

  // calculate weight(or percentage) of gray levels to whole image
  for (int i = 0; i < GrayScale; i++) {
     LevelWeight[i] = (float)historgram[i] / TotalPix;
  }

  // Iterate each gray level, and find a gray level as threshold make delta has maximum value
  // Method description:
  // image MxN
  // N0: background pixels
  // N1: foreground pixels
  // w0: background pixels' percentage
  // w1: foreground pixels' percentage
  // u0: background pixels' average gray level
  // u1: foreground pixels' average gray level
  // u: whole image's average gray level
  // deltaXXX: variances of the two classes
  //
  // w0 = N0/M×N                   (1)
  // w1 = N1/M×N                   (2)
  // N0+N1 = M×N                   (3)
  // w0+w1 = 1                     (4)
  // u = w0*u0+w1*u1               (5)
  // delta = w0(u0-u)^2+w1(u1-u)^2 (6)
  //       = w0w1(u0-u1)^2
  float w0, w1, u0tmp, u1tmp, u0, u1, u, deltaTmp, deltaMax = 0;
  for (int i = 0; i < GrayScale; i++) { //iterate each gray level [0:255]
     w0 = w1 = u0tmp = u1tmp = u0 = u1 = u = deltaTmp = 0;
     for (int j = 0; j < GrayScale; j++)
     {
        if (j <= i)   // background
        {
           w0 += LevelWeight[j];
           u0tmp += j * LevelWeight[j];
        }
        else   // foreground
        {
           w1 += LevelWeight[j];
           u1tmp += j * LevelWeight[j];
        }
     }
     u0 = u0tmp / w0;
     u1 = u1tmp / w1;
     u = u0tmp + u1tmp;
     deltaTmp = w0 * pow((u0 - u), 2) + w1 * pow((u1 - u), 2);
     if (deltaTmp > deltaMax)
     {
        deltaMax = deltaTmp;
        threshold = i;
     }
  }

  for (int i=0; i < src.rows; i++) {
    for (int j=0; j < src.cols; j++) {
      if (inv)
        dst.at<uchar>(i, j) = (src.at<uchar>(i, j) > threshold) ? 0 : 255;
      else
        dst.at<uchar>(i, j) = (src.at<uchar>(i, j) > threshold) ? 255 : 0;
    }
  }
  return threshold;
}
