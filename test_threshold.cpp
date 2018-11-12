#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

// 自定义结构体类型，将数据打包封装到结构体里
struct FkOpenCV {
    string winname;
    Mat im;
    FkOpenCV(string winname_, Mat im_): winname(winname_), im(im_){}
};

// 回调函数
void on_threshold(int bar_val, void* userdata) {
    // cout << "bar_val is:" << bar_val << endl; //bar_val就是createTrackbar调用时的value参数
    FkOpenCV fk = *(FkOpenCV*) userdata;

    Mat dst;
    threshold(fk.im, dst, bar_val, 255, 0);
    imshow(fk.winname, dst);
}

// demo代码
void thresh_seg_demo(Mat &image) {
  Mat im_gray;
  cvtColor(image, im_gray, CV_BGR2GRAY);

  string winname = "Threshold";
  namedWindow(winname);
  int value = 100;
  FkOpenCV fk(winname, im_gray);
  createTrackbar("Threshold", winname, &value, 255, on_threshold, &fk); // Cannot use Chinese as bar name
  on_threshold(value, &fk);
  waitKey();
  destroyAllWindows();
}

// 主函数
int main(int argc, const char** argv) {
    
  if (argc != 2) {
    cout << "Using Trackbar to adjust threshold level :" << endl;
    cout << argv[0] << " image_file" << endl;
    return -1;
  }
  /// Load an image
  Mat src = imread(argv[1], IMREAD_UNCHANGED);
  if( !src.data ) {
    cout << "Fail to open file: " << argv[1] << endl << endl;
    return -1;
  }
  namedWindow("原图");
  imshow("原图", src);

  thresh_seg_demo(src);
  return 0;
}
