#include "LP.h"
//#define WRITE
Mat InputImage(string s) {
	Mat LP = imread(s);
	if(LP.empty()) cout << "File not exist!" << endl;
	return LP;
}

void OutputImage(Mat x, string windowName, int waitTime) {
#ifndef WRITE	
	imshow(windowName, x);
	moveWindow(windowName, 100, 80);
	waitKey(waitTime);
#endif
}