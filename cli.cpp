#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
 
#include <iostream>

using namespace cv;
using namespace std;
 
const String keys =
    "{h help usage ? |      | print this message   }"
    "{@image1        |      | image1 for compare   }"
    "{@image2        |      | image2 for compare   }"
    "{@repeat        |1     | number               }"
    "{path           |.     | path to file         }"
    "{fps            | -1.0 | fps for output video }"
    "{N count        |100   | count of objects     }"
    "{ts timestamp   |      | use time stamp       }"
    ;
 
 
int main(int argc, const char** argv )
{
  CommandLineParser parser(argc, argv, keys);
  parser.about("Application name v1.0.0");
  
  if (parser.has("?"))
  {
      cout << "*** Begin CLI test." << endl;
      parser.printMessage();
      cout << "*** End CLI test." << endl;
      return 0;
  }
  
  int N = parser.get<int>("N");
  double fps = parser.get<double>("fps");
  String path = parser.get<String>("path");
  
  bool use_time_stamp = parser.has("timestamp");
  
  String img1 = parser.get<String>(0);
  String img2 = parser.get<String>(1);
  
  int repeat = parser.get<int>(2);
  
  if (!parser.check())
  {
     cout << "*** Arguments Error." << endl;
     parser.printErrors();
     return 0;
  }
  if (parser.has("count")) cout << "N = " << N << endl;
  if (parser.has("fps")) cout << "fps = " << fps << endl;
  if (parser.has("path")) cout << "path = " << path << endl;
  if (parser.has("@image1")) cout << "image1 = " << img1 << endl;
  if (parser.has("@image2")) cout << "image2 = " << img2 << endl;
  if (parser.has("@repeat")) cout << "number = " << repeat << endl;
  cout << "timestamp = " << use_time_stamp << endl;
  cout << "path to the executable = " << parser.getPathToApplication() << endl;

  for (int i=0; i<argc; i++)
    cout << argv[i] << " ";
  cout << endl;

  cout << "Hello command line." << endl;
}
