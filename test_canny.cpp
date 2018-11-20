/*
  Topic: Canny Edge Detection
    Showing how to detect edges using the Canny Algorithm

  Date: 107/10/15
  Author: Steven Chen
*/

#include "define.hpp"

#include <iostream>
using namespace std;

#include <opencv2/opencv.hpp>
using namespace cv;
 
bool DEBUG_SHOW = false;

const string hi_tkbar_name = "High Threshold:";
const string lo_tkbar_name = "Low Threshold:";

void MyColorToGray(const Mat& src, Mat& img); // Gray = R*0.299 + G*0.587 + B*0.114
void MedianFilter(const Mat& src, Mat& dst);
void BoxFilter(const Mat& src, Mat& dst);
void MyCanny(const Mat& src, Mat& detected_edges, int lo_threshold, int hi_threshold, bool L2gradient=true, bool debug=false);
int  LabelConnected(const Mat& img, Mat& label, uint connectivity=8);

// createTrackbar's UserData Structure
struct tkbar_udata_struct {
  string window_name;
  Mat img;
  bool L2gradient; // For Canny L2gradient precision: true | false
  uint connectivity; // for neighbering connectivity, 4 or 8 only
  tkbar_udata_struct(string winname, Mat im, bool gradient, uint conn) :
    window_name(winname), img(im), L2gradient(gradient), connectivity(conn) {}
};


void dbg_imshow(const String & winname, Mat mat) {
  if(DEBUG_SHOW)
    imshow (winname, mat);
}

static Scalar randomColor( RNG& rnd_num )
{
int icolor = (unsigned) rnd_num;
return Scalar( icolor&255, (icolor>>8)&255, (icolor>>16)&255 );
}

/*
 * @function Adj_CannyThreshold
 * @brief Trackbar callback - Canny thresholds input
 */
void Adj_CannyThreshold(int lo_bar_val, int hi_bar_val, void* userdata)
{
  tkbar_udata_struct tkbar_udata = *(tkbar_udata_struct*) userdata;

  string& window_name = tkbar_udata.window_name;
  Mat& src = tkbar_udata.img;
  uint connectivity = tkbar_udata.connectivity;
  bool L2gradient = tkbar_udata.L2gradient;

  // Convert the image to grayscale
  Mat src_gray(src.size(), CV_8UC1);
  MyColorToGray(src, src_gray);
  dbg_imshow("2: Convert to Gray", src_gray);

  /// Reduce noise with a kernel 3x3
  #ifdef OCV_BLUR
    blur(src_gray, src_gray, Size(3,3));
    dbg_imshow("3: Apply OCV blur", src_gray);
  #else
    MedianFilter(src_gray, src_gray); // remove noise
    dbg_imshow("3.1: Apply MedianFilter", src_gray);

    BoxFilter(src_gray, src_gray); // average
    dbg_imshow("3.2: Apply BoxFilter", src_gray);
  #endif

  // Canny edge detector
  Mat detected_edges = Mat::zeros(src_gray.size(), src_gray.type()); // all 0s for set all boundary are not edge
  #ifdef OCV_CANNY
    const int kernel_size = 3;
    Canny(src_gray, detected_edges, lo_bar_val, hi_bar_val, kernel_size, L2gradient);
  #else
    MyCanny(src_gray, detected_edges, lo_bar_val, hi_bar_val, L2gradient, DEBUG_SHOW);
  #endif
  dbg_imshow("4: OCV Canny detected_edges", detected_edges);

  // Using Canny's output as a mask, and display result
  Mat dst(src.size(), src.type(), Scalar::all(0));
  src.copyTo(dst, detected_edges);
  imshow(window_name, dst);

  // find connected components
  Mat labels(src_gray.size(), CV_16UC1, Scalar(0));
  #ifdef OCV_LABCONN
    int num_objects= connectedComponents(detected_edges, labels, connectivity, CV_16U);
  #else
    int num_objects = LabelConnected(detected_edges, labels, connectivity);
  #endif
  cout << "num_objects = " << num_objects << endl;

  // Create output image coloring the objects
  Mat output= Mat::zeros(src.rows, src.cols, CV_8UC3);
  RNG rnd_num( cvGetTickCount() ); // Random seed
  for(int i=1; i<num_objects; i++){
    Mat mask= labels==i;
    output.setTo(randomColor(rnd_num), mask);
    // imshow("mask"+to_string(i), output); // Steven
    // cout<<"Size of object["<<i<<"]="<<countNonZero(mask)<<endl;
  }
  imshow("Result", output);
}


