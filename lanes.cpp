#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <string>

#define SUBSTRACTION_CONSTANT 30
#define INTENSITY_TH 50
#define PI 3.14159265

using namespace std;
using namespace cv;

class Lanes
{
	Mat img,img_gray,bisect;
	vector<Vec2i> L,R;
public:
	Lanes(Mat im);
	void Mix_Channel();
	void Intensity_distribution();
	void display();
	void Intensity_adjust();
	void Brightest_Pixel();
	void Edge();
	void Hough();
	void Dilation();
	void control_points();
	void remove_grass();
	void control_vanishing();
};

int main(int argc, char** argv)
{
	if(argc < 2) return 0;
	VideoCapture vc(argv[1]);
	Mat frame;
	vc>>frame;
	resize(frame,frame,Size(1200,600));
	cout<<frame.rows<<' '<<frame.cols;
	int c = 0;
	while(frame.data)
	{
		c++;
		vc>>frame;
		if(c%10 != 0 ) continue;
		if(c > 1000) c=0;
		resize(frame,frame,Size(1200,600));
		Lanes L(frame	);
		L.Intensity_adjust();
		L.Mix_Channel();
		L.remove_grass();
		// L.display();
		// L.Intensity_distribution();

		// L.Brightest_Pixel();
		// L.control_points();
		// L.Hough();
		// L.Intensity_distribution();
		// L.Mix_Channel();
		// L.display();
		// L.Edge();
		// L.Hough();
		// L.display();
		L.Brightest_Pixel();
		L.control_points();
		// L.control_vanishing();
		waitKey(5);
	}
	return 0;

}

Lanes::Lanes(Mat im)
{
	this->img = im;
	cvtColor(im,img_gray,CV_BGR2GRAY);
}

void Lanes::Mix_Channel()
{
	blur(img,img,Size(5,5));
	for(int i = 0; i < img.rows; i++)
		for(int j = 0; j < img.cols; j++)
		{
			int temp = 2 * img.at<Vec3b>(i,j)[0] - img.at<Vec3b>(i,j)[1];
			if(temp < 0) temp=0;
			if(temp > 255) temp=255;
			img_gray.at<uchar>(i,j) = temp;
		}
	// imshow("Mix_Chann",img_gray);
	// waitKey(0);
}

void Lanes::display()
{
	imshow("input", img);
	imshow("Mix_Channel", img_gray);
	waitKey(0);
}

void Lanes::Intensity_distribution()
{
	Mat dist(256,img.rows,CV_8UC1,Scalar(0));
	for(int i = 0; i < img.rows; i++)
	{
		int sum = 0;
		for(int j = 0; j < img.cols; j++)
		{
			sum += img.at<Vec3b>(i,j)[0] + img.at<Vec3b>(i,j)[1] + img.at<Vec3b>(i,j)[2];
		}
		sum /= 3;
		sum /= img.cols;
		dist.at<uchar>(256-sum,i) = 255;
	}
	imshow("Intensity_distribution",dist);
	waitKey(0);
}

void Lanes::Intensity_adjust()
{
	Mat filter(img.rows,img.cols,CV_8UC1,Scalar(0));
	for(int i = 0; i <= img.rows/4; i++)
		for(int j = 0; j < img.cols; j++)
			filter.at<uchar>(i,j) = (img.rows/4 - i) * (SUBSTRACTION_CONSTANT) / (img.rows/4);
	// imshow("filter",filter);
	// waitKey(0);
	for(int i = 0; i < img.rows; i++)
		for(int j = 0; j < img.cols; j++)
			for(int k = 0; k < 3; k++)
			{
				int temp = img.at<Vec3b>(i,j)[k] - filter.at<uchar>(i,j);
				if(temp < 0) temp = 0;
				img.at<Vec3b>(i,j)[k] = temp;
			}
	// imshow("Filtered Image", img);
	// waitKey(0);
}

void Lanes::remove_grass()
{
	// imshow("cv2", img);
	// waitKey(0);
	equalizeHist(img_gray, img_gray);
	Mat non_grass = img.clone();
	for(int i = 0; i < img.rows; i++)
		for(int j = 0; j < img.cols; j++)
		{
			if(img_gray.at<uchar>(i,j) < 100)
			{
				non_grass.at<Vec3b>(i,j) = {0, 0, 0};
				img_gray.at<uchar>(i,j) = 0;
			}
			else
			{
				for(int k = 0; k < 3; k++)
					if(non_grass.at<Vec3b>(i,j)[k] < 100)
					{
						non_grass.at<Vec3b>(i,j) = {0, 0, 0};
						img_gray.at<uchar>(i,j) = 0;
						break;
					}
			}
		}
	img = non_grass;
	imshow("remove_grass", img_gray);
	waitKey(5);
}

