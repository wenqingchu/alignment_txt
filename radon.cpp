#include"LP.h"

void LicensePlate::rotate_image(const Mat &src, Mat &dst, double angle) {
	Point2d src_center(src.cols / 2.0, src.rows / 2.0);
	Mat rot_mat = getRotationMatrix2D(src_center, angle, 1.0);
	warpAffine(src, dst, rot_mat, src.size());
}

int project_sum(const Mat &src) {
	
	vector<int> cnt(src.rows, 0);
	//cout << "rows: " << src.rows << endl;
	for (int i = 0.3 * src.rows; i < 0.7 * src.rows; ++i)
	{
		for (int j = 10; j < src.cols-10; ++j)
		{
		  cnt[i] += (src.at<uchar>(i, j));
		  
		}
	}
			
		
	return *max_element(cnt.begin(), cnt.end());



/*

	vector<int> cnt(src.rows, 0);
	int ret = 0;
	int tmp = 0;
	for (int j = 0.3 * src.cols; j < 0.7 * src.cols; ++j)
	{
		tmp = 0;
		for (int i = 0; i < src.rows; ++i)
		{
		  tmp += (src.at<uchar>(i, j));
		  
		}
		cout << "tmp " << tmp << endl;
		if(tmp < 80)
		{
		  ret ++;
		}
	}
			
	

	return ret;

*/



}


int myproject_sum(const Mat &src) {
	vector<int> cnt(src.rows, 0);
	int ret = 0;
	for (int i = 1; i < src.rows; ++i)
	{
		for (int j = 0; j < src.cols; ++j)
		{
		  cnt[i] += (src.at<uchar>(i, j));
		  
		}
		if(cnt[i] < 10)
		{
		  ret ++;
		}
	}
			
		

	return ret;
}


bool is_minimum(const vector<int> &cnt, int id) {
	int size = (int)cnt.size();
	int radius = size / 20;
	if (id - 1 >= 0 && cnt[id] > cnt[id - 1])
		return false;
	if (id + 1 < size && cnt[id] > cnt[id + 1])
		return false;
	bool flag = true;
	for (int i = id - radius; i <= id + radius; ++i) {
		if (i >= 0 && i < size && cnt[i] < cnt[id]) {
			flag = false;
			break;
		}
	}
	return flag;
}

void vertical_cut(const Mat &src, Mat &dst) {
	vector<int> cnt(src.rows, 0);
	int total = 0;
	for (int i = 0; i < src.rows; ++i)
		for (int j = 0; j < src.cols; ++j) {
			int tmp = src.at<uchar>(i, j);
			cnt[i] += tmp;
			total += tmp;
		}
	int threshold = total / (3 * src.rows);
	int start_row, end_row;
	for (int i = 0; i < src.rows; ++i) {
		if (cnt[i] < threshold && is_minimum(cnt, i)) {
			start_row = i;
			break;
		}
	}
	for (int i = src.rows - 1; i >= 0; --i) {
		if (cnt[i] < threshold && is_minimum(cnt, i)) {
			end_row = i;
			break;
		}
	}
	dst = src(Range(start_row+1, end_row-1), Range::all()).clone();
}

