/* 
 * File: label_img.cpp
 * Purpose: 
 *      - Implements the Union-Find labeling algorithm in serial.
 *      - Contains a Union-Find datastructure to resolve equivalences between labels.
 */

#include <algorithm>
#include <vector>
#include <iostream>
#include <stdlib.h>
#include <utility>
#include <map>
// #include "imghelpers.hpp"
// #include "directions.hpp"

/* 
 * File: directions.h
 * Author: Sammy EL GHAZZAL
 * Date: Started 05/07/2013
 * Purpose: Implements the inline functions to get neighbors in an image vector. 
 */


// This file contains inline function to access elements in the
// neighborhood of a particular element. Returns ERROR_CODE
// if the neighboor requested is not reachable (the element is 
// on the boundary).

#include <climits>

typedef unsigned int uint;

static const uint ERROR_CODE = UINT_MAX - 1;

inline uint GetN(uint curr, uint width) {
    if (curr / width == 0) {
        return ERROR_CODE;
    }   
    return curr - width;
}

inline uint GetW(uint curr, uint width) {
    if (curr % width == 0) {
        return ERROR_CODE;
    }
    return curr - 1;
}

inline uint GetNW(uint curr, uint width) {
    if (curr / width == 0 || curr % width == 0) {
        return ERROR_CODE;
    }
    return curr - width - 1;
}

inline uint GetNE(uint curr, uint width) {
    if (curr / width == 0 || (curr + 1) % width == 0) {
        return ERROR_CODE;
    }
    return curr - width + 1;
}


// -----------------------------------------------------------------
using namespace std;

#include <opencv2/opencv.hpp>
using namespace cv;

const static uint OUT_COLOR = UINT_MAX - 1;


// helps model a tree like structure where the parent node is the 
// representative of an equivalence class 
struct node { 
    node *parent;
    uint label;
    node(uint label) { 
        this->label = label;
        this->parent = this;
    }   
};

// get the root node (the representative) associated to a node 
node *FindRep(node *n) { 
    while (n -> parent != n) { 
        n = n -> parent;
    }   
    return n;
}

void Union(node *x, node *y) { 
    node *xRep = FindRep(x);
    node *yRep = FindRep(y);
    yRep -> parent = xRep;
}

// create a link between two labels 
void Union(uint lab1, uint lab2, map<uint, node *>& labToNode) { 
    node *x = labToNode[lab1];
    node *y = labToNode[lab2];
    Union(x, y); 
}


// create new label (and add it to the map)
void AddLabel(uint& label, map<uint, node *>& labToNode) { 
    node *toAdd = new node(label);
    labToNode[label] = toAdd;
    ++label;
}


// Second pass of the Union-Find labeling algorithm. Resolves 
// the equivalences of labels. 
// Modifies res by reference to avoid allocating extra memory. 
void SecondPass(map<uint, node *>& labToNode, Mat& res, uint xLen, uint yLen, uint maxLabel) { 
    // decide on the final labels
    map<uint, uint> equivClass;
    for (int i = 0; i < (int)maxLabel; ++i) { 
        equivClass[i] = FindRep(labToNode[i]) -> label;
    }
    // second pass
    for (int i = 0; i < (int)yLen; ++i) { 
        for (int j = 0; j < (int)xLen; ++j) { 
            res.data[i * xLen + j] = equivClass[res.data[i * xLen + j]];
        }
    }
}


