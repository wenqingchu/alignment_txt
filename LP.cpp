#include "LP.h"
#include <iostream>
#include <fstream>

short dr[4][2] = {1, 0, 0, 1, -1, 0, 0, -1};
short q[MAXR * MAXC][2];
short mark[MAXR][MAXC];
void LicensePlate::myFloodFill(const Mat& d, int r, int c, int label, int R, int C) {
	int head = 0;
	int tail = 0;
	q[tail][0] = r;
	q[tail][1] = c;
	tail++;
	mark[r][c] = label;
	while(head < tail) {
		int cr = q[head][0];
		int cc = q[head][1];
		head++;
		for(int i = 0; i < 4; i++) {
			int nr = cr + dr[i][0];
			int nc = cc + dr[i][1];
			if(nr < 0 || nr >= R || nc < 0 || nc >= C || mark[nr][nc] != -1 || !d.at<uchar>(nr, nc)) continue;
			mark[nr][nc] = label;
			q[tail][0] = nr;
			q[tail][1] = nc;
			tail++;
		}
	}
}

int LicensePlate::GetComponent(const Mat& LP, vector<int>& area, vector<Rect>& rect) {
	int R = LP.rows;
	int C = LP.cols;
	memset(mark, -1, R * sizeof(mark[0]));
	int nComponent = 0;
	for(int r = 0; r < R; r++)
		for(int c = 0; c < C; c++) {
			if(mark[r][c] == -1 && LP.at<uchar>(r, c))
				myFloodFill(LP, r, c, nComponent++, R, C);
		}
	area.resize(nComponent, 0);
	rect.resize(nComponent);
	vector<pair<Point, Point> > vr(nComponent);
	for(int i = 0; i < nComponent; i++) {
		vr[i].first.x = 10000;
		vr[i].first.y = 10000;
		vr[i].second.x = 0;
		vr[i].second.y = 0;
	}
	for(int r = 0; r < R; r++)
		for(int c = 0; c < C; c++) {
			int which = mark[r][c];
			if(which != -1) {
				area[which]++;
				if(vr[which].first.x > c) vr[which].first.x = c;
				if(vr[which].first.y > r) vr[which].first.y = r;
				if(vr[which].second.x < c) vr[which].second.x = c;
				if(vr[which].second.y < r) vr[which].second.y = r;
			}
		}
	for(int i = 0;  i < nComponent; i++) {
		rect[i] = Rect(vr[i].first, vr[i].second);
	}
	return nComponent;
}


bool LicensePlate::IsMyLPColor(int b, int g, int r) {
	return ((b>110 && g<90 && r<60) || (b<50 && g>200 && r>200));
}

void LicensePlate::LPPreproccessing(const Mat& LP) {

  if (LPColor.channels() == 3){
    	vector<Mat> mv;
	split(LPColor, mv);
	LPRedChannel = mv[2].clone();
  }
  else{
    LPRedChannel = LPColor.clone();
  }
  double scale = 100 / LP.rows;
  resize(LPRedChannel, LPRedChannel, Size(int(LP.cols*scale), int(LP.rows*scale)));
}


void LicensePlate::DeleteSmallNonLPCompoent(Mat& LP, vector<int>& area, vector<Rect>& rect, int minComponentThreshold) {
	int nComponent = area.size();
	for(int i = 0; i < nComponent; i++) {
		if(area[i] < minComponentThreshold) area[i] = 0;
		if(rect[i].height < 30) area[i] = 0;
	}
	for(int r = 0; r < LP.rows; r++)
		for(int c = 0; c < LP.cols; c++) {
			int which = mark[r][c];
			if(which != -1 && !area[which]) {
				LP.at<uchar>(r, c) = 0;
			}
		}
	vector<int> tArea = area;
	vector<Rect> tRect = rect;
	area.clear();
	rect.clear();
	for(int i = 0; i < nComponent; i++) {
		if(tArea[i]) {
			area.push_back(tArea[i]);
			rect.push_back(tRect[i]);
		}
	}
}

bool Intersect(Rect a, Rect b, Rect& uRect) {
	if(a.x > b.x) swap(a, b);
	uRect = a | b;
	if(abs(a.y - b.y) < 30 && abs(a.height - b.height) < 30 && ((a & b).width > 0 || abs(a.x + a.width - b.x) < 50)) return 1;
	return 0;
}

