# code: test_CmdLineParser.cpp  
a openCV command line parser example
https://stevenchen886.blogspot.com/2018/09/opencv-example-of-commandlineparser.html  

# code: otsu_threshold.cpp  
a reference code about OTSU's method  
It is used to automatically perform clustering-based image thresholding, or, the reduction of a graylevel image to a binary image.  

# code: test_threshold.cpp 
a test code about without using global variable when call creatTtrackbar and do image thresholding with different method.  
$ ./compile.sh -o test_threshold test_threshold.cpp otsu_threshold.cpp  

# canny_edge_detection
Implement Canny edge detection algorithm with C++ and practice with 2 Trackbar to adjust hysteresis threshold.  
And then labeling connected components
code: canny.cpp  
$ ./compile.sh -o test_canny test_canny.cpp MyColorToGray.cpp MedianFilter.cpp BoxFilter.cpp MyCanny.cpp LabelConnected.cpp
$ test_canny image_file  