// First pass of the Union-Find labeling algorithm. Attributes labels
// to zones in a forward manner (i.e. by looking only at a subset of the
// neighbors). 
template<uint connectivity> void FirstPass(const Mat& img, Mat& res, map<uint, node *>& labToNode, uint &currLabel) {
  uint left, up, leftup, rightup, curr, currPos, leftC, upC, leftupC, rightupC;
  for (int i = 0; i < (int)img.rows; i++) { 
    for (int j = 0; j < (int)img.cols; j++) { 
      if (i == 0 && j == 0) {
        if(img.at<uchar>(i, j))
          res.at<uchar>(i, j) = currLabel;
      };
      curr = img.at<uchar>(i, j);

      currPos = i * img.cols + j;
      left = GetW(currPos, img.cols);
      up = GetN(currPos, img.cols);
      leftC = left == ERROR_CODE ? OUT_COLOR : img.data[left];
      upC = up == ERROR_CODE ? OUT_COLOR : img.data[up];
      vector<uint> match;
      if (leftC == curr) match.push_back(res.data[left]);
      if (upC == curr) match.push_back(res.data[up]);
      if (connectivity == 8) {
        leftup = GetNW(currPos, img.cols);
        rightup = GetNE(currPos, img.cols);
        leftupC = leftup == ERROR_CODE ? OUT_COLOR : img.data[leftup];
        rightupC = rightup == ERROR_CODE ? OUT_COLOR : img.data[rightup];
        if (leftupC == curr) match.push_back(res.data[leftup]);
        if (rightupC == curr) match.push_back(res.data[rightup]);
      }
      if (match.size() == 0) {
        res.data[currPos] = currLabel;
        AddLabel(currLabel, labToNode);
      }
      else {
        sort(match.begin(), match.end());
        uint ref = match[0];
        res.data[currPos] = ref;
        for (int i = 1; i < (int)match.size(); ++i) { 
          if (match[i] != ref) { 
              Union(ref, match[i], labToNode);
          }
        }
      }
    }
  }
  // imshow( "res img", res);
}


// The Union-Find labeling algoritm. Works in two passes and uses a Union-Find
// data strucure to resolve the equivalences between labels. 
void FindConnectedComponents(const Mat& img, Mat& res, uint connectivity) {
  uint currLabel = 0;
  map<uint, node *> labToNode;

  // first pass: assign labels to different zones
  // res.data[0] = currLabel;
  AddLabel(currLabel, labToNode);
  if (connectivity == 4) {
    FirstPass<4>(img, res, labToNode, currLabel);
  }
  else if (connectivity == 8) {
    FirstPass<8>(img, res, labToNode, currLabel);
  }
  else {
    cout << "This is not a valid connectivity! Exiting ..." << endl;
    exit(1);
  }

  // second pass: merge classes if necessary
  uint xLen = img.size().width;
  uint yLen = img.size().height;
  SecondPass(labToNode, res, xLen, yLen, currLabel);
}

int main(int argc, char **argv) {
  string infile = "";
  string outfile = "output";
  uint connectivity = 4;
  if (argc > 1) {
      infile = argv[1];
  }
  else { 
      cout << "You must provide an input file as argument." << endl;
      exit(1);
  }
  if (argc > 2) {
      outfile = argv[2];
  }
  if (argc > 3) { 
      connectivity = atoi(argv[3]);
  }

  // Load an image
  Mat src = imread(infile, IMREAD_GRAYSCALE);  // IMREAD_UNCHANGED
  if ( src.empty() ) {
    cout << "Fail to open file: " << argv[1] << endl << endl;
    cout << "You must provide an input file as argument." << endl;
    return -1;
  }
  imshow( "SRC img", src);

  Mat res(src.size(), src.type(), Scalar(0));
  FindConnectedComponents(src, res, connectivity);

  Mat dst = Mat::zeros(src.size(), src.type());

  /// Using Canny's output as a mask, and display result
  src.copyTo(dst, res);
  cv::imshow("dst img", dst);
  // Wait until user exit program by pressing a key
  waitKey(0);
  return 0;

  // uint xLen, yLen;
  // vector<uint> vec = ReadImgFromFile(infile, xLen, yLen);
  // vector<uint> res(vec.size());
  // FindConnectedComponents(vec, res, xLen, yLen, connectivity);
  // RelabelImg(res, xLen, yLen);
  // WriteImgToFile(outfile, res, xLen, yLen); 
  // return 0;
}