double LicensePlate::horizen(const Mat &src, Mat &dst) {
	int sz = (int)sqrt(double(src.rows * src.rows) + double(src.cols * src.cols)) + 10;
	int lp_sz = (int)sqrt(double(LP.rows * LP.rows) + double(LP.cols * LP.cols)) + 10;
	Mat img_ext = Mat(sz, sz, CV_8UC1, Scalar::all(0));
	Mat myimg_ext = Mat(sz, sz, CV_8UC1, Scalar::all(0));

	Mat myLP_ext;
	if (LP.channels() == 3){
	  myLP_ext = Mat(lp_sz, lp_sz, CV_8UC3, Scalar::all(0));
	}
	else{
	  myLP_ext = Mat(lp_sz, lp_sz, CV_8UC1, Scalar::all(0));
	}
	
	Mat temp;
	
	//threshold(src, temp, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	//OutputImage(src);
	Sobel(src,temp,CV_8U,0,1);
	
	int st_row = sz / 2 - src.rows / 2;
	int st_col = sz / 2 - src.cols / 2;
	
	
	int lp_st_row = lp_sz / 2 - LP.rows / 2;
	int lp_st_col = lp_sz / 2 - LP.cols / 2;
	
	src.copyTo(img_ext(Range(st_row, st_row+src.rows), Range(st_col, st_col+src.cols)));
	temp.copyTo(myimg_ext(Range(st_row, st_row+temp.rows), Range(st_col, st_col+temp.cols)));	
	LP.copyTo(myLP_ext(Range(lp_st_row, lp_st_row+LP.rows), Range(lp_st_col, lp_st_col+LP.cols)));
	
	int project_max = 0, submax = 0;
	double theta_record;
	int sum;
	Mat img_rot;
	for (double theta = -40; theta < 40; ++theta) {
		rotate_image(myimg_ext, img_rot, theta);
		//OutputImage(img_rot);

		sum = project_sum(img_rot);
		if (sum > project_max) {
		  project_max = sum;
		  theta_record = theta;
		}
		//cout << theta << " : " << sum << endl;
	}
	rotate_image(img_ext, dst, theta_record);
	rotate_image(myLP_ext, myLP_ext, theta_record);
	
	int s = src.rows - src.cols * tan(theta_record * 3.141592654 / 180);
	int lp_s = LP.rows - LP.cols * tan(theta_record * 3.141592654 / 180);
	s = s * cos(theta_record * 3.141592654 / 180) + 5; 
	lp_s = lp_s * cos(theta_record * 3.141592654 / 180) + 20;
	lp_s = LP.rows;
	/*
	if(theta_record < 0)
	{
	  s = src.rows;
	  lp_s = LP.rows;
	}
	*/
	//s = (src.rows + s) / 2;
	//lp_s = (LP.rows + lp_s) / 2;
	
	int c = src.cols+10;
	int lp_c = LP.cols+20;
	
	/*
	if(theta_record > 0 && theta_record < 45)
	{
	  c = src.cols / (cos((theta_record) * 3.141592654 / 180));
	}
	*/
	c = src.cols / (cos((theta_record) * 3.141592654 / 180));
	lp_c = LP.cols / (cos((theta_record) * 3.141592654 / 180));

	Rect ds;
	c = c + 10;
	s = s + 4;
	lp_c = lp_c + 10;
	lp_s = lp_s + 8;
	
	if (c>sz)
	{
	  c = sz;
	}
	if (lp_c>lp_sz)
	{
	  lp_c = lp_sz;
	}
	
	ds.x = sz/2 - c/2;
	ds.y = sz/2 - s/2;
	ds.width = c; 
	ds.height = s;

	dst = dst(ds);

	resize(dst,dst,src.size());
	
	ds.x = lp_sz/2 - lp_c/2;
	ds.y = lp_sz/2 - lp_s/2;
	ds.width = lp_c; 
	ds.height = lp_s;
	
	LP = myLP_ext(ds);
	

	return theta_record;
	
}


