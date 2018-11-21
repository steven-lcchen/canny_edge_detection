cmake_minimum_required (VERSION 2.6)

cmake_policy(SET CMP0012 NEW)

# Requires OpenCV
FIND_PACKAGE( OpenCV 3.0.0 REQUIRED )
MESSAGE("OpenCV version : ${OpenCV_VERSION}")

include_directories(${OpenCV_INCLUDE_DIRS})
link_directories(${OpenCV_LIB_DIR})


PROJECT(test_canny)
ADD_EXECUTABLE( ${PROJECT_NAME} test_canny.cpp MyColorToGray.cpp MedianFilter.cpp BoxFilter.cpp MyCanny.cpp LabelConnected.cpp otsu_threshold.cpp)
TARGET_LINK_LIBRARIES( ${PROJECT_NAME} ${OpenCV_LIBS} )

PROJECT(test_CmdLineParser)
ADD_EXECUTABLE( ${PROJECT_NAME} test_CmdLineParser.cpp)
TARGET_LINK_LIBRARIES( ${PROJECT_NAME} ${OpenCV_LIBS} )

PROJECT(test_threshold)
ADD_EXECUTABLE( ${PROJECT_NAME} test_threshold.cpp otsu_threshold.cpp LabelConnected.cpp)
TARGET_LINK_LIBRARIES( ${PROJECT_NAME} ${OpenCV_LIBS} )
