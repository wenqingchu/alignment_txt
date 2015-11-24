#include <opencv2/opencv.hpp>
#include <iostream>
#include <numeric>
#include <ctime>
#include <math.h>
#include <algorithm>

using namespace cv;
using namespace std;

//#define WRITE

const int LPNormalRows = 100;
const int MAXR = 800;
const int MAXC = 1200;

class LicensePlate {
public:
	Mat gx_img;
	Mat LP;
	Mat LPColor;
	Mat LPRedChannel;
	Mat LPOpen;
	Mat LPR[7];
	double horizen_record;
	double vertical_record;
	LicensePlate(Mat s) : LP(s), LPColor(s) {}
	void setImage(Mat s){LP=s;LPColor=s;}
	void LPPreproccessing(const Mat&);
	void ScanToCutHorizenFrame(Mat& LP, Rect& LPRect);
	//void ScanToCutVerticalFrame(Mat& LP, Rect& LPRect, Mat& RRR);
	//vector<Rect> LPCharSegmentation(Rect);
	void LPAlignment();

	string LPCharRecognition(vector<Rect>);
	Mat LPRun();
	void LPWrite(const string& path);
private:
	void ImageEnhancement(Mat& LP);
	double horizen(const Mat &src, Mat &dst);
	double vertical(const Mat &src, Mat &dst);
	void rotate_image(const Mat &src, Mat &dst, double angle);
	void myFloodFill(const Mat& d, int r, int c, int label, int R, int C);
	int GetComponent(const Mat& LP, vector<int>& area, vector<Rect>& rect);
	void MorphOperation(Mat& LP);
	void DeleteSmallNonLPCompoent(Mat& LP, vector<int>& area, vector<Rect>& rect, int minComponentThreshold);
	void UnionComponent(Mat& LP, vector<int>& area, vector<Rect>& rect);
	void FinallyDeleteNonLPCompoent(Mat& LP, vector<int>& area, vector<Rect>& rect, int minComponentThreshold);
	bool IsLPColor(int H, int S, int B);
	bool IsMyLPColor(int b, int g, int r);
	bool IsPossibleLPUsingColorFeature(const Mat& LPWithColor, Rect& rect, int& area);
	bool IsPossibleLPUsingCharFeature(Rect LPRect);
};

Mat InputImage(string);
void OutputImage(Mat, string windowName = "", int waitTime = 0);
void vertical_cut(const Mat &src, Mat &dst);
