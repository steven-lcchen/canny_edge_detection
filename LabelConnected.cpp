/* 
 * File: LabelConnected.cpp
 * Purpose: Label Connected Components
 *   - Implements the Union-Find labeling algorithm in serial.
 *   - Contains a Union-Find data structure to resolve equivalences between labels.
 */

#include <algorithm>
#include <iostream>
#include <utility>
#include <vector>
#include <map>
#include <climits>

using namespace std;

#include <opencv2/opencv.hpp>
using namespace cv;

static const uint ERROR_CODE = UINT_MAX - 1;
static const uint OUT_COLOR = UINT_MAX - 1;

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
node *FindRoot(node *n)
{ 
  while (n->parent != n) 
    n = n->parent;
  return n;
}

//// set x as y's parents
// Set smaller node as larger node's parents
void Union(node *x, node *y)
{ 
  node *xRep = FindRoot(x);
  node *yRep = FindRoot(y);
  //cout << "Root of "<< x->label<<": "<<xRep->label<<endl;
  //cout << "Root of "<< y->label<<": "<<yRep->label<<endl;
  if (yRep->label > xRep->label)
    yRep->parent = xRep;
  else if (yRep->label < xRep->label)
    xRep->parent = yRep;
}

// create a link between two labels, smaller label are parents
void Union(uint lab1, uint lab2, map<uint, node *>& labToNode)
{ 
  node *x = labToNode[lab1];
  node *y = labToNode[lab2];
  //cout << "Union:"<<x->label<<" & "<< y->label<<endl;
  Union(x, y);
}


// create new node for label (and add it to the map)
void AddLabel(uint& label, map<uint, node *>& labToNode)
{ 
    node *toAdd = new node(label);
    labToNode[label] = toAdd;
    label++;
}


// First pass of the Union-Find labeling algorithm. Attributes labels
// to zones in a forward manner (i.e. by looking only at a subset of the
// neighbors). 
template<uint connectivity> void FirstPass(const Mat& img, Mat& labels, map<uint, node *>& labToNode, uint &currLabel)
{
  int width  = img.cols;
  int height = img.rows;

  uint left, up, leftup, rightup, curr, currPos;
  uint leftC, upC, leftupC, rightupC;

  for (int y=0; y<height; y++) { 
    for (int x=0; x<width; x++) { 
      vector<uint> match; // initial match vector to 0 in each loop
      leftC = (x == 0) ? OUT_COLOR : img.at<uchar>(y, x-1);
      upC = (y == 0) ? OUT_COLOR : img.at<uchar>(y-1, x);
      curr = img.at<uchar>(y, x);
      if (leftC == curr) match.push_back(labels.at<ushort>(y, x-1));
      if (upC == curr) match.push_back(labels.at<ushort>(y-1, x));

      if (connectivity == 8) {
        leftupC = (y==0 || x==0) ? OUT_COLOR : img.at<uchar>(y-1, x-1);
        rightupC = (y==0 || (x==img.cols)) ? OUT_COLOR : img.at<uchar>(y-1, x+1);
        if (leftupC == curr) match.push_back(labels.at<ushort>(y-1, x-1));
        if (rightupC == curr) match.push_back(labels.at<ushort>(y-1, x+1));
      }

      if (match.size() == 0) { // w/o any neighber connected
        labels.at<ushort>(y, x) = currLabel; // assign a new label
        AddLabel(currLabel, labToNode);  // and create a node struct
      } else {
        sort(match.begin(), match.end());
        uint ref = match[0];
        labels.at<ushort>(y, x) = ref; // according to neighber assign minimum label
        for (int i = 1; i < (int)match.size(); i++) { // mark label equivalance relationship
          if (match[i] != ref) {
              Union(ref, match[i], labToNode);
          }
        }
      }
    } // for x
  } // for y
}


// Second pass of the Union-Find labeling algorithm. Resolves 
// the equivalences of labels. 
// Modifies label by reference to avoid allocating extra memory. 
void SecondPass(map<uint, node *>& labToNode, Mat& labels, uint maxLabel) { 
  // decide on the final labels
  map<uint, uint> equivClass;
  for (int i = 0; i < (int)maxLabel; ++i) {
    equivClass[i] = FindRoot(labToNode[i])->label;
  }

  // second pass
  for (uint i = 0; i < labels.rows; i++) {
    for (uint j = 0; j < labels.cols; j++) {
//       labels.at<ushort>(i,j) = FindRoot(labToNode[labels.at<ushort>(i,j)])->label;
      labels.at<ushort>(i,j) = equivClass[labels.at<ushort>(i,j)];
    }
  }
}


// Relabels res using a predefinite order (the one then used 
// for comparison). Will modify dst by reference to avoid allowing 
// too much memory. 
int RelabelImg(Mat& res, Mat& dst) {
  uint num_objects = 0;
  std::map<uint, uint> corresp;
  uint elem, newColor;
  for (int i = 0; i < res.rows; ++i) {
    for (int j = 0; j < res.cols; ++j) {
      elem = res.at<ushort>(i, j);
      if (corresp.find(elem) == corresp.end()) { // not found, new element
        corresp[elem] = num_objects;
        newColor = num_objects;
        num_objects++;
      } else {
        newColor = corresp[elem];
      }
      dst.at<ushort>(i, j) = newColor;
    }
  }
if (0) {
  map<uint, uint>::iterator it;
  for(it = corresp.begin() ; it != corresp.end() ; it++) {
    cout<<"corresp["<<it->first<<"]= "<< it->second <<": "<<endl;
  }
}
  return num_objects;
}


// The Union-Find labeling algoritm. Works in two passes and uses a Union-Find
// data strucure to resolve the equivalences between labels. 
int LabelConnected(const Mat& img, Mat& labels, uint connectivity=8)
{
  uint currLabel = 0;
  map<uint, node *> labToNode;
  int num_objects=0;

  // first pass: assign labels to different zones
  if (connectivity == 4) {
    FirstPass<4>(img, labels, labToNode, currLabel);
  }
  else if (connectivity == 8) {
    FirstPass<8>(img, labels, labToNode, currLabel);
  }
  else {
    cout << "This is not a valid connectivity! Exiting ..." << endl;
    exit(1);
  }

  // second pass: merge classes if necessary
  SecondPass(labToNode, labels, currLabel);

  // relabel image and compact labels
  num_objects = RelabelImg(labels, labels);

// ----------------------
// just for debug message
if (0) {
  map<uint, node*>::iterator it;
  // Dump labToNode array
  for(it = labToNode.begin() ; it != labToNode.end() ; it++) {
    cout<<"labToNode["<<it->first<<"]= "<< labToNode[it->first] <<": ";
    cout << "Label " << labToNode[it->first]->label;
    node *n;
    n = labToNode[it->first];
    while (n->parent != n) {
      cout << "->"<<n->parent->label;
      n = n->parent;
    }
    cout << endl;
  }

  for(it = labToNode.begin() ; it != labToNode.end() ; it++) {
    if (labToNode[it->first] == labToNode[it->first]->parent) {
      cout << "Root label: " << labToNode[it->first]->label << endl;
      num_objects++;
    }
  }
  cout << "currLabel: " << currLabel << endl;
  cout << "num_objects: " << num_objects << endl;
}
// ----------------------
  return num_objects;
}