void Lanes::Brightest_Pixel()
{
	bisect = img_gray.clone();
	for(int i = 0; i < img.rows; i++)
	{
		int l,r,max = 0;
		for(int j = 0; j < img.cols/2; j++)
		{
			bisect.at<uchar>(i,j) = 0;
			// if(img_gray.at<uchar>(i,j) < INTENSITY_TH) img_gray.at<uchar>(i,j) = 0;
			// else img_gray.at<uchar>(i,j) = 255;
			int sum = 0, count = 0;
			for(int m = i-1; m < i+2; m++)
				for(int n = j-1; n < j+2; n++)
				{
					if(m < 0 || n < 0 || m >= img.rows || n >= img.cols) continue;
					sum += img_gray.at<uchar>(m,n); count++;
				}
			if(count != 0) sum /= count;
			if(sum > max)
			{
				max = sum;
				l = j;
			}
			
		}
		if(max>100) bisect.at<uchar>(i,l) = 255;
		max = 0;
		for(int j = img.cols/2 + 1; j < img.cols; j++)
		{
			bisect.at<uchar>(i,j) = 0;
			// if(img_gray.at<uchar>(i,j) < INTENSITY_TH) img_gray.at<uchar>(i,j) = 0;
			// else img_gray.at<uchar>(i,j) = 255;
			int sum = 0, count = 0;
			for(int m = i-1; m < i+2; m++)
				for(int n = j-1; n < j+2; n++)
				{
					if(m < 0 || n < 0 || m >= img.rows || n >= img.cols) continue;
					sum += img_gray.at<uchar>(m,n); count++;
				}
			if(count != 0) sum /= count;
			if(sum > max)
			{
				max = sum;
				r = j;
			}
			
		}
		if(max > 100) bisect.at<uchar>(i,r) = 255;
	}
	// Dilation();
	// imshow("bisect",bisect);
	// imshow("Threshold",img_gray);
	// waitKey(0);
}

void Lanes::Edge()
{
	Mat Gx, Gy, Ga(img.rows,img.cols,CV_8UC1,Scalar(0)), Gb(img.rows,img.cols,CV_8UC1,Scalar(0)), Gc(img.rows,img.cols,CV_8UC1,Scalar(0)), Gd(img.rows,img.cols,CV_8UC1,Scalar(0));
	equalizeHist(img_gray, img_gray);
	Mat temp = img_gray.clone();
	for(int i = 0; i < img.rows; i++)
	{
		for(int j = 0; j < img.cols; j++)
		{
			int count=0;
			for(int m = i-1; m < i+2; m++)
			{
				for(int n = j-1; n < j+2; n++)
				{
					if(m<0 || n<0 || m>=img.rows || n>=img.cols) continue;
					if(img_gray.at<uchar>(m,n) < INTENSITY_TH) count++;
				}
			}
			if(count != 0) temp.at<uchar>(i,j)=0;
		}
	}
	img_gray = temp;
	// imshow("erode",img_gray);
	// Sobel(img_gray, Gx, -1, 1, 0, 3, CV_SCHARR);
	// Sobel(img_gray, Gy, -1, 0, 1, 3, CV_SCHARR);
	// // imshow("gx",Gx);
	// // imshow("gy",Gy);
	
	// Mat temp = img_gray.clone();

	// int A[3][3] = {-1,0,1,-2,0,2,-1,0,1};
	// int B[3][3] = {-2,-1,0,-1,0,1,0,1,2};
	// int C[3][3] = {0,1,2,-1,0,1,-2,-1,0};
	// int D[3][3] = {2,2,2,0,0,0,-5,-5,-5};

	// for(int i = 0; i < img.rows; i++)
	// 	for(int j = 0; j < img.cols; j++)
	// 	{
	// 		int sum_a, sum_b, sum_c, sum_d, count_a, count_b, count_c, count_d;
	// 		sum_a = sum_b = sum_c = sum_d = count_a = count_b = count_c = count_d = 0;
	// 		for(int m = -1; m < 2; m++)
	// 			for(int n = -1; n < 2; n++)
	// 			{
	// 				if(i+m < 0 || j+n < 0 || i+m >= img.rows || j+n >= img.cols) continue;
	// 				sum_a += img_gray.at<uchar>(i+m,j+n) * A[m+1][n+1];
	// 				sum_b += img_gray.at<uchar>(i+m,j+n) * B[m+1][n+1];
	// 				sum_c += img_gray.at<uchar>(i+m,j+n) * C[m+1][n+1];
	// 				sum_d += img_gray.at<uchar>(i+m,j+n) * D[m+1][n+1];
	// 				count_a += abs(A[m+1][n+1]);
	// 				count_b += abs(B[m+1][n+1]);
	// 				count_c += abs(C[m+1][n+1]);
	// 				count_d += abs(D[m+1][n+1]);
	// 			}
	// 		sum_a /= count_a; sum_b /= count_b; sum_c /= count_c; sum_d /= count_d;
	// 		Ga.at<uchar>(i,j) = sum_a; Gb.at<uchar>(i,j) = sum_b; Gc.at<uchar>(i,j) = sum_c; Gd.at<uchar>(i,j) = sum_d;

	// 		if(sqrt(sum_a*sum_a + sum_b*sum_b + sum_c*sum_c - sum_d*sum_d) > INTENSITY_TH) temp.at<uchar>(i,j) = 255;
	// 		else temp.at<uchar>(i,j) = 0;
	// 		// int x = Gx.at<uchar>(i,j);
	// 		// int y = Gy.at<uchar>(i,j);
	// 		// float grad = atan((float)x / y) * 180.0/PI;
	// 		// // cout<<grad<<'\t';
	// 		// if(grad < 0) grad *= -1;
	// 		// if((sqrt(x*x + y*y) > INTENSITY_TH)) img_gray.at<uchar>(i,j) = 255;
	// 		// else img_gray.at<uchar>(i,j) = 0;
	// 	}
	// img_gray = temp;
	// // Canny()
	// imshow("edges",img_gray);
	// imshow("ga",Ga); imshow("gb",Gb); imshow("Gc",Gc); imshow("Gd", Gd);
	// // imshow("Gx", Gx); imshow("Gy",Gy);
	// waitKey(0);
}

