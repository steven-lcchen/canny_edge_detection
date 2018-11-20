/*
  Topic: Canny Edge Detection
    Showing how to detect edges using the Canny Algorithm

 * @function MyCanny
 * 1. Get Gradient's magnitude
 * 2. Non-Maximum Suppression
 * 3. hystersis threshold

  Date: 107/10/15
  Author: Steven Chen
*/

#include "define.hpp"

#include <iostream>
using namespace std;

#include <opencv2/opencv.hpp>
using namespace cv;
 
// int otsu_threshold (const Mat& src, Mat& dst, int typ=0);

void MySobel (const Mat& src, Mat& grad_x, Mat& grad_y)
{
  // Horizontal filter
  const Mat gx = (Mat_<int>(3, 3) << -1, 0, 1, -2, 0, 2, -1, 0, 1);
  // Vertical filter
  const Mat gy = (Mat_<int>(3, 3) << -1, -2, -1, 0, 0, 0, 1, 2, 1);

  Mat raster;
  // Duplicate boundary for image
  raster = Mat::zeros(src.rows+2, src.cols+2, src.type()); 
  src.copyTo(raster(Rect(1, 1, src.cols, src.rows)));
  raster.row(1).copyTo(raster.row(0));
  raster.col(1).copyTo(raster.col(0));
  raster.col(raster.cols-2).copyTo(raster.col(raster.cols-1));
  raster.row(raster.rows-2).copyTo(raster.row(raster.rows-1));

  // Calculate Gx/Gy gradient
  for (int y=1; y<raster.rows-1; y++) {
    for (int x=1; x<raster.cols-1; x++) {
      for (int m=-1; m<=1; m++) {
        for (int n=-1; n<=1; n++) {
          grad_x.at<short>(y-1, x-1) += raster.at<uchar>(y+m, x+n) * gx.at<int>(1+m, 1+n);
          grad_y.at<short>(y-1, x-1) += raster.at<uchar>(y+m, x+n) * gy.at<int>(1+m, 1+n);
        } // n
      } // m
    } // x
  } // y
}



/*
 * @function MyCanny
 * 1. Get Gradient's magnitude
 * 2. Non-Maximum Suppression
 * 3. hystersis threshold
 */
