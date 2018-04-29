#include<iostream>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<vector>

using namespace std;
using namespace cv;

int main()
{
	Mat img = imread("../images/new.png",CV_LOAD_IMAGE_GRAYSCALE);
	cout<<img.rows<<' '<<img.cols<<endl;
	vector<Vec4i> lines;
	Mat color_lines = img.clone();
	HoughLinesP(img, lines, 3.0, 2.0*CV_PI/180, 2);
	for(int i = 0; i < lines.size(); i++)
		line(color_lines, Point(lines[i][0], lines[i][1]), Point(lines[i][2], lines[i][3]), Scalar(0,0,255), 3, 8);
	imshow("lines_bisect",color_lines);
	waitKey(0);
	vector<Vec4i> lines_1;
	HoughLinesP(img, lines_1, 3.0, 2.0*CV_PI/180, 30);
	for(int i = 0; i < lines_1.size(); i++)
		line(color_lines, Point(lines_1[i][0], lines_1[i][1]), Point(lines_1[i][2], lines_1[i][3]), Scalar(255,0,0), 3, 8);
	imshow("lines_bisect",color_lines);
	// imshow("lines_gray",img_gray);
	waitKey(0);
}