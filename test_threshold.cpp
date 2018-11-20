// Image threshold test ---
// OpenCV's Binary image: It may use track bar to adjust threshold level
// OpenCV's OTSU image: Using OpenCV's OTSU threshold function
// My OTSU image: According to my code of OTSU threshold
// OCV Binary image with My OTSU value: OpenCV's Binary Threshold image with my OTSU threshold value
// By Steven Chen
// 2018/11/12

#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int otsu_threshold (const Mat& src, Mat& dst, int typ=0);

// pack data into struct for track bar callback function
struct FkOpenCV {
    string winname;
    Mat im;
    FkOpenCV(string winname_, Mat im_): winname(winname_), im(im_){}
};

// Call Back function
void on_threshold(int bar_val, void* userdata)
{
  // cout << "bar_val is:" << bar_val << endl;
  FkOpenCV fk = *(FkOpenCV*) userdata;

  Mat dst = Mat::zeros(fk.im.size(), CV_8U);
  threshold(fk.im, dst, bar_val, 255, THRESH_BINARY);
  imshow(fk.winname, dst);

  // Reference to other threshold method
  Mat threshold_mat = Mat::zeros(fk.im.size(), fk.im.type());
  int block_sz = int(bar_val/255*fk.im.cols/2)*2+3;
  adaptiveThreshold(fk.im, threshold_mat, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, block_sz, 0);
  // adaptiveThreshold(fk.im, threshold_mat, 255, ADAPTIVE_THRESH_GAUSSIAN_C , THRESH_BINARY, block_sz, 0);
  imshow("Gray to OCV adaptive Threshold", threshold_mat);

  // Test OpenCV's OTSU threshold
  Mat ocv_otsu_mat = cv::Mat::zeros(fk.im.size(), CV_8U);
  threshold(fk.im, ocv_otsu_mat, 128, 255, CV_THRESH_OTSU);
  imshow( "OCV OTSU image", ocv_otsu_mat);

  // Test My OTSU threshold function
  Mat my_otsu_mat = cv::Mat::zeros(fk.im.size(), CV_8U);
  int my_otsu_val = otsu_threshold (fk.im, my_otsu_mat);
  imshow( "My OTSU image", my_otsu_mat);
  stringstream ss;
  ss << "My OTSU Value: " << my_otsu_val;
  displayOverlay("My OTSU image", ss.str(), 5000);

  // Test OpenCV binary threshold with My OTSU value
  Mat my_otsuthres_mat = cv::Mat::zeros(fk.im.size(), CV_8U);
  threshold(fk.im, my_otsuthres_mat, my_otsu_val, 255, THRESH_BINARY);
  imshow( "OCV Threshold with My OTSU value", my_otsuthres_mat);
  displayStatusBar("OCV Threshold with My OTSU value", ss.str(), 5000);

  bool not_equ = false;
  for (int i=0; i< fk.im.rows; i++) {
    for (int j=0; j<fk.im.cols; j++) {
      if (ocv_otsu_mat.at<uchar>(i, j) != my_otsu_mat.at<uchar>(i, j)) {
        not_equ = true;
        break;
      }
    }
    if (not_equ==true) break;
  }
  if (not_equ)
    cout << "ocv_otsu_mat != my_otsu_mat" << endl;
  else
    cout << "ocv_otsu_mat == my_otsu_mat" << endl;

  not_equ = false;
  for (int i=0; i< fk.im.rows; i++) {
    for (int j=0; j<fk.im.cols; j++) {
      if (ocv_otsu_mat.at<uchar>(i, j) != my_otsuthres_mat.at<uchar>(i, j)) {
        not_equ = true;
        break;
      }
    }
    if (not_equ==true) break;
  }
  if (not_equ)
    cout << "ocv_otsu_mat != my_otsuthres_mat" << endl;
  else
    cout << "ocv_otsu_mat == my_otsuthres_mat" << endl;

}

// main program
int main(int argc, const char** argv)
{
  if (argc != 2) {
    cout << "Image threshold value test --- " << endl;
    cout << "Using Trackbar to adjust threshold level :" << endl;
    cout << argv[0] << " image_file" << endl;
    return -1;
  }

  // Load source image
  Mat src = imread(argv[1], IMREAD_UNCHANGED);
  if( !src.data ) {
    cout << "Fail to open file: " << argv[1] << endl << endl;
    return -1;
  }
  namedWindow("Source Image");
  imshow("Source Image", src);

  // Convert color into gray scale
  Mat im_gray;
  if (src.channels() == 3)
    cvtColor( src, im_gray, CV_BGR2GRAY );
  else
   im_gray = src.clone();

  // Using Trackbar to adjust threshold level  
  string winname = "OCV Binary image";
  namedWindow(winname);
  int value = 100;
  FkOpenCV fk(winname, im_gray);
  createTrackbar("Threshold", winname, &value, 255, on_threshold, &fk); // Cannot use Chinese as bar name
  on_threshold(value, &fk);

  waitKey();
  destroyAllWindows();

  return 0;
}
