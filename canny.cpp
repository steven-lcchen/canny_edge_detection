/*
  Topic: Canny Edge Detection
    Showing how to detect edges using the Canny Algorithm

  Date: 107/10/15
  Author: Steven Chen
*/

// #define OCV_CVTCOLOR 1
// #define OCV_BLUR 1
// #define OCV_CANNY 1
// #define OCV_SOBEL 1
#define L2GRADIENT true // For Canny L2gradient precision: true | false
 
#include <iostream>
using namespace std;

#include <opencv2/opencv.hpp>
using namespace cv;
 
// createTrackbar's UserData Structure
struct tkbar_udata_struct {
  string window_name;
  Mat img;
  tkbar_udata_struct(string winname, Mat im): window_name(winname), img(im) {}
};

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
          img.at<uchar>(y, x) = R*0.299 + G*0.587 + B*0.114;
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

void MedianFilting(Mat& img)
{
  int d;
  unsigned char data[9];

  int width = img.cols;
  int height = img.rows;

  for (int i = 1; i < height - 1; i++)
  {
     for (int j = 1; j < width - 1; j++)
     {
        d = 0;
        for (int m = -1; m <= 1; m++)
        {
           for (int n = -1; n <= 1; n++)
           {
              data[d] = img.at<uchar>(i+n, j+m);
              d++;
           }
         }
         Sort(data);
         img.at<uchar>(i, j) = data[4];
      }
   }
}
 
void BoxFilting(Mat& img)
{
  unsigned int box;

  int width = img.cols;
  int height = img.rows;

  for (int i = 1; i < height - 1; i++)
  {
     for (int j = 1; j < width - 1; j++)
     {
        box = 0;
        for (int m = -1; m <= 1; m++)
        {
           for (int n = -1; n <= 1; n++)
              box += img.at<uchar>(i+n, j+m);
         }
         img.at<uchar>(i, j) = box/9;
      }
   }
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
  //// expand boundary
  // for (int y=0; y<raster.rows; y++) 
  // {
  //   for (int x=0; x<raster.cols; x++) 
  //   {
  //     if (y==0)
  //       if (x==0)
  //         raster.at<uchar>(y, x) = img.at<uchar>(y, x);
  //       else if (x==raster.cols-1)
  //         raster.at<uchar>(y, x) = img.at<uchar>(y, x-2);
  //       else
  //         raster.at<uchar>(y, x) = img.at<uchar>(y, x-1);
  //     else if (y==raster.rows-1) 
  //       if (x==0)
  //         raster.at<uchar>(y, x) = img.at<uchar>(y-2, x);
  //       else if (x==raster.cols-1)
  //         raster.at<uchar>(y, x) = img.at<uchar>(y-2, x-2);
  //       else
  //         raster.at<uchar>(y, x) = img.at<uchar>(y-2, x-1);
  //     else
  //       if (x==0)
  //         raster.at<uchar>(y, x) = img.at<uchar>(y-1, x);
  //       else if (x==raster.cols-1)
  //         raster.at<uchar>(y, x) = img.at<uchar>(y-1, x-2);
  //       else
  //         raster.at<uchar>(y, x) = img.at<uchar>(y-1, x-1);
  //   }
  // }
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

void MyCanny (const Mat& src, Mat& dst, double lo_threshold, double hi_threshold)
{
  Mat grad_x, grad_y; // CV_16S
  const double PI = 3.1415926535897932384626433832795;

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
  imshow("Non-Maximum Suppression", dst);
 
  // Hysteresis threshold
  for (int y=1; y<dst.rows-1; y++) 
  {
    for (int x=1; x<dst.cols-1; x++) 
    {
      if (dst.at<uchar>(y, x) >= hi_threshold)
        dst.at<uchar>(y, x) = 255;
      else if (dst.at<uchar>(y, x) < lo_threshold)
        dst.at<uchar>(y, x) = 0;
      else
      {
        if (dst.at<uchar>(y-1, x-1) >= hi_threshold || dst.at<uchar>(y-1, x) >= hi_threshold || dst.at<uchar>(y-1, x+1) >= hi_threshold ||
            dst.at<uchar>(y  , x-1) >= hi_threshold ||                                          dst.at<uchar>(y  , x+1) >= hi_threshold ||
            dst.at<uchar>(y+1, x-1) >= hi_threshold || dst.at<uchar>(y+1, x) >= hi_threshold || dst.at<uchar>(y+1, x+1) >= hi_threshold)
          dst.at<uchar>(y, x) = 255;
        else
          dst.at<uchar>(y, x) = 0;
      }
    }
  }
}

/**
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
  const int ratio = 3;
  // if (hi_bar_val > lo_bar_val*ratio)
  //   if (lo_bar_val*ratio > 255)
  //     hi_bar_val = 255;
  //   else
  //     hi_bar_val = lo_bar_val*ratio;

  /// Convert the image to grayscale
  MyColorToGray(src, src_gray);

  /// Reduce noise with a kernel 3x3
  #ifdef OCV_BLUR
    blur( src_gray, detected_edges, Size(3,3) );
  #else
    detected_edges = src_gray.clone();
    MedianFilting(detected_edges); // remove noise
    BoxFilting(detected_edges); // average
  #endif
  imshow( "Blur of Gray SRC", detected_edges);

  /// Canny detector
  #ifdef OCV_CANNY
    Canny( detected_edges, detected_edges, lo_bar_val, hi_bar_val, kernel_size, L2GRADIENT);
  #else
    MyCanny(detected_edges, detected_edges, lo_bar_val, hi_bar_val);
  #endif

  /// Create a matrix of the same type and size as src (for dst)
  dst.create( src.size(), src.type() );

  /// Using Canny's output as a mask, we display our result
  dst = Scalar::all(0);

  src.copyTo( dst, detected_edges);
  imshow("Canny_detected_edges", detected_edges);
  imshow( window_name, dst );
}

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
  imshow( "SRC img", src);

  tkbar_udata_struct tkbar_udata("Edge Map", src);

  /// Create a window
  namedWindow( tkbar_udata.window_name, CV_WINDOW_AUTOSIZE );

  /// Create a Trackbar for user to enter threshold
  createTrackbar( "Min Threshold:", tkbar_udata.window_name, &loThreshold, max_Threshold, LoTkBar_Change, &tkbar_udata);
  createTrackbar( "Max Threshold:", tkbar_udata.window_name, &hiThreshold, max_Threshold, HiTkBar_Change, &tkbar_udata);
  /// Show the image
  CannyThreshold(loThreshold, hiThreshold, &tkbar_udata);

  /// Wait until user exit program by pressing a key
  waitKey(0);

  return 0;
}
