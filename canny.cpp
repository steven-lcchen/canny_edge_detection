/*
  Topic: Canny Edge Detection
    Showing how to detect edges using the Canny Algorithm

  Date: 107/10/15
  Author: Steven Chen
*/

#define OCV_CMDLINE 1
// #define OCV_CVTCOLOR 1
// #define OCV_BLUR 1
// #define OCV_CANNY 1

// To use MyCanny function
#ifndef OCV_CANNY
// #define OCV_SOBEL 1
// #define REF_THRESHOLD 1
#endif
#define L2GRADIENT true // For Canny L2gradient precision: true | false
 
#include <iostream>
using namespace std;

#include <opencv2/opencv.hpp>
using namespace cv;
 
bool DEBUG_SHOW =0;

void imshow(const String & winname, Mat mat)
{
  if(DEBUG_SHOW)
    cv::imshow (winname, mat);
}

// createTrackbar's UserData Structure
struct tkbar_udata_struct {
  string window_name;
  Mat img;
  tkbar_udata_struct(string winname, Mat im): window_name(winname), img(im) {}
};


// Convert color into gray scale.
// w/o use float point.
void MyColorToGray(const Mat& src, Mat& img) // Gray = R*0.299 + G*0.587 + B*0.114
{
  if (src.channels() == 3) {
    /// Convert the image to grayscale
    #ifdef OCV_CVTCOLOR
      cvtColor( src, img, CV_BGR2GRAY );

    #else
      img.create(src.size(), CV_8UC1);
      for (int y=0; y<img.rows; y++)
        for (int x=0; x<img.cols; x++)
        {
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

void Sort(unsigned char data[9])
{
  int i;
  int j;
  int min_val;
  int index = 0;
  for (i = 0; i < 9; i++)
  {
     min_val = data[i];
     index = i;
     for (j = i+1; j < 9; j++)
     {       
       if (min_val > data[j])
       {       
          min_val = data[j];
          index = j;
       }
     }
     data[index] = data[i];
     data[i] = min_val;
  }
}


// MedianFilting: Remove extreme pixel value (noise) and replace it by medium value of neighbers.
void MedianFilting(const Mat& src, Mat& dst)
{
  int d;
  unsigned char data[9];

  Mat raster = Mat::zeros(src.rows+2, src.cols+2, src.type()); 
  // Copy & Duplicate boundary for image
  for (int y=0; y<raster.rows; y++) 
  {
    for (int x=0; x<raster.cols; x++) 
    {
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

  // Execute medium filtering
  for (int y = 1; y < height - 1; y++)
  {
     for (int x = 1; x < width - 1; x++)
     {
        d = 0;
        for (int m = -1; m <= 1; m++)
        {
           for (int n = -1; n <= 1; n++)
           {
              data[d] = raster.at<uchar>(y+m, x+n);
              d++;
           }
         }
         Sort(data);
         dst.at<uchar>(y-1, x-1) = data[4];
      }
   }
}

 
// Box Filter: all filter element are 1. average all neighber pixels.
void BoxFilting(const Mat& src, Mat& dst)
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
  for (int i = 1; i < height - 1; i++)
  {
     for (int j = 1; j < width - 1; j++)
     {
        average = 0;
        for (int m = -1; m <= 1; m++)
        {
           for (int n = -1; n <= 1; n++)
              average += raster.at<uchar>(i+m, j+n);
         }
         dst.at<uchar>(i-1, j-1) = average/9;
      }
   }
}
 

// OTSU Thresholding is used to automatically perform clustering-based image thresholding.
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


void MySobel (const Mat& img, Mat& grad_x, Mat& grad_y)
{
  // Horizontal filter
  Mat gx = (Mat_<int>(3, 3) << -1, 0, 1, -2, 0, 2, -1, 0, 1);
  // Vertical filter
  Mat gy = (Mat_<int>(3, 3) << -1, -2, -1, 0, 0, 0, 1, 2, 1);

  Mat raster;
  // Duplicate boundary for image
  raster = Mat::zeros(img.rows+2, img.cols+2, img.type()); 
  img.copyTo(raster(Rect(1, 1, img.cols, img.rows)));
  raster.row(1).copyTo(raster.row(0));
  raster.col(1).copyTo(raster.col(0));
  raster.col(raster.cols-2).copyTo(raster.col(raster.cols-1));
  raster.row(raster.rows-2).copyTo(raster.row(raster.rows-1));

  // Calculate Gx/Gy gradient
  grad_x = Mat::zeros(img.rows, img.cols, CV_16S);
  grad_y = Mat::zeros(img.rows, img.cols, CV_16S);
  for (int y=1; y<raster.rows-1; y++) 
  {
    for (int x=1; x<raster.cols-1; x++) 
    {
      for (int m=-1; m<=1; m++) 
      {
        for (int n=-1; n<=1; n++)
        {
          grad_x.at<short>(y-1, x-1) += raster.at<uchar>(y+m, x+n) * gx.at<int>(1+m, 1+n);
          grad_y.at<short>(y-1, x-1) += raster.at<uchar>(y+m, x+n) * gy.at<int>(1+m, 1+n);
        }
      }
    }
  }
}


/*
 * @function MyCanny
 * 1. Get Gradient's magnitude
 * 2. Non-Maximum Suppression
 * Note: Full MyCanny function shold include hystersis threshold. As below:
         void MyCanny(const Mat& src, Mat& dst, int lo_threshold, int hi_threshold)
 */
void MyCanny(const Mat& src, Mat& dst)
{
  Mat grad_x, grad_y; // CV_16S

  #ifdef OCV_SOBEL
    Sobel(src, grad_x, CV_16S, 1, 0, 3, 1, 0, BORDER_DEFAULT);
    Sobel(src, grad_y, CV_16S, 0, 1, 3, 1, 0, BORDER_DEFAULT);
  #else
    MySobel(src, grad_x, grad_y);
  #endif

  // Gradient's magnitude
  Mat grad_mag; // CV_8U
  Mat abs_grad_x, abs_grad_y;
  #if L2GRADIENT == false
    convertScaleAbs(grad_x, abs_grad_x); // convert to CV_8U
    convertScaleAbs(grad_y, abs_grad_y); // convert to CV_8U
    addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad_mag);
  #else
    pow(grad_x, 2, abs_grad_x);
    pow(grad_y, 2, abs_grad_y);
    Mat sum = abs_grad_x + abs_grad_y;
    sum.convertTo(grad_mag, CV_32F);
    sqrt(grad_mag, grad_mag);
    convertScaleAbs(grad_mag, grad_mag);
  #endif
  imshow("Sobel gradient magnitude", grad_mag);

  // Non-Maximum Suppression
  short gx, gy;
  int g1, g2, g3, g4;
  double dTemp, dTemp1, dTemp2;
  double weight;
  dst = Mat::zeros(grad_mag.rows, grad_mag.cols, grad_mag.type()); // all 0s for set all boundary are not edge
  for (int y=1; y<grad_mag.rows-1; y++) 
  {
    for (int x=1; x<grad_mag.cols-1; x++) 
    {
      // the gradient of current point
      gx = grad_x.at<short>(y, x);
      gy = grad_y.at<short>(y, x);
      dTemp = grad_mag.at<uchar>(y, x);

      if (dTemp == 0) // if gradient==0, then it is not the edge point
      {
        dst.at<uchar>(y, x) = 0;
      }
      else // else check gradient direction
      {
        if (abs(gy) > abs(gx))
        {
          weight = fabs(gx) / fabs(gy);
          g2 = grad_mag.at<uchar>(y-1, x);
          g4 = grad_mag.at<uchar>(y+1, x);
          if(gx*gy > 0)
          {
            //g1 g2
            //   C
            //   g4 g3
            g1 = grad_mag.at<uchar>(y-1, x-1);
            g3 = grad_mag.at<uchar>(y+1, x+1);
          }
          else //  if(gx*gy < 0)
          {
            //    g2 g1
            //    C
            // g3 g4
            g1 = grad_mag.at<uchar>(y-1, x+1);
            g3 = grad_mag.at<uchar>(y+1, x-1);
          }
        }
        else // if (abs(gy) <= abs(gx))
        {
          weight = fabs(gy) / fabs(gx);
          g2 = grad_mag.at<uchar>(y, x-1);
          g4 = grad_mag.at<uchar>(y, x+1);
          if(gx*gy > 0)
          {
            // g1
            // g2 C g4
            //      g3
            g1 = grad_mag.at<uchar>(y-1, x-1);
            g3 = grad_mag.at<uchar>(y+1, x+1);
          }
          else // if(gx*gy < 0)
          {
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
      if(dTemp>=dTemp1 && dTemp>=dTemp2)
      {
        dst.at<uchar>(y, x) = grad_mag.at<uchar>(y, x);
      }
      else
      {
        dst.at<uchar>(y, x) = 0;
      }			
    }
  }
}


/*
 * @function CannyThreshold
 * @brief Trackbar callback - Canny thresholds input with a ratio 1:3
 */
void CannyThreshold(int lo_bar_val, int hi_bar_val, void* userdata)
{
  tkbar_udata_struct tkbar_udata = *(tkbar_udata_struct*) userdata;

  Mat src = tkbar_udata.img;
  string window_name = tkbar_udata.window_name;

  Mat src_gray;
  Mat detected_edges, dst;
  const int kernel_size = 3;
  // const int ratio = 3;

  /// Convert the image to grayscale
  MyColorToGray(src, src_gray);

  /// Reduce noise with a kernel 3x3
  #ifdef OCV_BLUR
    blur(src_gray, detected_edges, Size(3,3));
  #else
    MedianFilting(src_gray, src_gray); // remove noise
    BoxFilting(src_gray, src_gray); // average
  #endif
  imshow( "Blur of Gray SRC", src_gray);

  /// Canny edge detector
  #ifdef OCV_CANNY
    // Canny(src_gray, detected_edges, lo_bar_val, lo_bar_val*ratio, kernel_size, L2GRADIENT);
    Canny(src_gray, detected_edges, lo_bar_val, hi_bar_val, kernel_size, L2GRADIENT);
  #else
    Mat nmax_suppress;
    MyCanny(src_gray, nmax_suppress);
    imshow("Non-Maximum Suppression", nmax_suppress);

    detected_edges = Mat::zeros(nmax_suppress.size(), nmax_suppress.type()); // all 0s for set all boundary are not edge
    // Hysteresis threshold
    for (int y=1; y<nmax_suppress.rows-1; y++) 
    {
      for (int x=1; x<nmax_suppress.cols-1; x++) 
      {
        if (nmax_suppress.at<uchar>(y, x) >= hi_bar_val)
          detected_edges.at<uchar>(y, x) = 255;
        else if (nmax_suppress.at<uchar>(y, x) < lo_bar_val)
          detected_edges.at<uchar>(y, x) = 0;
        else
          if (nmax_suppress.at<uchar>(y-1, x-1) >= hi_bar_val || nmax_suppress.at<uchar>(y-1, x) >= hi_bar_val || nmax_suppress.at<uchar>(y-1, x+1) >= hi_bar_val ||
              nmax_suppress.at<uchar>(y  , x-1) >= hi_bar_val ||                                                  nmax_suppress.at<uchar>(y  , x+1) >= hi_bar_val ||
              nmax_suppress.at<uchar>(y+1, x-1) >= hi_bar_val || nmax_suppress.at<uchar>(y+1, x) >= hi_bar_val || nmax_suppress.at<uchar>(y+1, x+1) >= hi_bar_val)
            detected_edges.at<uchar>(y, x) = 255;
          else
            detected_edges.at<uchar>(y, x) = 0;
      }
    }
    cv::imshow("Canny_detected_edges", detected_edges);
 
    // To reference with other threshold method
    #ifdef REF_THRESHOLD
      Mat threshold_mat;
      threshold(nmax_suppress, threshold_mat, lo_bar_val, 255, THRESH_BINARY);
      imshow("Compare w/ OpenCV Constant Threshold", threshold_mat);
    
      threshold(nmax_suppress, threshold_mat, lo_bar_val, 255, CV_THRESH_OTSU);
      imshow("Compare w/ OpenCV OTSU Threshold", threshold_mat);

      int block_sz = int(lo_bar_val/255*nmax_suppress.cols/2)*2+3;
      adaptiveThreshold(nmax_suppress, threshold_mat, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, block_sz, 0);
      // adaptiveThreshold(nmax_suppress, threshold_mat, 255, ADAPTIVE_THRESH_GAUSSIAN_C , THRESH_BINARY, block_sz, 0);
      imshow("Compare w/ OpenCV adaptive Threshold", threshold_mat);

      int my_otsu_thval = otsu_threshold (nmax_suppress);
      threshold(nmax_suppress, threshold_mat, my_otsu_thval, 255, THRESH_BINARY);
      imshow("Compare w/ My OTSU Threshold", threshold_mat);

    #endif
  #endif
  /// Create a matrix of the same type and size as src (for dst)
  // dst.create( src.size(), src.type() );
  // dst = Scalar::all(0);
  dst = Mat::zeros(src.size(), src.type());

  /// Using Canny's output as a mask, and display result
  src.copyTo(dst, detected_edges);
  cv::imshow(window_name, dst);
}


// Low threshold track bar
void LoTkBar_Change(int pos, void* userdata)
{
  tkbar_udata_struct tkbar_udata = *(tkbar_udata_struct*) userdata;
  int loThreshold = pos;
  int hiThreshold = getTrackbarPos ("Max Threshold:", tkbar_udata.window_name);
  if (loThreshold > hiThreshold)
    setTrackbarPos ("Max Threshold:", tkbar_udata.window_name, loThreshold);
  // cout << "lo_bar_val is:" << loThreshold << endl;
  // cout << "hi_bar_val is:" << hiThreshold << endl;
  CannyThreshold(loThreshold, hiThreshold, &tkbar_udata);
}

// High threshold track bar
void HiTkBar_Change(int pos, void* userdata)
{
  tkbar_udata_struct tkbar_udata = *(tkbar_udata_struct*) userdata;
  int hiThreshold = pos;
  int loThreshold = getTrackbarPos ("Min Threshold:", tkbar_udata.window_name);
  if (loThreshold > hiThreshold)
    setTrackbarPos ("Min Threshold:", tkbar_udata.window_name, hiThreshold);
  // cout << "lo_bar_val is:" << loThreshold << endl;
  // cout << "hi_bar_val is:" << hiThreshold << endl;
  CannyThreshold(loThreshold, hiThreshold, &tkbar_udata);
}



/** @function main */

const String keys =
    "{h help usage ? |      | print this message    }"
    "{@image_file    |      | image file for process}"
    "{d debug show   |      | show image for debug  }"
    ;

int main( int argc, char** argv )
{
  // Parse command line 
  #ifdef OCV_CMDLINE
    CommandLineParser parser(argc, argv, keys);
    parser.about("Using Trackbar to adjust Canny edge detection.");

    if (parser.has("?"))
    {
       parser.printMessage();
       return 0;
    }
    String filename = parser.get<String>(0);
    DEBUG_SHOW = parser.has("debug");

  #else
    if (argc < 2 || argc > 3) {
      cout << "Using Trackbar to adjust Canny edge detection:" << endl;
      cout << argv[0] << " <image_file> [0|1: for show debug image]" << endl;
      return -1;
    }
    else if (argc == 3)
    {
      DEBUG_SHOW = (bool)atoi(argv[2]);
    }
    string filename = argv[1];
  #endif

  // Load an image
  Mat src = imread(filename, IMREAD_UNCHANGED);
  if ( !src.data ) {
    cout << "Fail to open file: " << argv[1] << endl << endl;
    cout << "Using Trackbar to adjust Canny edge detection:" << endl;
    cout << argv[0] << " <image_file> [0|1: for show debug image]" << endl;
    return -1;
  }
  cv::imshow( "SRC img", src);
  cout << "DEBUG_SHOW= " << DEBUG_SHOW << endl;

  int loThreshold = 30;
  int hiThreshold = 90;
  int const max_Threshold = 255;
  tkbar_udata_struct tkbar_udata("Edge Map", src);

  // Create a image window
  namedWindow( tkbar_udata.window_name, CV_WINDOW_AUTOSIZE );
  // Create a Trackbar for user to adjust high/low threshold
  createTrackbar( "Low Threshold:",  tkbar_udata.window_name, &loThreshold, max_Threshold, LoTkBar_Change, &tkbar_udata);
  createTrackbar( "High Threshold:", tkbar_udata.window_name, &hiThreshold, max_Threshold, HiTkBar_Change, &tkbar_udata);
  // Initial callback function
  CannyThreshold(loThreshold, hiThreshold, &tkbar_udata);

  // Wait until user exit program by pressing a key
  waitKey(0);
  return 0;
}