double LicensePlate::vertical(const Mat &src, Mat &dst) {
	int sz = (int)sqrt(double(src.rows * src.rows) + double(src.cols * src.cols)) + 10;
	Mat img_ext = Mat(sz, sz, CV_8UC1, Scalar::all(0));
	Mat myimg_ext = Mat(sz, sz, CV_8UC1, Scalar::all(0));
	
	
	Mat temp;
	Sobel(src,temp,CV_8U,1,0);
	int st_row = sz / 2 - src.rows / 2;
	int st_col = sz / 2 - src.cols / 2;
	
	temp.copyTo(myimg_ext(Range(st_row, st_row+temp.rows), Range(st_col, st_col+temp.cols)));	
	
	
	threshold(myimg_ext, myimg_ext, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	int project_max = 0, submax = 0;
	double theta_record,stheta;
	int sum;
	Mat img_rot;
	for (double theta = 75; theta < 105; ++theta) {
	  if(theta == 90) continue;
	  rotate_image(myimg_ext, img_rot, theta);
	  sum = myproject_sum(img_rot);
	  if (sum > project_max) {
	    project_max = sum;
	    theta_record = theta;
	    
	  }
	}

	theta_record = 90 - theta_record;
	
	int LP_c = LP.cols + 2 * LP.rows * abs(tan(theta_record * 3.141592654 / 180));
	Mat myLP_ext;
	//cout << LP.cols << " " << LP.rows << endl;
	//cout << LP_c << endl;
	if (LP.channels() == 3){
	  myLP_ext = Mat(LP.rows, LP_c, CV_8UC3, Scalar::all(0));
	}
	else{
	  myLP_ext = Mat(LP.rows, LP_c, CV_8UC1, Scalar::all(0));
	}
	//cout << myLP_ext.cols << " " << myLP_ext.rows << endl;
	LP.copyTo(myLP_ext(Range(0, LP.rows), Range((LP_c-LP.cols)/2, (LP_c-LP.cols)/2+LP.cols)));
	
	//OutputImage(myLP_ext);
		
	vector<Point2f> corners(4);  

	vector<Point2f> corners_trans(4);  
	corners_trans[0] = Point2f(LP.cols-1,0);  
	corners_trans[1] = Point2f(LP.cols-1,LP.rows-1);  
	corners_trans[2] = Point2f(0,LP.rows-1);  
	corners_trans[3] = Point2f(0,0);  
	
	
	if(theta_record > 0)
	{
	  corners[0] = Point2f(LP.cols + (LP_c - LP.cols) / 2 -1,0);  
	  corners[1] = Point2f(LP_c-1,LP.rows-1);  
	  corners[2] = Point2f((LP_c - LP.cols) / 2-1, LP.rows-1);  
	  corners[3] = Point2f(0,0);  
	  
	  

	  /*
	  for(int i=0; i<LP.rows; i++)
	  {
	    int start = i * tan(theta_record * 3.141592654 / 180);
	    
	    for(int j=0; j<LP.cols; j++)
	    {
	      if(start + j>= LP.cols-1) break;
	      LP.at<Vec3b>(i,j) = LP.at<Vec3b>(i, start+j);
	    }
	  }
	  */

	}
	else if(theta_record < 0)
	{
	  
	  
	  corners[0] = Point2f(LP_c-1,0);  
	  corners[1] = Point2f(LP.cols + (LP_c - LP.cols) / 2-1,LP.rows-1);  
	  corners[2] = Point2f(0, LP.rows-1);  
	  corners[3] = Point2f((LP_c - LP.cols) / 2-1,0); 
	  /*
	  theta_record = -theta_record;
	  for(int i=0; i<LP.rows; i++)
	  {
	    int start = (LP.rows-i) * tan(theta_record * 3.141592654 / 180);
	    
	    for(int j=0; j<LP.cols; j++)
	    {
	      if(start + j>= LP.cols-1) break;
	      LP.at<Vec3b>(i,j) = LP.at<Vec3b>(i, start+j);
	    }
	  }
	  */
	}
	//cout << corners << endl;
	//cout << corners_trans << endl;
	//cout << myLP_ext.cols << " " << myLP_ext.rows << endl;
	//cout << LP.cols << " " << LP.rows << endl;
	
	Mat mapMatrix = getPerspectiveTransform(corners,corners_trans);
	warpPerspective(myLP_ext, LP, mapMatrix, cv::Size(LP.cols + (LP_c - LP.cols) / 2, LP.rows));
	//cout << theta_record << endl;
	return theta_record;
	
}