void Lanes::Dilation()
{
	Mat temp = bisect.clone();
	for(int i = 0; i < img.rows; i++)
	{
		for(int j = 0; j < img.cols; j++)
		{
			int count=0;
			for(int m = i-1; m < i+2; m++)
			{
				for(int n = j-1; n < j+2; n++)
				{
					if(m<0 || n<0 || m>=img.rows || n>=img.cols) continue;
					if(img_gray.at<uchar>(m,n) > INTENSITY_TH) count++;
				}
			}
			if(count > 1) temp.at<uchar>(i,j)=255;
		}
	}
	bisect = temp;
}

void Lanes::Hough()
{
	vector<Vec4i> lines;
	Mat color_lines = img.clone();
	HoughLinesP(bisect, lines, 3.0, 2.0*CV_PI/180, 2);
	for(int i = 0; i < lines.size(); i++)
		line(color_lines, Point(lines[i][0], lines[i][1]), Point(lines[i][2], lines[i][3]), Scalar(0,0,255), 3, 8);
	imshow("lines_bisect",color_lines);
	waitKey(0);
	vector<Vec4i> lines_1;
	HoughLinesP(img_gray, lines_1, 3.0, 2.0*CV_PI/180, 30);
	for(int i = 0; i < lines_1.size(); i++)
		line(color_lines, Point(lines_1[i][0], lines_1[i][1]), Point(lines_1[i][2], lines_1[i][3]), Scalar(255,0,0), 3, 8);
	// imshow("lines_bisect",color_lines);
	// imshow("lines_gray",img_gray);
	// waitKey(0);
}

void Lanes::control_points()
{
	Mat temp(img.rows, img.cols, CV_8UC1, Scalar(0));
	for(int i = img.rows-1; i >= 20; i-=5)
	{
		if(i-20 < 0) break;
		int left = 0,right = 0,c_l = 0, c_r = 0;
		for(int j = i; j > i-20; j--)
		{
			if(j < 0) break;
			for(int k = 0; k < img.cols/2; k++)
			{
				if(bisect.at<uchar>(j,k) > 100) { left += k; c_l++; }
			}
			for(int k = img.cols/2+1; k < img.cols; k++)
			{
				if(bisect.at<uchar>(j,k) > 100) { right += k; c_r++; }
			}
		}
		if(c_l == 0 && c_r == 0) continue;
		if(c_l != 0) left /= c_l;
		if(c_r != 0) right /= c_r;
		if(c_l != 0) temp.at<uchar>(i-10,left) = 255;
		if(c_r != 0) temp.at<uchar>(i-10,right) = 255;
	}
	int flag_l = 0, flag_r = 0, x_l, y_l, x_r, y_r;
	for(int i = img.rows; i > img.rows/3; i--)
	{
		for(int j = 0; j < img.cols/2; j++)
		{
			if(temp.at<uchar>(i,j) != 255) continue;
			if(flag_l == 0) {flag_l = 1; x_l = j; y_l = i; continue; }
			line(img, Point(x_l, y_l), Point(j, i), Scalar(255,0,0), 3, 8);
			x_l = j; y_l = i;
		}
		for(int j = img.cols/2 + 1; j < img.cols; j++)
		{
			if(temp.at<uchar>(i,j) != 255) continue;
			if(flag_r == 0) {flag_r = 1; x_r = j; y_r = i; continue; }
			line(img, Point(x_r, y_r), Point(j, i), Scalar(0,0,255), 3, 8);
			x_r = j; y_r = i;
		}
	}
	// imshow("points",temp);
	namedWindow("lanes",WINDOW_NORMAL);
	imshow("lanes",img);
	// waitKey(0);
}

// void Lanes::control_vanishing()
// {

// }