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
	Mat img_ext = Mat(sz, sz, CV_8UC1, Scalar::all(0));
	Mat myimg_ext = Mat(sz, sz, CV_8UC1, Scalar::all(0));

	Mat myLP_ext = Mat(sz, sz, CV_8UC3, Scalar::all(0));
	Mat myLP_dst;
	LP.copyTo(myLP_dst);
	Mat temp;
	
	//threshold(src, temp, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	OutputImage(src);
	Sobel(src,temp,CV_8U,0,1);
	
	int st_row = sz / 2 - src.rows / 2;
	int st_col = sz / 2 - src.cols / 2;
	
	
	src.copyTo(img_ext(Range(st_row, st_row+src.rows), Range(st_col, st_col+src.cols)));
	temp.copyTo(myimg_ext(Range(st_row, st_row+temp.rows), Range(st_col, st_col+temp.cols)));	
	LP.copyTo(myLP_ext(Range(st_row, st_row+LP.rows), Range(st_col, st_col+LP.cols)));
	
	//OutputImage(img_ext);
	//OutputImage(myLP_ext);
	int project_max = 0, submax = 0;
	double theta_record,stheta;
	int sum;
	Mat img_rot;
	for (double theta = -30; theta < 30; ++theta) {
		rotate_image(myimg_ext, img_rot, theta);
		//OutputImage(img_rot);	

		sum = project_sum(img_rot);
		cout << theta << " " << sum << endl;

		if (sum > project_max) {
		  project_max = sum;
		  theta_record = theta;
		}
	}
	//OutputImage(img_ext);
	rotate_image(img_ext, dst, theta_record);
	rotate_image(myLP_ext, myLP_ext, theta_record);
	//OutputImage(dst);
	int s = src.rows - src.cols * tan(theta_record * 3.141592654 / 180);
	s = s * cos(theta_record * 3.141592654 / 180) + 5; 
	if(theta_record < 0)
	{
	  s = src.rows;
	}
	s = (src.rows + s) / 2;
	
	int c = src.cols+10;
	
	if(theta_record > 0 && theta_record < 45)
	{
	  c = src.cols / (cos((theta_record) * 3.141592654 / 180));
	  //c = src.cols * cos((theta_record) * 3.141592654 / 180) + src.cols * sin((theta_record) * 3.141592654 / 180);
	}

	Rect ds;
	ds.x = sz/2 - c/2-5;
	ds.y = sz/2 - s/2;
	ds.width = c+10; 
	ds.height = s;
	//cout << ds.x << " "  << ds.y << " " << c << " " << s << endl;
	//cout << sz << endl;

	//OutputImage(dst);
	dst = dst(ds);
	//OutputImage(dst);
	//OutputImage(myLP_ext);
	myLP_dst = myLP_ext(ds);
	//OutputImage(myLP_dst);
	resize(dst,dst,src.size());
	resize(myLP_dst,LP,LP.size());
	imwrite("tmp.jpg",LP);

	cout << "horizon: " << theta_record << endl;
	

	return theta_record;
	
}


double LicensePlate::vertical(const Mat &src, Mat &dst) {
	int sz = (int)sqrt(double(src.rows * src.rows) + double(src.cols * src.cols)) + 10;
	Mat img_ext = Mat(sz, sz, CV_8UC1, Scalar::all(0));
	Mat myimg_ext = Mat(sz, sz, CV_8UC1, Scalar::all(0));
	
	
	Mat temp;
	OutputImage(src);
	Sobel(src,temp,CV_8U,1,0);
	int st_row = sz / 2 - src.rows / 2;
	int st_col = sz / 2 - src.cols / 2;
	
	temp.copyTo(myimg_ext(Range(st_row, st_row+temp.rows), Range(st_col, st_col+temp.cols)));	
	
	
	threshold(myimg_ext, myimg_ext, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	int project_max = 0, submax = 0;
	double theta_record,stheta;
	int sum;
	Mat img_rot;
	for (double theta = 65; theta < 115; ++theta) {
	  if(theta == 90) continue;
		//rotate_image(myimg_ext, img_rot, theta);
		rotate_image(myimg_ext, img_rot, theta);
		sum = myproject_sum(img_rot);
		if (sum > project_max) {
		  project_max = sum;
		  theta_record = theta;
		}
	}

	theta_record = 90 - theta_record;
	
	if(theta_record > 0)
	{
	  
	  
	  for(int i=0; i<LP.rows; i++)
	  {
	    int start = i * tan(theta_record * 3.141592654 / 180);
	    
	    for(int j=0; j<LP.cols; j++)
	    {
	      if(start + j>= LP.cols-1) break;
	      LP.at<Vec3b>(i,j) = LP.at<Vec3b>(i, start+j);
	    }
	  }

	}
	else if(theta_record < 0)
	{
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
	}
	cout << "vertical: " << theta_record << endl;
	return theta_record;
	
}
