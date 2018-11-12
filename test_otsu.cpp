#include <iostream>
using namespace std;

#include <opencv2/opencv.hpp>
using namespace cv;
 
int otsu_threshold (const Mat& frame);

/** @function main */
int main( int argc, char** argv )
{
  Mat src;
  int loThreshold = 30;
  int hiThreshold = 90;
  int const max_Threshold = 255;

  if (argc != 2) {
    cout << "Using Trackbar to adjust Canny edge detection:" << endl;
    cout << argv[0] << " image_file" << endl;
    return -1;
  }
  /// Load an image
  src = imread(argv[1], IMREAD_UNCHANGED);
  if( !src.data ) {
    cout << "Fail to open file: " << argv[1] << endl << endl;
    return -1;
  }

  Mat dst;
  cvtColor( src, dst, CV_BGR2GRAY );
  imshow( "SRC img", dst);
  int lo_threshold = otsu_threshold (dst);
  threshold(dst, dst, lo_threshold, 255, THRESH_BINARY);
  imshow( "OTSU img", dst);
  waitKey(0);
}


int otsu_threshold (const Mat& frame)
{
  const int GrayScale = 256;
  int width = frame.cols;
  int height = frame.rows;
  long pixelSum = width * height; 
  int historgram[GrayScale] = {0};
  float pixelPro[GrayScale] = {0};
  int i, j, threshold = 0;
  uchar* data = (uchar*)frame.data;

  // Count pixels at each gray level
  for (i = 0; i < height; i++)
  {
    for (j = 0; j < width; j++)
    {
      historgram[frame.at<uchar>(i, j)]++;
    }
  }

  // calculate percentage of gray levels to whole image & its maximum
  // float maxPro = 0.0;
  // int kk = 0;
  for (i = 0; i < GrayScale; i++)
  {
     pixelPro[i] = (float)historgram[i] / pixelSum;
     // if (pixelPro[i] > maxPro)
     // {
     //    maxPro = pixelPro[i];
     //    kk = i;
     // }
  }

  float w0, w1, u0tmp, u1tmp, u0, u1, u, deltaTmp, deltaMax = 0;
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
  // To iterate each gray level, and get the gray level as threshold make delta has maximum value

  //iterate each gray level [0:255]
  for (i = 0; i < GrayScale; i++)
  {
     w0 = w1 = u0tmp = u1tmp = u0 = u1 = u = deltaTmp = 0;
     for (j = 0; j < GrayScale; j++)
     {
        if (j <= i)   // background
        {
           w0 += pixelPro[j];
           u0tmp += j * pixelPro[j];
        }
        else   // foreground
        {
           w1 += pixelPro[j];
           u1tmp += j * pixelPro[j];
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

  return threshold;
}