// Low threshold track bar
void LoTkBar_Change(int pos, void* userdata)
{
  tkbar_udata_struct tkbar_udata = *(tkbar_udata_struct*) userdata;
  int loThreshold = pos;
  int hiThreshold = getTrackbarPos (hi_tkbar_name, tkbar_udata.window_name);
  if (loThreshold > hiThreshold) {
    hiThreshold = loThreshold;
    setTrackbarPos (hi_tkbar_name, tkbar_udata.window_name, hiThreshold);
  }
  // cout << "lo_bar_val is:" << loThreshold << endl;
  // cout << "hi_bar_val is:" << hiThreshold << endl;
  Adj_CannyThreshold(loThreshold, hiThreshold, &tkbar_udata);
}

// High threshold track bar
void HiTkBar_Change(int pos, void* userdata)
{
  tkbar_udata_struct tkbar_udata = *(tkbar_udata_struct*) userdata;
  int hiThreshold = pos;
  if (hiThreshold == 0) {
    hiThreshold = 1;
    setTrackbarPos (hi_tkbar_name, tkbar_udata.window_name, hiThreshold);
  }
  int loThreshold = getTrackbarPos (lo_tkbar_name, tkbar_udata.window_name);
  if (hiThreshold < loThreshold) {
    loThreshold = hiThreshold;
    setTrackbarPos (lo_tkbar_name, tkbar_udata.window_name, loThreshold);
  }
  // cout << "lo_bar_val is:" << loThreshold << endl;
  // cout << "hi_bar_val is:" << hiThreshold << endl;
  Adj_CannyThreshold(loThreshold, hiThreshold, &tkbar_udata);
}


const String cmd_help =
  "{h help usage ? |   | print this message    }"
  "{@image_file    |   | image file for process}"
  "{c connectivity | 8 | connectivity=4 or 8 only}"
  "{l l2gradient   |   | L2gradient=true or false}"
  "{d debug show   |   | show some images for debug}"
  ;

/** @function main */
int main( int argc, char** argv )
{
  // Parse command line 
  if (argc < 2 || argc > 4) {
    cout << "Using Trackbar to adjust threshold for testing Canny algorithm & labeling connected components:" << endl;
    cout << argv[0] << " <image_file> [-c=4|8] [-l=0: [-d: for show debug image]" << endl;
    cout << "More information ... -[h help usage ?]" << endl;
    return -1;
  }

  CommandLineParser parser(argc, argv, cmd_help);
  parser.about("Using Trackbar to adjust threshold for test Canny edge detection algorithm.");

  if (parser.has("?"))
  {
     parser.printMessage();
     return 0;
  }
  String filename = parser.get<String>(0);
  uint connectivity = parser.get<int>("c");
  cout << "connectivity= " << connectivity << endl;
  bool L2gradient = parser.has("l");
  cout << "L2gradient= " << L2gradient << endl;
  DEBUG_SHOW = parser.has("debug");
  cout << "DEBUG_SHOW= " << DEBUG_SHOW << endl;
  if (!DEBUG_SHOW) cout << "add option [-d] to show debug image" << endl;

  // Load an image
  Mat src = imread(filename, IMREAD_UNCHANGED);
  if ( !src.data ) {
    cout << "Fail to open file: " << argv[1] << endl << endl;
    cout << "Using Trackbar to adjust Canny edge detection:" << endl;
    cout << argv[0] << " <image_file> [0|1: for show debug image]" << endl;
    return -1;
  }
  imshow( "1: Source Image", src);

  int loThreshold = 30;
  int hiThreshold = 90;
  int const max_Threshold = 255;
  // initial trackbar udata
  tkbar_udata_struct tkbar_udata("Canny_detected_edges", src, L2gradient, connectivity);

  // Create a image window
  namedWindow(tkbar_udata.window_name, CV_WINDOW_AUTOSIZE);

  // Create a Trackbar for user to adjust high/low threshold
  createTrackbar( lo_tkbar_name, tkbar_udata.window_name, &loThreshold, max_Threshold, LoTkBar_Change, &tkbar_udata);
  createTrackbar( hi_tkbar_name, tkbar_udata.window_name, &hiThreshold, max_Threshold, HiTkBar_Change, &tkbar_udata);

  // Initial callback function
  Adj_CannyThreshold(loThreshold, hiThreshold, &tkbar_udata);

  // Wait until user exit program by pressing a key
  cout << "Press any key to continue ..." << endl;
  waitKey(0);
  destroyAllWindows();
  return 0;
}
