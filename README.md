# canny_edge_detection
Implement Canny edge detection algorithm with C++  
practice with 2 Trackbar adjustment for threshold.  
code: canny.cpp  
$ ./compile.sh -o canny canny.cpp  
$ canny image_file  

# a reference code about without using global variable when call creatTtrackbar
code: threshold.cpp 
$ ./compile.sh -o threshold threshold.cpp  

# a reference code about OTSU's method
It is used to automatically perform clustering-based image thresholding,[1] or, the reduction of a graylevel image to a binary image.  
code: otsu_threshold.cpp  
$ ./compile.sh -o otsu_threshold otsu_threshold.cpp  

# a openCV command line parser example
code: cli.cpp  
https://stevenchen886.blogspot.com/2018/09/opencv-example-of-commandlineparser.html