void LicensePlate::UnionComponent(Mat& LP, vector<int>& area, vector<Rect>& rect) {
	int maxUnion = 20;
	while(maxUnion--) {
		int changed = 1;
		int nComponent = area.size();
		vector<int> ar;
		vector<Rect> rt;
		for(int i = 0; i < nComponent; i++) {
			if(!area[i]) continue;
			bool isIntersect = 0;
			for(int j = i + 1; j < nComponent; j++) {
				Rect uRect;
				if(area[j] && Intersect(rect[i], rect[j], uRect)) {
					ar.push_back(area[i] + area[j]);
					rt.push_back(uRect);
					area[j] = 0;
					isIntersect = 1;
					break;
				}
			}
			if(!isIntersect) {
				ar.push_back(area[i]);
				rt.push_back(rect[i]);
			}
		}
		area = ar, rect = rt;
		if(!changed) break;
	}
}

bool LicensePlate::IsLPColor(int H, int S, int B) {
	return ((H > 90 && H < 130) || (H > 10 && H < 40)) && S > 30 && B > 50;
}

bool LicensePlate::IsPossibleLPUsingColorFeature(const Mat& LPColor, Rect& rect, int& area) {
	Mat ROI = LPColor(rect).clone();
	Mat binary, HSV;
	cvtColor(ROI, binary, CV_BGR2GRAY);
	cvtColor(ROI, HSV, CV_BGR2HSV);
	int R = HSV.rows;
	int C = HSV.cols;
	int nBluePixel = 0;
	for(int r = 0; r < R; r++)
		for(int c = 0; c < C; c++) {
			uchar h = HSV.at<uchar>(r, c * 3);
			uchar s = HSV.at<uchar>(r, c * 3 + 1);
			uchar b = HSV.at<uchar>(r, c * 3 + 2);
			if(IsLPColor(h, s, b)) {
				nBluePixel++;
				binary.at<uchar>(r, c) = 255;
			} else {
				binary.at<uchar>(r, c) = 0;
			}
		}
	//OutputImage(binary);
	//cout << "LP area color pixels: " << nBluePixel << endl;
	if(nBluePixel < 10000) return 0;
	//if(rect.area() < 50000) return 1;
	//OutputImage(binary);
	vector<Rect> rt;
	vector<int> ar;

	Mat HClose = getStructuringElement(MORPH_CROSS, Size(15, 1));
	morphologyEx(binary, binary, MORPH_CLOSE, HClose);
	//OutputImage(binary);
	Mat VerticalGap = getStructuringElement(MORPH_CROSS, Size(1, 5));

	morphologyEx(binary, binary, MORPH_OPEN, VerticalGap);

	GetComponent(binary, ar, rt);
	for(int i = 0; i < ar.size(); i++)
		rectangle(binary, rt[i], Scalar(255, 255, 255), 3);


	int which = max_element(ar.begin(), ar.end()) - ar.begin();
	rt[which].x += rect.x;
	rt[which].y += rect.y;
	rect = rt[which];
	area = ar[which];
	return 3 * nBluePixel > rect.area();
}