void MyCanny(const Mat& src, Mat& detected_edges, int lo_threshold, int hi_threshold, bool L2gradient=true, bool debug=false)
{
  Mat grad_x, grad_y; // CV_16S
  grad_x = Mat::zeros(src.rows, src.cols, CV_16S);
  grad_y = Mat::zeros(src.rows, src.cols, CV_16S);

  #ifdef OCV_SOBEL
    Sobel(src, grad_x, CV_16S, 1, 0, 3, 1, 0, BORDER_DEFAULT);
    Sobel(src, grad_y, CV_16S, 0, 1, 3, 1, 0, BORDER_DEFAULT);
  #else
    MySobel(src, grad_x, grad_y);
  #endif

  // Gradient's magnitude
  Mat grad_mag; // CV_8U
  Mat abs_grad_x, abs_grad_y;
  if (L2gradient == false) {
    convertScaleAbs(grad_x, abs_grad_x); // convert to CV_8U
    convertScaleAbs(grad_y, abs_grad_y); // convert to CV_8U
    addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad_mag);
  } else {
    pow(grad_x, 2, abs_grad_x);
    pow(grad_y, 2, abs_grad_y);
    Mat sum = abs_grad_x + abs_grad_y;
    sum.convertTo(grad_mag, CV_32F); // convert to CV_32F
    sqrt(grad_mag, grad_mag);
    convertScaleAbs(grad_mag, grad_mag); // convert CV_32F to CV_8U
  }

  // Non-Maximum Suppression
  short gx, gy;
  int g1, g2, g3, g4;
  double dTemp, dTemp1, dTemp2;
  double weight;
  Mat nmax_suppress = Mat::zeros(src.rows, src.cols, src.type()); // all 0s for set all boundary are not edge
  for (int y=1; y<grad_mag.rows-1; y++) {
    for (int x=1; x<grad_mag.cols-1; x++) {
      // the gradient of current point
      gx = grad_x.at<short>(y, x);
      gy = grad_y.at<short>(y, x);
      dTemp = grad_mag.at<uchar>(y, x);

      // if gradient==0, then it is not the edge point
      if (dTemp == 0) {
        nmax_suppress.at<uchar>(y, x) = 0;
      } else { // else check gradient direction
        if (abs(gy) > abs(gx)) {
          weight = fabs(gx) / fabs(gy);
          g2 = grad_mag.at<uchar>(y-1, x);
          g4 = grad_mag.at<uchar>(y+1, x);
          if(gx*gy > 0) {
            //g1 g2
            //   C
            //   g4 g3
            g1 = grad_mag.at<uchar>(y-1, x-1);
            g3 = grad_mag.at<uchar>(y+1, x+1);
          } else { //  if(gx*gy < 0)
            //    g2 g1
            //    C
            // g3 g4
            g1 = grad_mag.at<uchar>(y-1, x+1);
            g3 = grad_mag.at<uchar>(y+1, x-1);
          }
        }
        else { // if (abs(gy) <= abs(gx))
          weight = fabs(gy) / fabs(gx);
          g2 = grad_mag.at<uchar>(y, x-1);
          g4 = grad_mag.at<uchar>(y, x+1);
          if(gx*gy > 0) {
            // g1
            // g2 C g4
            //      g3
            g1 = grad_mag.at<uchar>(y-1, x-1);
            g3 = grad_mag.at<uchar>(y+1, x+1);
          } else { // if(gx*gy < 0)
            //      g3
            // g2 C g4
            // g1
            g1 = grad_mag.at<uchar>(y+1, x-1);
            g3 = grad_mag.at<uchar>(y-1, x+1);
          }
        }
      }
      dTemp1 = weight*g1 + (1-weight)*g2;
      dTemp2 = weight*g3 + (1-weight)*g4;	
      if(dTemp>=dTemp1 && dTemp>=dTemp2) {
        nmax_suppress.at<uchar>(y, x) = grad_mag.at<uchar>(y, x);
      } else {
        nmax_suppress.at<uchar>(y, x) = 0;
      }			
    }
  }

  // Hysteresis threshold
  for (int y=1; y<nmax_suppress.rows-1; y++) {
    for (int x=1; x<nmax_suppress.cols-1; x++) {
      if (nmax_suppress.at<uchar>(y, x) >= hi_threshold)
        detected_edges.at<uchar>(y, x) = 255;
      else if (nmax_suppress.at<uchar>(y, x) < lo_threshold)
        detected_edges.at<uchar>(y, x) = 0;
      else
        if (nmax_suppress.at<uchar>(y-1, x-1) >= hi_threshold || nmax_suppress.at<uchar>(y-1, x) >= hi_threshold || nmax_suppress.at<uchar>(y-1, x+1) >= hi_threshold ||
            nmax_suppress.at<uchar>(y  , x-1) >= hi_threshold ||                                                    nmax_suppress.at<uchar>(y  , x+1) >= hi_threshold ||
            nmax_suppress.at<uchar>(y+1, x-1) >= hi_threshold || nmax_suppress.at<uchar>(y+1, x) >= hi_threshold || nmax_suppress.at<uchar>(y+1, x+1) >= hi_threshold)
          detected_edges.at<uchar>(y, x) = 255;
        else
          detected_edges.at<uchar>(y, x) = 0;
    }
  }
//// Just for comparation
//   int otsu_val = otsu_threshold (nmax_suppress, detected_edges);
//   cout << "otsu_threshold=" << otsu_val << endl;
//   int th_val = otsu_val - (otsu_val>>2);
//   cout << "set threshold=" << th_val << endl;
//   threshold(nmax_suppress, detected_edges, th_val, 255, THRESH_BINARY);

  if (debug) {
    imshow("MyCanny 1: Sobel gradient magnitude", grad_mag);
    imshow("MyCanny 2: Non-Maximum Suppression", nmax_suppress);
    imshow("MyCanny 3: Hysteresis threshold", detected_edges);
  }
}