bool LicensePlate::IsPossibleLPUsingCharFeature(Rect LPRect) {
	Mat ROI = LPRedChannel(LPRect).clone();
	medianBlur(ROI, ROI, 3);
	threshold(ROI, ROI, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	Mat RR = ROI.clone();
	Mat HClose = getStructuringElement(MORPH_CROSS, Size(45, 1));
	morphologyEx(RR, RR, MORPH_CLOSE, HClose);

	vector<int> ar;
	vector<Rect> rt;

	GetComponent(RR, ar, rt);
	int maxRectArea = 0, p = -1;
	for(int i = 0; i < ar.size(); i++) {
		if(maxRectArea < ar[i]) {
			maxRectArea = ar[i];
			p = i;
		}
	}
	assert(p != -1);
	LPRect.x += rt[p].x;
	LPRect.y += rt[p].y;
	LPRect.width = rt[p].width;
	LPRect.height = rt[p].height;

	ROI = ROI(rt[p]);

	int xHist[MAXC];
	memset(xHist, 0, sizeof(xHist));
	for(int r = 0; r < ROI.rows; r++)
		for(int c = 0; c < ROI.cols; c++) {
			if(ROI.at<uchar>(r, c)) xHist[c]++;
		}

	int N = ROI.cols;
	int rk[MAXC];
	memcpy(rk, xHist, sizeof(rk));
	sort(rk, rk + N);
	int bottomThreshold = 0;
	int topThreshold = 0;
	int percent = N / 3;
	for(int i = 0; i < percent; i++) {
		bottomThreshold += rk[i];
		topThreshold += rk[N - i - 1];
	}
	bottomThreshold /= percent;
	topThreshold /= percent;

	bottomThreshold = max(topThreshold / 10, bottomThreshold);
	bool isTop[MAXC];
	for(int i = 0; i < N; i++) {
		isTop[i] = (xHist[i] >= bottomThreshold);
	}
	Mat hist(1, ROI.cols, CV_8UC1);
	vector<pair<int, int> > interval, unionInterval;
	for(int i = 0; i < ROI.cols; i++) {
		hist.at<uchar>(0, i) = isTop[i];
		if(isTop[i]) interval.push_back(make_pair(i, i + 1));
	}
	int L, R = -100;
	for(int i = 0; i < interval.size(); i++) {
		int l = interval[i].first;
		int r = interval[i].second;
		if(l - R < 5) {
			R = r;
		} else {
			if(R != -100) unionInterval.push_back(make_pair(L, R));
			L = l, R = r;
		}
	}
	if(R != -100) unionInterval.push_back(make_pair(L, R));

	if(unionInterval.size() < 3 || unionInterval.size() > 10) return 0;

	return 1;
}


void LicensePlate::FinallyDeleteNonLPCompoent(Mat& LP, vector<int>& area, vector<Rect>& rect, int minComponentThreshold = 0) {
	int nComponent = area.size();
	vector<pair<int, int> > score(nComponent);
	for(int i = 0; i < nComponent; i++) {
		if(area[i] < minComponentThreshold) score[i].first = 0, score[i].second = 0;
		else {
			score[i].first += 100;
			score[i].second += 100;
			if(IsPossibleLPUsingColorFeature(LPColor, rect[i], area[i])) score[i].first++, score[i].second += 10;
			int w = rect[i].width;
			int h = rect[i].height;
			if(w > 2 * h && w < 6 * h) score[i].first++, score[i].second = 10 / (abs(w / h - 4) + 1);
			if(w * h < 4 * area[i]) score[i].first++, score[i].second = 6 * area[i] / (w * h);
			if(4 * w * h < LP.rows * LP.cols) score[i].first++, score[i].second += 10;
			if(IsPossibleLPUsingCharFeature(rect[i])) score[i].first++, score[i].second += 10;
		}
	}

	pair<int, int> maxScore = *max_element(score.begin(), score.end());

	for(int i = 0; i < nComponent; i++)
		if(score[i].first < maxScore.first) area[i] = 0;

	vector<int> tArea = area;
	vector<Rect> tRect = rect;
	area.clear();
	rect.clear();
	for(int i = 0; i < nComponent; i++) {
		if(tArea[i]) {
			area.push_back(tArea[i]);
			rect.push_back(tRect[i]);
		}
	}
}

void LicensePlate::ImageEnhancement(Mat& LP) {
	int hist[256];
	memset(hist, 0, sizeof(hist));
	for(int r = 0; r < LP.rows; r++)
		for(int c = 0; c < LP.cols; c++)
			++hist[LP.at<uchar>(r, c)];
	int size = LP.rows * LP.cols;
	int L = size / 100;
	int H = 99 * size / 100;
	int a = 0, b = 255;
	int tot = 0;
	for(int i = 0; i < 256; i++) {
		tot += hist[i];
		if(tot < L) a = i;
		if(tot > H && b == 255) b = i - 1;
	}
	int match[256];
	for(int i = 0; i < 256; i++) {
		if(i < a) match[i] = 0;
		else if(i > b) match[i] = 255;
		else match[i] = 255 * (i - a) / (b - a);
	}
	for(int r = 0; r < LP.rows; r++)
		for(int c = 0; c < LP.cols; c++) {
			uchar &t = LP.at<uchar>(r, c);
			t = match[t];
		}
}

void LicensePlate::MorphOperation(Mat& LP) {
	Mat Back = getStructuringElement(MORPH_RECT, Size(25, 25));
	Mat VerticalGap = getStructuringElement(MORPH_CROSS, Size(1, 5));
	Mat One = getStructuringElement(MORPH_CROSS, Size(35, 1));
	morphologyEx(LP, LP, MORPH_TOPHAT, Back);
	ImageEnhancement(LP);
	threshold(LP, LP, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	LPOpen = LP.clone();
	morphologyEx(LP, LP, MORPH_CLOSE, One);
	morphologyEx(LP, LP, MORPH_OPEN, VerticalGap);//67
}


void LicensePlate::ScanToCutHorizenFrame(Mat& LP, Rect& LPRect) {
  
  
  	int R = LP.rows;
	int C = LP.cols;

	int isOkRow[MAXR], isOkCol[MAXR], rCount[MAXR], cCount[MAXR];
	memset(isOkRow, 0, sizeof(isOkRow));
	memset(isOkCol, 0, sizeof(isOkCol));
	memset(rCount, 0, sizeof(rCount));
	memset(cCount, 0, sizeof(cCount));
	
	int threshold = 0;
	Mat ttemp;
	Sobel(LP,ttemp,CV_8U,1,0);
	//OutputImage(ttemp,"sobel");

	for(int r = 0; r < R; r++) {
		int nChange = 0;
		for(int c = 5; c < C; c++) {
			if(LP.at<uchar>(r, c) != LP.at<uchar>(r, c - 5)) nChange++;
			if(LP.at<uchar>(r, c) == 255)
			{
			  rCount[r]++;
			  //cCount[c]++;
			}
		}
		if(r>25 && r<=75)
		{
		  threshold = threshold + rCount[r];
		}
		isOkRow[r] = nChange;
		//printf("r:%d, rCount:%d, nChange:%d\n", r,rCount[r], nChange);
	}
	
	threshold = 0.55 * threshold / 50;
	//cout << "threshold: " << threshold/50 << endl;
	//cout << "threshold: " << threshold << endl;
	int startR = 10, endR = 90;
	for(int r = 2*R/3; r < R; r++)
	{
	  if((rCount[r] < threshold || rCount[r] > 180) || (isOkRow[r]<60))
	  {
	    endR = r-1;
	    break;
	  }
	}
	
	for(int r = R/3; r > 0; r--)
	{
	  if((rCount[r] < 40 || rCount[r] > 180) || (isOkRow[r]<60))
	  {
	    startR = r+1;
	    break;
	  }
	}
	//printf("startR:%d, endR:%d\n", startR, endR);

	LPRect.y += startR;
	LPRect.height = endR - startR + 1;
	//printf("LPRect.height:%d\n", LPRect.height);
	startR = startR > 3?startR:4;
	for(int c = 0; c < C; c++) {
		int nChange = 0;
		for(int r = startR/2; r < (endR+100)/2; r++) {
			if(LP.at<uchar>(r, c) != LP.at<uchar>(r - 2, c)) nChange++;
			if(LP.at<uchar>(r, c) == 255) cCount[c]++;
		}
		isOkCol[c] = nChange;
		//printf("c:%d, nChange:%d, cCount:%d\n", c, nChange, cCount[c]);
	}
	int startC = 2, endC = 248;

	LPRect.x += startC;
	LPRect.width = endC - startC + 1;

	
	LP = LP(Rect(startC, startR, endC - startC + 1, endR - startR + 1));
	
  
}



void LicensePlate::LPAlignment() {
  
	Mat ROI = LPRedChannel.clone();

	ImageEnhancement(ROI);

	Mat temp;
	ROI.copyTo(temp);
	horizen_record = horizen(ROI,temp);
	/*
	ROI = temp;

	threshold(ROI, ROI, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	// for other color license plate
	int black=0, white = 0;

	for(int i=15; i<ROI.rows-15; i++)
	{
	  for(int j=20; j<ROI.cols-20; j++)
	  {
	    if(ROI.at<uchar>(i,j) == 0)
	    {
	      black++;
	    }
	    else{
	      white++;
	    }
	  }
	}
	//cout << "black:  "  << black << "  white: " << white << endl;
	if(black + 500 < white)
	{
	  for(int i=0; i<ROI.rows; i++)
	  {
	    for(int j=0; j<ROI.cols; j++)
	    {
	     ROI.at<uchar>(i,j) = 255 - ROI.at<uchar>(i,j); 
	    }
	    
	  }
	}
	*/
	//vector<int> aa;
	//vector<Rect> rr;
	//GetComponent(ROI, aa, rr);
	//DeleteSmallNonLPCompoent(ROI, aa, rr, 100);
	//Rect place;
	//place.x = 0;
	//place.y = 0;
	//place.width = int(100.0 / LP.rows * LP.cols);
	//place.height = 100;
	
	//ScanToCutHorizenFrame(ROI, place);
	//vertical_record = vertical(ROI,temp);
}

Mat LicensePlate::LPRun() {
	LPPreproccessing(LP);
	
	LPAlignment();
	return LP;
}

