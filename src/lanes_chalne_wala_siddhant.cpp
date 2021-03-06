#include "../include/lane_detector/lanes_chalne_wala.hpp"

// typedef std::chrono::high_resolution_clock::time_point TimeVar;	
// int main(int argc, char** argv)
// {
// 	cout<<"running"<<endl;
// 	ros::init(argc, argv, "image_converter");
// 	ros::NodeHandle nh_;

// 	// 	if(argc < 2) return 0;
//     VideoCapture cap(argv[1]); // open the default camera
//     if(!cap.isOpened())  // check if we succeeded
//         return -1;
//     // model = svm_load_model("data.txt.model");
// 	int fr = 0;
// 	while(1)
// 	{
// 		// if(fr == 1000) fr = 0;
// 		//Mat given=imread(argv[1],1);
// 		Mat frame;
// 		cap>>frame;
// 		if(!frame.data) cout<<"null";
// 		Lanes L(frame);     // L --> frame
// 		//L.Intensity_distribution();
// 		fr++;
// 		// if(fr%20 != 0) continue;
	

// 		// L.Intensity_adjust();
// 		// L.remove_grass();

// 		L.Mix_Channel();
// 		L.topview();
// 		// L.display();
// 		// L.superpixels();
// 		L.parabola();
// 		// L.topview(1);
// 			// L.Hough();
// 			// L.display();
/// 		// L.Brightest_Pixel_col();
// 		// L.Brightest_Pixel_row();
// 		// L.control_points();
// 		// L.curve_fitting();
// 			// L.control_vanishing();
// 		waitKey(10);
// 	}
// 	// waitKey(0);
// 	return 0;

// }

// void imageCb(const sensor_msgs::ImageConstPtr& msg)
// {
//     Mat img;
// 	cv_bridge::CvImagePtr cv_ptr;
// 	clock_t begin, end;
// 	begin = clock();
//     try
//     {
// 		cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
// 		img = cv_ptr->image;
// 	}
// 	catch (cv_bridge::Exception& e)
// 	{
// 		ROS_ERROR("cv_bridge exception: %s", e.what());
// 		return;
// 	}
// 	if(img.rows < 0) return;
// 	counter++;
// 	if(counter > 1000) counter = 0;
// 	// if(counter % 20 != 0) return;
// 	end = clock();;
// 	// cout<<"\nCV_BRIDGE = "<<(end - begin)/1000.0;
	
// 	begin = clock();
// 	Lanes L(img);
// 	end = clock();
// 	// cout<<"\nconstructor = "<<(end - begin)/1000.0;

// 	// cout<<img.rows<<" img rowa"<<endl;
// 	// L.Intensity_adjust();
// 	begin = clock();
// 	L.Mix_Channel();
// 	end = clock();
// 	// cout<<"\nMix channel = "<<(end - begin)/1000.0;

// 	L.remove_grass();
// 	L.topview();
	
// 	begin = clock();
// 	L.parabola();
// 	end = clock();
// 	// cout<<"\nParabola = "<<(end - begin)/1000.0;

// }


// VideoWriter video("outcpp.avi",CV_FOURCC('M','J','P','G'),10, Size(1920, 1200));

int main(int argc, char** argv)
{

	cout<<"running"<<endl;
	ros::init(argc, argv, "image_converter");
	ros::NodeHandle nh_;

	ros::Publisher waypoint_pub = nh_.advertise<geometry_msgs::PoseStamped>("move_base_simple/goal", 1);
	
	ros::Publisher lanes_pub = nh_.advertise<sensor_msgs::LaserScan>("/lanes", 10);
	image_transport::ImageTransport it_(nh_);
	image_transport::Subscriber image_sub_;
	
	image_sub_ = it_.subscribe("/camera/image_color", 10, &imageCb);


    // model = svm_load_model("data.txt.model");
    ros::Rate r(1);
	// int fr = 0;
	while(1)
	{
		r.sleep();
		ros::spinOnce();
	}

	return 0;
}

void imageCb(const sensor_msgs::ImageConstPtr& msg)
{
    Mat img;
	cv_bridge::CvImagePtr cv_ptr;
	clock_t begin, end;
	// begin = clock();
    try
    {
		cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
		img = cv_ptr->image;
		// resize(img,img,Size(img.rows/2,img.cols/2));
	}
	catch (cv_bridge::Exception& e)
	{
		ROS_ERROR("cv_bridge exception: %s", e.what());
		return;
	}
	if(img.rows < 0) return;
	// video.write(img);
	// end = clock();;
	// cout<<"\nCV_BRIDGE = "<<(end - begin)/1000.0;



	
	// begin = clock();
	Lanes L(img);

	// end = clock();
	// cout<<"\nconstructor = "<<(end - begin)/1000.0;

	// cout<<img.rows<<" img rowa"<<endl;
	// L.Intensity_adjust();
	// begin = clock();
	L.remove_obstacles();
	L.preprocess();
	// L.remove_grass();
	// L.Mix_Channel();
	// end = clock();
	// cout<<"\nMix channel = "<<(end - begin)/1000.0;

	// L.remove_grass();
	L.topview();
	
	// begin = clock();
	L.parabola();
	// end = clock();
	// cout<<"\nParabola = "<<(end - begin)/1000.0;
}


Lanes::Lanes(Mat img)
{
	resize(img, img, Size(960, 600));
	// imshow("img", img);
	// cout<<img.rows<<' '<<img.cols<<endl;
	this->img = img;
	top_view_rgb=img.clone();
	cvtColor(this->img, this->img_gray,CV_BGR2GRAY);
	this->flag_width = 0;
	this->width_lanes = 400;
}

int Lanes::IsValid(Mat A,int x,int y)
{
	if(x<0||y<0||x>=A.cols||y>=A.rows)
		return 0;
	return 1;
}

void Lanes::Mix_Channel()
{
	// blur(img,img,Size(5,5));
	for(int i = 0; i < img.rows; i++)   // grass removal
		for(int j = 0; j < img.cols; j++)
		{
			int temp = 2 * img.at<Vec3b>(i,j)[0] - img.at<Vec3b>(i,j)[1]; 
			if(temp < 0) temp=0;
			if(temp > 255) temp=255;
			img_gray.at<uchar>(i,j) = (unsigned int)temp;
		}
	top_view=img_gray.clone();
	// imshow("Mix_Chann",img_gray);
	// waitKey(0);
}

void Lanes::display()
{
	imshow("input", img);
	imshow("Mix_Channel", img_gray);
	waitKey(1);
}


void Lanes::preprocess()
{
	// Mat pre = img.clone();
 //    for(int i=0; i < pre.rows; i++)
	// {
	// 	for(int j=0; j < pre.cols; j++)
	// 	{
	// 		if(pre.at<Vec3b>(i, j)[0] > 160 && pre.at<Vec3b>(i, j)[1] > 160 && pre.at<Vec3b>(i, j)[2] > 160)
	// 			continue;
	// 		else
	// 		{
	// 			pre.at<Vec3b>(i, j)[0] = 0;
	// 			pre.at<Vec3b>(i, j)[1] = 0;
	// 			pre.at<Vec3b>(i, j)[2] = 0;

	// 		}
	// 	}
	// }

	// medianBlur(pre, pre, 7);
 //    erode(pre, pre, Mat(), Point(-1,-1),3);
 //    medianBlur(pre, pre, 7);
 //    medianBlur(pre, pre, 7);

	// Mat channel[3];
 //    split(pre, channel);

	// Mat B2_G = (2*channel[0]-channel[1]);
	// threshold(B2_G, B2_G, 90, 255, THRESH_BINARY);
 //    Mat B_G = (channel[0]-channel[1]);
 //    threshold(B_G, B_G, 60, 255, THRESH_BINARY);

 //    Mat result = B2_G-B_G;


	// medianBlur(result, result, 7);
 //    erode(result, result, Mat(), Point(-1,-1),3);
 //    medianBlur(result, result, 7);
 //    medianBlur(result, result, 7);
 //    img_gray = result;
 //    // imshow("preproces", result);
 //    // waitKey(3);
	// top_view=img_gray.clone();

	Mat frame = img.clone();
	// remove_obstacles(frame);

	// vid >> frame;
	Mat channels[3];
	split(frame,channels);
	//:	imshow("2B-G",2*channels[0] - channels[1]);
	Mat lab_,labu[3];
	cvtColor(frame,lab_,CV_BGR2Lab);
	// split(lab_,labu);
	int dilation_size = 1;
	Mat element = getStructuringElement(MORPH_CROSS,
	                               Size( 2*dilation_size + 1, 2*dilation_size+1 ),
	                               Point( dilation_size, dilation_size ) );

	// namedWindow("LAB_check",WINDOW_NORMAL);
	// imshow("LAB_check",lab_);
	// waitKey(0);
	Mat lab;
	inRange (lab_,Scalar(220,100,100),Scalar(255,140,140),lab);   
	medianBlur (lab,lab, 7 );
	int tem = 5;
	while(tem--)
		erode(lab,lab, element );
	tem = 5;
	while(tem--)
		dilate(lab,lab,element);
	img_gray = 	lab;
	top_view = img_gray.clone();

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
	// imshow("Intensity_distribution",dist);
	//waitKey(0);
}

void Lanes::Intensity_adjust()  // the top part of frame may have some intensity variation
{
	Mat filter(img.rows,img.cols,CV_8UC1,Scalar(0));
	for(int i = 0; i <= img.rows/4; i++)
		for(int j = 0; j < img.cols; j++)
			filter.at<uchar>(i,j) = (unsigned int )(img.rows/4 - i) * (SUBSTRACTION_CONSTANT) / (img.rows/4);
	// imshow("filter",filter);
	// waitKey(0);
	for(int i = 0; i < img.rows; i++)
		for(int j = 0; j < img.cols; j++)
			for(int k = 0; k < 3; k++)
			{
				int temp = img.at<Vec3b>(i,j)[k] - filter.at<uchar>(i,j);
				if(temp < 0) temp = 0;
				img.at<Vec3b>(i,j)[k] = (unsigned int )temp;
			}
	// imshow("Filtered Image", img);
	//waitKey(0);
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
			if(img_gray.at<uchar>(i,j) < TH_REM_GRASS)
			{

				non_grass.at<Vec3b>(i,j)[0] = 0;
				non_grass.at<Vec3b>(i,j)[1] = 0;
				non_grass.at<Vec3b>(i,j)[2] = 0;
				img_gray.at<uchar>(i,j) = 0;
			}
			else
			{
				for(int k = 0; k < 3; k++)
					if(non_grass.at<Vec3b>(i,j)[k] < TH_REM_GRASS)
					{
						non_grass.at<Vec3b>(i,j)[0] = 0;
						non_grass.at<Vec3b>(i,j)[1] = 0;
						non_grass.at<Vec3b>(i,j)[2] = 0;
						img_gray.at<uchar>(i,j) = 0;
						break;
					}
			}
		}
	img = non_grass;
	// imshow("remove_grass", non_grass);
	// waitKey(0);
}


void Lanes::topview()
{
	top_view_rgb = img.clone();	
	// Mat h = (Mat_<float>(3, 3)<< 3.665649454142774, 5.249642779023947, -2017.634107745852, 0.1725239771632309, 10.74704553239514, -3191.00361122947, 7.864729235797308e-05, 0.006494804637725546, 1);
	Mat h = (Mat_<float>(3, 3)<<0.9524258265572764, 1.932259226972684, 71.36069918907818, 0.08413921815847476, 3.18697351467409, -77.56300977045593, -2.409361384497643e-05, 0.00403298641581695, 1);


	// float scale_factor = 3.4;
	// cout<<top_view.rows<<' '<<top_view.cols<<endl;
	// resize(top_view, top_view, Size(960*2, 1200));
	// resize(top_view_rgb, top_view_rgb, Size(960*2, 1200));

	warpPerspective(top_view, top_view, h, Size(960,600));
	warpPerspective(top_view_rgb, top_view_rgb, h, Size(960,600));
	// resize(top_view, top_view, Size(960, 600));
	// resize(top_view_rgb, top_view_rgb, Size(960, 600));

	imshow("preprocess", top_view);
	waitKey(10);

}
/*
void Lanes::Edge()
{
	Mat Gx, Gy, Ga(img.rows,img.cols,CV_8UC1,Scalar(0)), Gb(img.rows,img.cols,CV_8UC1,Scalar(0)), Gc(img.rows,img.cols,CV_8UC1,Scalar(0)), Gd(img.rows,img.cols,CV_8UC1,Scalar(0));
	// equalizeHist(img_gray, img_gray);     // adjusting the contrast
	Mat temp = img_gray.clone();
	for(int i = 0; i < img.rows; i++)  // pixel
	{
		for(int j = 0; j < img.cols; j++)
		{
			int count=0;
			for(int m = i-1; m < i+2; m++)  // a kernel
			{
				for(int n = j-1; n < j+2; n++)
				{
					if(m<0 || n<0 || m>=img.rows || n>=img.cols) continue;   // isvalid condition
					if(img_gray.at<uchar>(m,n) < INTENSITY_TH) count++;   // erosion condition
				}
			}
			if(count != 0) temp.at<uchar>(i,j)=0;  
		}
	}
	img_gray = temp;
	vector<vector<Point> > contours;
	vector<Vec4i> heirarchy;
	vector<Rect> roi;
	vector<Mat> roi_img;
	Mat t=img_gray.clone();
	
	Mat t1 = img_gray.clone();
	
	cvtColor(t1,t,CV_GRAY2BGR);
	threshold(t1,t1,0,255,THRESH_BINARY+THRESH_OTSU);
	Mat element = getStructuringElement( MORPH_RECT,Size( 2*1 + 1, 2*1+1 ),Point( 1, 1 ) ); 
    int i=5;
    while(i--)
       erode(t1,t1,element);
   	i = 1;
   	// while(i--)
    // dilate(t1,t1,element);
	imshow("thesh",img_gray);
	//Canny(img_gray,img_gray,50,100,3,0);
	findContours(t1,contours,heirarchy,CV_RETR_TREE,CV_CHAIN_APPROX_NONE);
	float resize_factor=1.5;
	for(int i=0;i<contours.size();i++)
	{
		double area=contourArea(contours[i]);
		if(area<100 || area > (0.1*t1.rows*t1.cols)) continue;
		drawContours(t,contours,i,Scalar(255,0,0),2,8,heirarchy,0,Point());
		Rect rect=boundingRect(Mat(contours[i]));
		int width=rect.br().x-rect.tl().x;
		int height=rect.br().y-rect.tl().y;
		Rect roi_latest;
		Point tl=rect.tl(),br=rect.br();
		if((width*1.0)/height>1.5f) {
			tl=rect.tl()-Point(width*0.3*resize_factor,height*1*resize_factor);
			br=rect.br()+Point(width*0.5*resize_factor,height*0.5*resize_factor);
			if(!(tl.x>=0))
				tl.x=0;
			if(!(tl.y>=0))
				tl.y=0;
			if(!(br.x<img.cols))
				br.x=img.cols-1;
			if(!(br.y<img.rows))
				br.y=img.rows-1;
			}
		else {
			// tl=rect.tl()-Point(width*0.3*resize_factor,height*1*resize_factor);
			// br=rect.br()+Point(width*0.5*resize_factor,height*0.5*resize_factor);
			tl=rect.tl();
			br=rect.br();
		}
		rectangle(img,tl,br,Scalar(0,255,0),2,8,0);
		roi_latest=Rect(tl.x,tl.y,br.x-tl.x,br.y-tl.y);
		
		roi.push_back(roi_latest);
		roi_img.push_back(img(roi_latest));

		// imwrite("data/"+to_string(j)+".png",img(roi_latest));
		j++;
	}

	//roi_img now contains all the images in it which are to be predicted for lebelling
	for(int i = 0;i<roi_img.size();i++) {
		Rect rect = roi[i];
		//cout << "Check" << endl;
		Point tl = rect.tl(),br = rect.br();
		rectangle(img,tl,br,Scalar(0,0,0),2,8,0);
	//SVM CODE
	vector< vector < float> > v_descriptorsValues;
 	vector< vector < Point> > v_locations;

 	resize(roi_img[i], roi_img[i], Size(64,128) );

 	HOGDescriptor d( Size(64,128), Size(16,16), Size(8,8), Size(8,8), 9);
  	vector< float> descriptorsValues;
  	vector< Point> locations;
  	d.compute( roi_img[i], descriptorsValues, Size(0,0), Size(0,0), locations);

  	double predict_label;

    //x = (struct svm_node *) malloc(sizeof(struct svm_node));
   	prob.l = 1;
    prob.y = Malloc(double,prob.l); //space for prob.l doubles
	prob.x = Malloc(struct svm_node *, prob.l); //space for prob.l pointers to struct svm_node
	x_space = Malloc(struct svm_node, (3780+1) * prob.l); //memory for pairs of index/value

	int j=0; //counter to traverse x_space[i];
	for (int i=0;i < prob.l; ++i)
	{
		//set i-th element of prob.x to the address of x_space[j]. 
		//elements from x_space[j] to x_space[j+data[i].size] get filled right after next line
		prob.x[i] = &x_space[j];
		for (int k=0; k<3780; ++k, ++j)
		{
			x_space[j].index=k+1; //index of value
			x_space[j].value=descriptorsValues[k]; //value
			// cout<<"x_space["<<j<<"].index = "<<x_space[j].index<<endl;
			// cout<<"x_space["<<j<<"].value = "<<x_space[j].value<<endl;
		}
		x_space[j].index=-1;//state the end of data vector
		x_space[j].value=0;
		// cout<<"x_space["<<j<<"].index = "<<x_space[j].index<<endl;
		// cout<<"x_space["<<j<<"].value = "<<x_space[j].value<<endl;
		j++;

	}


	//set all default parameters for param struct
	param.svm_type = C_SVC;
	param.kernel_type = RBF;
	param.degree = 3;
	param.gamma = 0;	// 1/num_features
	param.coef0 = 0;
	param.nu = 0.5;
	param.cache_size = 100;
	param.C = 1;
	param.eps = 1e-3;
	param.p = 0.1;
	param.shrinking = 1;
	param.probability = 0;
	param.nr_weight = 0;
	param.weight_label = NULL;
	param.weight = NULL;

	double out = svm_predict(model,x_space);

	if(out == 1) {
	rectangle(img_gray,Point(rect.x,rect.y),Point(rect.x + rect.width,rect.y + rect.height),Scalar(0),-1,8,0);
	//cvtColor(img,img_gray,CV_BGR2GRAY);
	}
	}
	rectangle(img_gray,Point(0,img.rows*0.75),Point(img.cols-1,img.rows-1),Scalar(0),-1,8,0);
	line(img,Point(0,img.rows/3),Point(img.cols-1,img.rows/3), Scalar(255,0,0), 2, CV_AA);
	top_view=img_gray.clone();
	
	imshow("erode",img_gray);
	imshow("t",t);
	
}

*/



vector<double> gen_new_way(Mat img, double a, double lam1, double lam2, double w, vector<double> old_waypoint) {

        double shift_x;
        double shift_y;
        double theta;
        double multi = 1;

        cout << "old.x : " << old_waypoint[0] << " old.y " << old_waypoint[1] << endl;
        //temp contains vector<int> of obstacle detected in front view
       	vector<int> obstacle = obstacle_coords(img, old_waypoint);

        //lane check cicle
        //circle(img, Point((float)((1.0*obstacle[1]*obstacle[1])/lam1 + a),img.rows- obstacle[1]),15 ,Scalar(255, 255, 0),-1,1,0);

	vector<double> new_way;

        Point check_point = Point(old_waypoint[0]+DIST_CHECK*cos(old_waypoint[2]), old_waypoint[1]+DIST_CHECK*sin(old_waypoint[2]));
	if(no_lane)
	{
                for (int i = 0; i < 10; i++) cout << "NO LANE" << endl;
		new_way.push_back(old_waypoint[0]+JUMP*cos(old_waypoint[2]));
		new_way.push_back(old_waypoint[1]+JUMP*sin(old_waypoint[2]));

                cout << "current y when no lane " << new_way[1];
                multi = 0;

	}
	else
	{
		if(is_prev_single)
		{

			if(is_prev_single_left)
			{
                                for (int i = 0; i < 10; i++) cout << "LEFT LANE: dist " << fabs(obstacle[0] - ((obstacle[3]*obstacle[3])/lam1 + a)) << endl;
                                //if (fabs(obstacle[0] - ((obstacle[1]*obstacle[1])/lam1 + a)) > MIN_GAP_BETWEEN_OBSTACLE_AND_LANE) {
                                if (fabs(obstacle[0] - ((img.rows*img.rows)/(4*lam1) + a)) > MIN_GAP_BETWEEN_OBSTACLE_AND_LANE) {
                                        multi = 0;

                                        new_way.push_back((obstacle[0] + ((obstacle[1]*obstacle[1])/lam1 + a))/2);
                                }
                                else {
                                        multi = 1;
                                        new_way.push_back(obstacle[2]+NAV_GAP);
                                }
				new_way.push_back(old_waypoint[1]);
                                cout << "current y when single left lane " << new_way[1];
                                circle(img, Point(new_way[0], img.rows-new_way[1]),15 ,Scalar(150, 3, 15),-1,1,0);
			}
			else
                        {       

                                for (int i = 0; i < 10; i++) { cout << "RIGHT LANE: dist " << fabs(obstacle[2] - ((obstacle[3]*obstacle[3])/lam1 + a)) << endl;
                                cout << "obstacle_right.x" << obstacle[2] << endl;
                                cout << "lane_right.x" << ((obstacle[3]*obstacle[3])/lam1 + a) << endl;
                                }
                                //if (fabs(obstacle[2] - ((obstacle[3]*obstacle[3])/lam1 + a)) > MIN_GAP_BETWEEN_OBSTACLE_AND_LANE) {
                                if (fabs(obstacle[2] - ((img.rows*img.rows)/(4*lam1) + a)) > MIN_GAP_BETWEEN_OBSTACLE_AND_LANE) {

                                        cout << "entering from between" << endl;
                                        cout << "entering from between" << endl;
                                        cout << "entering from between" << endl;
                                        cout << "entering from between" << endl;
                                        cout << "entering from between" << endl;
                                        multi = 0;
                                                new_way.push_back((obstacle[2]+((obstacle[3]*obstacle[3])/lam1 + a))/2);
                                                }

                                else {
                                        multi = -1;
                                        new_way.push_back(obstacle[0]-NAV_GAP);
                                }
				new_way.push_back(old_waypoint[1]);
                                cout << "current y when single right lane " << new_way[1];
                                circle(img, Point(new_way[0], img.rows - new_way[1]),15 ,Scalar(150, 3, 15),-1,1,0);
			}
		}
		else
		{
                        multi = 0;
                        for (int i = 0; i < 10; i++) cout << "BOTH LANE: left? " << (fabs(obstacle[0]-((obstacle[1]*obstacle[1]/lam1) + a)) < fabs(obstacle[2]-((obstacle[3]*obstacle[3])/lam1 + a))) << endl;

			//if(fabs(obstacle[0]-(((obstacle[1]*obstacle[1])/lam1) + a))<fabs(obstacle[2]-((obstacle[3]*obstacle[3])/lam1 + a)))//obstacle on left side
			if(fabs(obstacle[0]-(((img.rows*img.rows)/(4*lam1)) + a))<fabs(obstacle[2]-((img.rows*img.rows)/(4*lam1) + a)))//obstacle on left side
				new_way.push_back(obstacle[2]+NAV_GAP);
			else//obstacle on right side
				new_way.push_back(obstacle[0]-NAV_GAP);
                        new_way.push_back(old_waypoint[1]);
                        cout << "current y when double lane " << new_way[1];
		}
	}

        circle(img, Point((float)((1.0*img.rows*img.rows)/(4*lam1) + a),img.rows/2),15 ,Scalar(255, 255, 0),-1,1,0);
        
        /*
        shift_x = new_way[0] - old_waypoint[0];
        shift_y = (obstacle[1] - old_waypoint[1])/2;

        theta = multi * fabs(atan(shift_y/shift_x)) + old_waypoint[2];
        */
        theta =  PI/4 + (old_waypoint[2]/2);
	new_way.push_back(theta);
        line(img, Point(new_way[0],img.rows- new_way[1]), Point(new_way[0] + 150*cos(theta),img.rows- new_way[1] - 150*sin(theta)) , Scalar(255, 255, 0), 4, 8);
	return new_way;
}

bool is_bottom_at_left (Mat img, float a) {
        if (a < img.cols/2) return true;
        else return false;
}

vector<double> gen_way(Mat img, float a, float lam1, float lam2, float w)
{
	float range = 2;
	float offset = 0;
	if(a >= img.cols/2 && (a + w) >= img.cols/2)
		w = 10001;
	if(a < img.cols/2 && (a + w) < img.cols/2)
		w = 10002;
	vector<double> way;
	is_lane_single=abs(w)>img.cols;

        //single lane
	if(is_lane_single)
	{
		Point center_single = centroid(img,a,lam1);
		//check left or right if it is first frame
		if(first_frame)
		{
			first_frame=false;
			if(is_bottom_at_left(img, a))
				is_lane_left=true;
			else
				is_lane_left=false;

		}
		//check left or right if prev is single
		else if(is_prev_single)
		{
			if(is_prev_single_left)
				is_lane_left=true;	
			else
				is_lane_left=false;
		}
		//check left or right if prev frame had 2 lanes
		else
		{
			if(abs(left_prev_x-center_single.x)<abs(right_prev_x-center_single.x))
				is_lane_left=true;
			else
				is_lane_left=false;	
		}

		is_prev_single=true;
		is_prev_single_left=is_lane_left;

		//calculate waypoint if right lane
		if(!is_lane_left)
		{
			cout << "only right lane visible\n";

			float grad = heading(img, a, lam1); //	lam1/((range - offset)*2*PPM);
                        float y, x;
                        if (grad > PI/4 && grad < (3*PI)/4) {
                                y = 2*PPM;
                        }
                        else {
                                y = 100;
                        }
		
			x = pow((y/PPM - offset)*PPM, 2)/lam1 + a - wide/2;
			// way.push_back(x);			
			// way.push_back(y); 
			way.push_back(x);			
			way.push_back(y); 

			//grad = atan(grad);
			// grad = grad < 0 ? PI + grad : grad;
			// grad = grad < 0 ? grad + 3.14/12 : grad - 3.14/12;
			way.push_back(grad);
			return way;
		}
		//calculate waypoint if left lane
		else
		{
				cout<<"only left lane visible\n";
		
                                float y = 100;
				float grad = heading(img, a, lam1);//	lam1/((range - offset)*2*PPM);

                                if (grad > PI/4 && grad < (3*PI)/4) {
                                        y = 2*PPM;
                                }
				float x = pow((y/PPM - offset)*PPM, 2)/lam1 + a + wide/2;
				// way.push_back(x);
				// way.push_back(y);
				way.push_back(x);			
				way.push_back(y); 

				// grad = atan(grad);
				// grad = grad < 0 ? PI + grad : grad;
				// grad = grad < 0 ? grad + 3.14/12 : grad - 3.14/12;
				way.push_back(grad);
				return way;

		}

	}

	//two lanes
	else
	{
        cout<<"both lane visible\n";
		
                first_frame=false;
		float temp = pow((range - offset)*PPM, 2)/lam1 + a;
		float temp2 = pow((range - offset)*PPM, 2)/lam2 + a + w;
		way.push_back(temp/2 + temp2/2);
		way.push_back(range*PPM);
		float grad1 = heading(img, a, lam1); //lam1/((range - offset)*2*PPM);
		float grad2 = heading(img, a+w, lam2); //lam2/((range - offset)*2*PPM);
		//grad1 = atan(grad1);
		//grad2 = atan(grad2);
		// grad1 = grad1 < 0 ? PI + grad1 : grad1;
		// grad2 = grad2 < 0 ? PI + grad2 : grad2;
		// grad1 = grad1 < 0 ? grad1 + 3.14/12 : grad1 - 3.14/12;
		// grad2 = grad2 < 0 ? grad2 + 3.14/12 : grad2 - 3.14/12;
		is_prev_single=false;
		way.push_back((grad1 + grad2)/2);
		int lane1_centroid_x=centroid(img,a,lam1).x, lane2_centroid_x=centroid(img,a+w,lam2).x;
		if(lane1_centroid_x<lane2_centroid_x)
		{
			left_prev_x=lane1_centroid_x;
			right_prev_x=lane2_centroid_x;
		}
		else
		{
			left_prev_x=lane2_centroid_x;
			right_prev_x=lane1_centroid_x;		
		}
		return way;
	}
}




void Lanes::parabola()
{
	// imshow("normal_top_view_grey", top_view);
	// imshow("top_view_rgb", top_view_rgb);
	// waitKey(5);
	// cvtColor(top_view_rgb, top_view, CV_BGR2GRAY);
	Mat temp(top_view.rows, top_view.cols, CV_8UC3, Scalar(0,0,0));
	Mat mario(top_view.rows, top_view.cols, CV_8UC1, Scalar(0));  

	for(int i = 0; i < top_view.rows; i++)
		for(int j = 0; j < top_view.cols; j++)
			if(top_view.at<uchar>(i,j) > TH_DOT)
				for(int k = 0; k < 3; k++)
					temp.at<Vec3b>(i,j)[k] = 255;

	Mat element = getStructuringElement(MORPH_RECT,
                                       Size( 2*1 + 1, 2*1+1 ),
                                       Point(1,1 ));
    dilate(temp, temp, element);


	// imshow("temp", temp);
	// waitKey(5);
	// IplImage im = temp;
	// IplImage* image = &im;
	// image = cvCreateImage(cvSize(top_view.cols, top_view.rows), 8, 3);

	/*IplImage *lab_image = cvCloneImage(image);
    cvCvtColor(image, lab_image, CV_BGR2Lab);*/
    int w = temp.rows, h = temp.cols;
    int nr_superpixels = 600;
    int nc = 100;

    // double step = sqrt((w * h) / (double) nr_superpixels);
   

    /* Perform the SLIC superpixel algorithm. */
    // Slic slic;
    // slic.generate_superpixels(lab_image, step, nc);
    // slic.create_connectivity(lab_image);
    
    // end = clock();
    // cout<<"\nSlic = "<<(end - start)/1000.0;
    // /* Display the contours and show the result. */
    // slic.display_contours(image, CV_RGB(255,0,0));
    // cvShowImage("slic", image);
    // imshow("top", top_view);
    // waitKey(2);

    /* gpu accelerated super pixel clustering */
	// clock_t start, end;
 //    start = clock();
	
	resize(temp,temp,Size(temp.cols/2,temp.rows/2));

	cvtColor(temp, temp, CV_BGR2BGRA);
	// imshow("image original in top view", temp);
	// cudaFree();
	// waitKey(1);
	FastImgSeg segmenter;
	segmenter.initializeFastSeg(temp.cols, temp.rows, nr_superpixels);
	segmenter.LoadImg(temp.data);
	segmenter.DoSegmentation(RGB_SLIC, nc);
	segmenter.Tool_GetMarkedImg();
	Mat marked(temp.rows, temp.cols, CV_8UC4, segmenter.markedImg);
	Mat mask(temp.rows, temp.cols, CV_32SC1, segmenter.segMask);
	 imshow("img super pixel cluster marked for threshold settings", marked);
	 waitKey(5);
	// end = clock();
	// cout<<"segmented\n";
	// segmenter.~FastImgSeg();

	Mat slic_img = temp.clone();//cvarrToMat(image);
    // imshow("image", img);
    // waitKey(10);

	int max = 0;
	for(int i = 0; i < mask.rows; i++)
		for(int j = 0; j < mask.cols; j++)
			if((int)mask.at<int>(i, j) > max) max = (int)mask.at<int>(i, j);
	// cout<<max;




    /*	the array count_pix stores the number of pixels in each superpixel  */
    /*	the array count_col stores the number of pixels in top_view that pass the colour threshold     */
    int* count_pix = new int[max + 1];
    int* count_col = new int[max + 1];
    int* count_x = new int[max + 1];
    int* count_y = new int[max + 1];

    /*	the control points will be pushed in P	*/
    vector<Point> P;

    /*	the control points will be printed on Mat dot	*/
    Mat dot(top_view.rows, top_view.cols, CV_8UC1, Scalar(0));
    Mat new_dot(top_view.rows, top_view.cols, CV_8UC1, Scalar(0));  


    /*	initialising count_pix, count_col, count_x and count_y to 0   */
    for(int i = 0; i <= max; i++)
    count_pix[i] = count_col[i] = count_x[i] = count_y[i] = 0;


    /*	calculating the number of pixels of pixels in each superpixel and the number ofpixels that pass the colour threshold	*/
    for(int i = 0; i < top_view.rows; i++)
    {
	for(int j = 0; j < top_view.cols; j++)
	{
	    if(i/2 >= mask.rows || j/2 >= mask.cols) continue;
	    int idx = mask.at<int>(i/2,j/2);
	    count_pix[idx]++;
	    count_x[idx] += j;
	    count_y[idx] += i;
	    if(top_view.at<uchar>(i, j) > TH_DOT) count_col[idx]++;

	    //cout << "count_pix" << count_pix[idx] << endl;
	}
    }

    /*
    for (int i = 0; i <= max; i++) {
	if (count_x[i] == 0 || count_y[i] == 0) continue;
	count_x[i] /= count_pix[i];
	count_y[i] /= count_pix[i];
	if(count_pix[i] < TH_SMALL_SUPERPIXEL) continue;
	if(count_col[i]/(count_pix[i]*1.0) < TH_MIN_WHITE_REGION) continue;
	dot.at<uchar>(count_y[i], count_x[i]) = 255;
	P.push_back(Point(count_x[i], mario.rows-count_y[i]));
	//cout << "Point P.x" << P.back().x << " y " << P.back().y << endl;
    }
    */



    /*
	mark all the points of those superpixels on mario which:
	a) are part of a big superpixel
	b) have a proper ratio of good points to total points
    */

    vector<Point> Q;
    for(int i = 0; i < top_view.rows; i++)
    {
	for(int j = 0; j < top_view.cols; j++)
	{
	    int idx = mask.at<int>(i/2,j/2);
	    if(count_pix[idx] < TH_SMALL_SUPERPIXEL) continue;
	    if(count_pix[idx] == 0) continue;
	    if(count_col[idx]/(count_pix[idx]*1.0) < TH_MIN_WHITE_REGION) continue;
	    mario.at<uchar>(i, j) = 255;
	    Q.push_back(Point(j,mario.rows -i));

	}
    }


    /*	K is the kernel size and 
     	dist_for_inlier is min(|dist in x from lane|, |dist in y|) for which a point is to be considered an inlier	*/


    for (int i = 0; i < no_of_random_points_for_dot; i++) {
	// cout << "Q " << Q[i] << endl;
	int temp =  abs(random()%Q.size());
	P.push_back(Q[temp]);
	// cout << "P " << P.back() << endl;
	dot.at<uchar>(mario.rows-Q[temp].y, Q[temp].x) = 255;
    }
	
    //int white_count = 0;

    /*
    for (int w = i-K/2; w < i+1+K/2; w++) {
	    for (int e = j-K/2; e < j+1+K/2; e++) {
		    if (w <0 || w >= mario.rows || e <0 || e>= mario.cols) continue;
		    if (mario.at<uchar>(w,e) == 255) {
			    white_count++;
		    }
	    }
    }
    if (white_count > 1) {
	    dot.at<uchar>(i,j) = 255;
	    P.push_back(Point(j, mario.rows-i));
    }
    */
	

        imshow("mario", mario);

	/*
	for(int i = 0; i < top_view.rows; i++)
	{
		for(int j = 0; j < top_view.cols; j++)
		{
			// if(!i%10 && !j%10)
			// cout<<i<<' '<<j<<endl;
			// if(i/2 >= mask.rows || j/2 >= mask.cols) continue;
			
			int idx = mask.at<int>(i/2,j/2);
			//cout<<"Idx : "<<idx<<endl;
			if(count_pix[idx] < TH_SMALL_SUPERPIXEL) continue;
			// cout<<(count_col[idx]/(count_pix[idx]*1.0))<<endl;
			if(count_pix[idx] == 0) continue;
			if(count_col[idx]/(count_pix[idx]*1.0) < TH_MIN_WHITE_REGION) continue;
			// circle(dot, Point(j, i),3 ,Scalar(255),-1,1,0);
			dot.at<uchar>(i, j) = 255;
			int x = j;
			int y = top_view.rows - i;
			P.push_back(Point(x, y));
			 count_pix[idx] = 0;
		}
		// cout<<i<<endl;
	}
	*/
	cout<<"P size = "<<P.size()<<endl;

	// for(int i = 0; i < P.size(); i++)
	// 	cout<<P[i].x<<' '<<P[i].y<<endl;
	imshow("dotted_condom", dot);
	waitKey(5);

	int flag_no_lane = 0;

	vector<double> way(3);
	if(first_f)
	{
		way[0]=top_view_rgb.cols/2;
		way[1]=1*PPM;
		way[2]=PI/2;
		first_f=false;
		old_waypoint=way;
	}


	float a_gl = 1, lam_gl = 1, lam2_gl = 1, w_gl = 1;
	// int flag_mario = 0;
	if(P.size() < 6)
	{
		cout<<"\nNot enough points";
        
		//when no lane
        w_gl = 10000;

        first_frame = true;
        //vector<double> waypoint;
        way = old_waypoint;
        flag_no_lane = 1;
	}
	// imshow("undotted", slic_img);
	
	int p1_g, p2_g, p3_g, p4_g;
	int score_gl = 0;/* comm_count_gl = 0;*/
    int score_l_gl = 0, score_r_gl = 0; 
	for(int i = 0; i < NUM_ITER && !flag_no_lane; i++)
	{
		int p1 = random()%P.size(), p2 = random()%P.size(), p3 = random()%P.size(), p4 = random()%P.size();
		if(p2 == p1) p2 = random()%P.size();
		if(p3 == p1 || p3 == p2) p3 = random()%P.size();
		Point ran_points[4];
		ran_points[0] = P[p1];
		ran_points[1] = P[p2];
		ran_points[2] = P[p3];
		ran_points[3] = P[p4];
		int flag = 0;
		// for(int j = 0; j < 3; j++)
		// 	for(int k = j+1; k < 4; k++)
		// 		if(norm(ran_points[j] - ran_points[k]) < 20)
		// 		{
		// 			flag = 1;
		// 			break;
		// 		}
		// if(flag)
		// {
		// 	i--;
		// 	continue;
		// }
		Point temp;
		for(int m = 0; m < 3; m++)
                {
			for(int n = 0; n < 3 - m; n++)
			{
				if(ran_points[n].x > ran_points[n+1].x) 
				{
					temp = ran_points[n];
					ran_points[n] = ran_points[n+1];
					ran_points[n+1] = temp;
				}
			}	
		}
		// cout<<i<<endl;
		// if(ran_points[2].x - ran_points[1].x < 100)
		// {
		// 	i--;
		// 	continue;
		// }
		// if(abs(ran_points[1].y - ran_points[0].y) < 100)
		// {
		// 	i--;
		// 	continue;
		// }
		// if(abs(ran_points[2].y - ran_points[3].y) < 100)
		// {
		// 	i--;
		// 	continue;
		// }

		int score_loc = 0;/*, comm_count = 0;*/
                int score_l_loc = 0, score_r_loc = 0;
		float* param = parabola_params(ran_points);
		float a = param[0], lam = param[1], lam2 = param[2], w = param[3];
		if(isnan(w)) w = 15000;
		// cout<<"sss "<<w<<endl;
		if(!IsAllowed(lam, a, lam2, w, top_view.rows, top_view.cols)) continue;
		if(!flag_width)
		{
			cout<<"%%%%%%"<<w<<endl;
			width_lanes=w;
			flag_width=1;
		} 
		if(flag_width&&abs(w)<5000)
			width_lanes=width_lanes*0.8 + w*0.2;
		
		// for(int p = 0; p < P.size(); p++)
		// {
		// 	int flag = 0;
  //                       int flag_l = 0;
		// 	for(int x = 0; x < top_view.cols; x++)
		// 	{
		// 		int y_l = sqrt(lam*(x - a)), y_r = sqrt(lam2*(x - a - w));
		// 		// y_l = top_view.rows - y_l;
		// 		// y_r = top_view.rows - y_r;
		// 		if(y_l < 0 && y_r < 0) continue;
		// 		float dist_l = sqrt(pow(x - P[p].x, 2) + pow(y_l - P[p].y, 2));
		// 		if(dist_l < 50)
		// 		{
		// 			flag = 1;
  //                                       flag_l = 1;
		// 		}
		// 		float dist_r = sqrt(pow(x - P[p].x, 2) + pow(y_r - P[p].y, 2));
		// 		if(dist_r < 50)
		// 		{
		// 			if(!flag) flag = 1;
		// 			else
		// 			{
		// 				flag = 0;
		// 				// comm_count++;
		// 			}
		// 		}
		// 	}
		// 	if(flag) {
  //                               score_loc++;
  //                               if (flag_l) {
  //                                       score_l_loc++;
  //                               }
  //                               else {
  //                                       score_r_loc++;
  //                               }
  //                       } 

		// 	// cout<<score_loc<<endl;
		// }

		for(int p = 0; p < P.size(); p++)
		{
			int flag = 0;
            int flag_l = 0;


			float dist_lx = fabs((P[p].y*P[p].y)/lam + a - P[p].x);
			float dist_ly = fabs(sqrt(lam*(P[p].x-a)) - P[p].y);
			if(dist_lx < dist_for_inlier || dist_ly < dist_for_inlier)
			{
				flag = 1;
                                    flag_l = 1;
			}
			float dist_rx = fabs((P[p].y*P[p].y)/lam2 + a + w - P[p].x);
			float dist_ry = fabs(sqrt(lam2*(P[p].x-a-w)) - P[p].y);
			//float dist_r = sqrt(pow(x - P[p].x, 2) + pow(y_r - P[p].y, 2));
			if(dist_rx < dist_for_inlier || dist_ry < dist_for_inlier)
			{
				if(!flag) flag = 1;
				else
				{
					flag = 0;
					// comm_count++;
				}
			}



			if(flag) {
                score_loc++;
                if (flag_l) {
                        score_l_loc++;
                }
                else {
                        score_r_loc++;
                }
            } 

			// cout<<score_loc<<endl;
		}
		if(score_loc>score_gl)
		{
			if(w < 100 && w > -100) continue;
			score_gl = score_loc;
                        score_l_gl = score_l_loc;
                        score_r_gl = score_r_loc;
			a_gl = a;
			lam_gl = lam;
			lam2_gl = lam2;
			w_gl = w;
			p1_g = p1;
			p2_g = p2;
			p3_g = p3;
			p4_g = p4;
			cout<<score_gl<<'\t';
			// comm_count_gl = comm_count;
		}
	}

        /*
        static int frame_count = 0;

        if (frame_count % 9 == 0) frame_count = 0;

        track temp;
        temp.a = a_gl;
        temp.w = w_gl;
        temp.lam1 = lam_gl;
        temp.lam2 = lam2_gl;

        if (median.size() == 8) {
                median.pop_front();
                median.push_back(temp);
        }
        else {
                median.push_back(temp);
        }
        frame_count++;


        if (median[0].a > median[1].a && median[0].a > median[2].a) {
                if (median[1].a > median[2]) {
                        average.push_back(median[1]);
                }
                else {
                        average.push_back(median[2]);
                }
        }
        else {
                if (median[0].a > median[1].a && median[0].a < median[2].a) {
                        average.push_back(median[0]);
                }

                else if (median[0].a < median[1].a && median[0].a > median[2].a) {
                        average.push_back(median[0]);
                }
                else {
                        if (median[1].a < median[2]) {
                                average.push_back(median[1]);
                        }
                        else {
                                average.push_back(median[2]);
                        }

                }
        }


        track temp;
        temp.a = a_gl;
        temp.w = w_gl;
        temp.lam1 = lam_gl;
        temp.lam2 = lam2_gl;
        track result;
        result.a=0;
        result.lam1=0;
        result.lam2=0;
        result.w=0;
        median[frames%3]=temp;
		if(frames%3==0)
		{
			if (median[0].a > median[1].a && median[0].a > median[2].a) {
                if (median[1].a > median[2]) {
                        average[(frames/3)%3]=(median[1]);
                }
                else {
                        average[(frames/3)%3]=(median[2]);
                }
        	}
        	else {
                if (median[0].a > median[1].a && median[0].a < median[2].a) {
                        average[(frames/3)%3]=(median[0]);
                }

                else if (median[0].a < median[1].a && median[0].a > median[2].a) {
                        average[(frames/3)%3]=(median[0]);
                }
                else {
                        if (median[1].a < median[2]) {
                                average[(frames/3)%3]=(median[1]);
                        }
                        else {
                                average[(frames/3)%3]=(median[2]);
                        }

                	}
        		}
		}
		if(frames%9==0)
		{

			for(int i=0; i<3; i++)
			{
				result.a+=average[i].a;
				result.w+=average[i].w;
				result.lam1+=average[i].lam1;
				result.lam2+=average[i].lam2;
			}

		}
                */



		//when no lane
		no_lane=score_gl < THRESHOLD_FOR_ANY_LANE;
        if (no_lane)  {
                w_gl = 10000;

                first_frame = true;
                vector<double> waypoint;
                way.push_back(top_view_rgb.cols/2);
                way.push_back(1*PPM);
                way.push_back(PI/2);
                flag_no_lane = 1; 
        }

        //when one lane only
        // else if (score_l_gl < LANE_THRESHOLD || score_r_gl < LANE_THRESHOLD) {
        //         w_gl = 10000;

        //         first_frame = true;
        //         vector<double> waypoint;
        //         waypoint.push_back(top_view_rgb.cols/2);
        //         waypoint.push_back(1*PPM);
        //         waypoint.push_back(PI/2);
        //         flag_no_lane = 1;
        // }

	cout<<"w = "<<w_gl<<" a = "<<a_gl<<" lam = "<<lam_gl<<"lam2 = "<<lam2_gl<<endl;
	float params[]={lam_gl,a_gl,lam2_gl,w_gl};

	for(int x = 0; x < top_view.cols && !flag_no_lane; x++)
	{
		int y_l = sqrt(lam_gl*(x - a_gl));
		int y_r = sqrt(lam2_gl*(x - a_gl - w_gl));
		
		circle(dot, Point(x, top_view_rgb.rows - y_l),3,Scalar(255),-1,8,0); // dot is a single channel image
		circle(dot, Point(x, top_view_rgb.rows - y_r),3,Scalar(255),-1,8,0);
		circle(new_dot, Point(x, top_view_rgb.rows - y_l),3,Scalar(255),-1,14,0); // dot is a single channel image
		circle(new_dot, Point(x, top_view_rgb.rows - y_r),3,Scalar(255),-1,14,0);
	}

	// imshow("top_view_rgb", top_view_rgb);
	// imshow("sampled_points", dot);
	// waitKey(2);
    if (!flag_no_lane) {
		circle(top_view_rgb, Point(P[p1_g].x, top_view.rows - P[p1_g].y),10,Scalar(0,0,255),-1,8,0);
		circle(top_view_rgb, Point(P[p2_g].x, top_view.rows - P[p2_g].y),10,Scalar(0,0,255),-1,8,0);
		circle(top_view_rgb, Point(P[p3_g].x, top_view.rows - P[p3_g].y),10,Scalar(0,0,255),-1,8,0);
		circle(top_view_rgb, Point(P[p4_g].x, top_view.rows - P[p4_g].y),10,Scalar(0,0,255),-1,8,0);
        // way = gen_way(top_view_rgb, a_gl, lam_gl, lam2_gl, w_gl);
		way=waypoint_objectivefunc_l(params,old_waypoint,top_view_rgb);
		// old_waypoint = way;
    }
	// way.push_back(0.1);
	// way.push_back(0.1);
	// way.push_back(0.1);
	cout<<"waypoint : "<<way[0]<<' '<<way[1]<<endl;
	cout<<"dim : "<<top_view.rows<<' '<<top_view.cols<<endl;
	//arrowedLine(top_view_rgb, Point(way[0], top_view_rgb.rows - way[1]), Point(way[0] + 100*cos(way[2]), top_view_rgb.rows - way[1] - 100*sin(way[2])), Scalar(0, 0, 255), 3);
	//arrowedLine(dot, Point(way[0], top_view_rgb.rows - way[1]), Point(way[0] + 100*cos(way[2]), top_view_rgb.rows - way[1] - 100*sin(way[2])), Scalar(255), 3);

	 circle(top_view_rgb, Point(way[0], dot.rows - way[1]),10,Scalar(255, 0, 0),-1,8,0);
	 circle(dot, Point(way[0], dot.rows - way[1]),10,Scalar(255),-1,8,0);


    //set up transformtransf
    // tf::StampedTransform transform;
    // tf::TransformListener listener;
//*******************************************************
    //****************************************************
    // try
    // {
    //   listener.lookupTransform("base_link", "odom",  
    //                            ros::Time::now(), transform);
    // }
    // catch (tf::TransformException ex){
    //   ROS_ERROR("%s",ex.what());
    //   ros::Duration(1.0).sleep();
    // }

	geometry_msgs::PoseStamped waypoint;
	//geometry_msgs::PoseStamped waypoint_out;

	// waypoint.header.frame_id = "laser";
	// waypoint.pose.position.x = (way[1]*3.0/(top_view.rows)) + transform.getOrigin().x();
	// waypoint.pose.position.y = (-1 * way[0]*3.0/top_view.rows) + transform.getOrigin().y();
	// waypoint.pose.position.z = 0 + transform.getOrigin().z();
	// float theta = way[2] + tf::getYaw(transform.getRotation());

    if(waypoint_on_obstacle(top_view_rgb, way))
    {
		counter = -1*frames_to_skip_when_obstacle_detected;
		//way=gen_new_way(top_view_rgb, a_gl, lam_gl, lam2_gl, w_gl, way);
		vector<double> old_way=way;
		// way=gen_new_way(top_view_rgb, a_gl, lam_gl, lam2_gl, w_gl, way);
		vector<int> obstacle = obstacle_coords(top_view_rgb, old_way);
		way=waypoint_objectivefunc(params,obstacle,old_way,top_view_rgb);
		// old_waypoint = way;
    }
	imshow("top view rgb with dot showing waypoint", top_view_rgb);
	imshow("top view grey for laser scan", new_dot);
	waitKey(2);

	// geometry_msgs::PoseStamped waypoint;
	waypoint.pose.position.x = way[1]/PPM /*+ transform.getOrigin().x()*/;
	waypoint.pose.position.y = -1*(way[0] - top_view.cols/2)/PPM/* + transform.getOrigin().y()*/;
	waypoint.pose.position.z = 0 ;//+ transform.getOrigin().z();
	float theta = (way[2] - PI/2) /*+ atan2(transform.getOrigin().y(),transform.getOrigin().x())*/;
	cout<<"theta = "<<way[2]<<endl;

	tf::Quaternion frame_qt = tf::createQuaternionFromYaw(theta);
	waypoint.pose.orientation.x = frame_qt.x();
	waypoint.pose.orientation.y = frame_qt.y();
	waypoint.pose.orientation.z = frame_qt.z();
	waypoint.pose.orientation.w = frame_qt.w();


	//make the laser->odom transform
	//listener.transformPoint("odom",waypoint,waypoint_out);



	// tf::Quaternion frame_qt = tf::createQuaternionFromYaw(theta);
	// waypoint.pose.orientation.x = frame_qt.x();
	// waypoint.pose.orientation.y = frame_qt.y();
	// waypoint.pose.orientation.z = frame_qt.z();
	// waypoint.pose.orientation.w = frame_qt.w();
	
	counter++;
	if(counter > 1000)
		counter = 0;

	// if (abs(w) > top_view_rgb.cols)
	// 	if (counter % 2 != 0) return;
	// 	else;

	// else if(counter % 8 != 0) return;



	waypoint.header.frame_id = "base_link";
	ros::NodeHandle nh_;


	ros::Publisher waypoint_pub = nh_.advertise<geometry_msgs::PoseStamped>("/move_base_simple/goal/", 10);
	ros::Publisher lanes_pub = nh_.advertise<sensor_msgs::LaserScan>("/lanes", 1);


	LaserScan scan = imageConvert(mario);
	lanes_pub.publish(scan);

	waypoint_pub.publish(waypoint);
	cout<<"Published\n";
	// imshow("Result showing dot image", dot);
	// imshow("Res2", top_view_rgb);
	// waitKey(10);
}

float* Lanes::parabola_params(Point *points)   //mode: 1-6 => llr,lrl,rll,lrr,rlr,rrl
{
	Point p1,p2,p3,p4,temp;
	float *param;
	param = new float [4];
	
	// float dist1,dist2;
	// dist1 = sqrt(pow(p1.x-p2.x , 2) + pow(p1.y-p2.y, 2));
	// dist2 = sqrt(pow(p4.x-p3.x , 2) + pow(p4.y-p3.y, 2));

	// if(dist1 < 0 || dist2 < 0){
	// 	param[0]=param[1]=param[2]=param[3]=0;
	// 	return param;
	// } 


	p1 = points[0];
	p2 = points[1];
	p3 = points[2];
	p4 = points[3];

	// cout<<p1.x<<"  "<<p1.y<<"  "<<p2.x<<"  "<<p2.y<<"  "<<p3.x<<"  "<<p3.y<<"  "<<p4.x<<"  "<<p4.y<<"  "<<endl;

	param[0] = (pow(p1.y,2) * p2.x - pow(p2.y,2)*p1.x)/((pow(p1.y,2)*1.0 - pow(p2.y,2)*1.0)) ; // a
	param[1] = pow(p1.y,2)/((p1.x*1.0 - param[0]*1.0));                                        // lambda1
	param[3] = (pow(p3.y,2) * p4.x - pow(p4.y,2)*p3.x - param[0] * (pow(p3.y,2) - pow(p4.y,2)))/(pow(p3.y,2) - pow(p4.y,2))*1.0; // w
	if(param[3] > top_view.cols || param[3] < -1*top_view.cols) param[3] = 256;
	param[2] = pow(p3.y,2)/(p3.x - param[0] - param[3])*1.0;                                        // lambda2
	return param;

}

bool IsAllowed(float lam1,float a,float lam2,float w,int rows,int cols)
{
	if(fabs(w)>200000)return 1;	//Single lane
	else if(abs(w)<100)return 0;	//w is very small
	else
	{
		if(fabs(lam1)>500||fabs(lam2)>500)	//Non horizontal lines
		{
			for(int i=0;i<rows;i++)
			{
				int x1=((i*i)/lam1)+a;
				int x2=((i*i)/lam2)+a+w;
				if(abs(x1-x2)<100)return 0;
			}
		}
		else
		{
			for(int i=0;i<cols;i++)
			{
				int y1=sqrt(lam1*(i-a));
				int y2=sqrt(lam2*(i-a-w));
				if(abs(y1-y2)<100)return 0;
			}
		}
	}
	return 1;
}


sensor_msgs::LaserScan imageConvert(Mat image) 
{  
	sensor_msgs::LaserScan scan;
	scan.angle_min = -2.36;
	scan.angle_max = 2.36;
	scan.angle_increment = 0.004;
	scan.header.stamp = ros::Time::now();
	scan.header.frame_id = "laser";
	scan.range_min=0;
	scan.range_max=30;
	scan.time_increment=(float)(0.025/1181);
	scan.scan_time=0.025;
	scan.ranges.resize(1181);

	int centre = image.cols/2 + .3*PPM;
	
	for(int t=0;t<=1180;t++)
	{
		scan.ranges[t] = numeric_limits<float>::infinity();  //maximum range value
	}
	for(int i=0;i<image.rows;i++)
	for(int j=0;j<image.cols;j++)
	{
	  // ROS_INFO("what");
	  float angle,dist;
	  float x,y;

	  if(image.at<uchar>(i,j)>128)
	  {

	    x=(float)(j-centre)/(1.0*PPM);
	    y=(float)((image.rows-i)+ 0*PPM)/PPM;
	    
	    if(y!=0)
	      angle = (-1.0)*atan((1.0)*x/y);
	    else 
	      { 
	        if(x>0) angle=-1.57;
	        else angle=(1.57);
	      }

	    dist=sqrt(x*x+y*y);
	    int index=(int)((angle-scan.angle_min)/scan.angle_increment);
	     if(scan.ranges[index]>dist)
	      scan.ranges[index]=dist;
	  }
	}

	return scan;
}


double heading (Mat img, double a, double lam) {
        double x, y, theta = 0, temp;
        int count = 0;
        for (int i = img.rows-2; i >= 0; i--) {
                
                y = (img.rows-1)-i;
                x = (y*y)/lam + a;
                if (x < 0 || x >= img.cols || y < 0 || y >= img.rows) continue;
                temp = atan(lam/(2*y));
                if (temp < 0) {
                        temp += PI;
                }
                theta += temp;
                count++;
        }
        theta /= count;

        return theta;
}

void Lanes::joiner_of_white_spaces_in_obstacles(Mat img) {

        int max = 120;

        for (int j = 0; j < img.cols ;j++) {
                int flag = 0;
                int count = 0;
                int index = 0;
                for (int i = 0; i < img.rows-1; i++) {

                        if (flag == 1 && img.at<uchar>(i,j) == 0) {
                                count++;
                        }
                        if (flag == 1 && img.at<uchar>(i,j) == 255) {
                                if (count < max) {
                                        for (int i = index; i < index+count; i++) {
                                                img.at<uchar>(i,j) = 255;
                                        }
                                }
                                flag = 0;
                                count = 0;

                        }
                        //black
                        if (img.at<uchar>(i,j) == 255 && img.at<uchar>(i+1, j) == 0) {
                                index = i;
                                flag = 1;
                        }
                }
        
        }
}

void Lanes::remove_obstacles() {

        obstacles.clear();

        Mat channels[3];
        Mat channels_w[3];
        Mat dark_obs;
        Mat white_obs;
        Mat bright_obs;

        vector<vector<Point> > contours_bright;
        vector<Vec4i> hierarchy_bright;
        vector<vector<Point> > contours_dark;
        vector<Vec4i> hierarchy_dark;
        vector<vector<Point> > contours_w;
        vector<Vec4i> hierarchy_w;

        /*  for bright red and blue obstacles   */
        split(img, channels);
        bright_obs = channels[2] - channels[1];
        dark_obs = channels[0] - channels[1];

        medianBlur(bright_obs, bright_obs, 9);

        //imshow("bright_obs", bright_obs);

        /*  for white obstacles */
        cvtColor(img, white_obs, CV_BGR2HLS);
        split(white_obs, channels_w);

        /*  thresholding    */
        threshold(bright_obs, bright_obs, 50, 255, THRESH_BINARY);
        threshold(dark_obs, dark_obs, 50, 255, THRESH_BINARY);
        threshold(channels_w[1], white_obs, 200, 255, THRESH_BINARY);

        /*  morphology operations   */
        dilate(bright_obs, bright_obs, Mat(), Point(-1,-1), 7);
        erode(white_obs, white_obs, Mat(), Point(-1, -1), 7);
        //dilate(dark_obs, dark_obs, Mat(), Point(-1,-1), 4);

        /*  joiner function joins the white spaces between detected obstacles   */
        joiner_of_white_spaces_in_obstacles(bright_obs);

        /*  finding contours    */
        findContours(bright_obs, contours_bright, hierarchy_bright, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
        findContours(dark_obs, contours_dark, hierarchy_dark, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
        findContours(white_obs, contours_w, hierarchy_w, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

        /*  storing bounding boxes  */
        vector<Rect> box_bright(contours_bright.size());
        vector<Rect> box_dark(contours_dark.size());
        vector<Rect> box_w(contours_w.size());

        /*  bright red  */
        for (int i = 0; i < contours_bright.size(); i++) {
                box_bright[i] =  boundingRect(contours_bright[i]);

                if (box_bright[i].area() > 8000) {
                        circle(img, Point(box_bright[i].tl().x, box_bright[i].br().y), 8,Scalar(255, 120, 255),-1,8,0);
                        circle(img, Point(box_bright[i].br().x, box_bright[i].br().y), 8,Scalar(120, 255, 255),-1,8,0);
                        //rectangle(img, box_bright[i].tl(), box_bright[i].br(), Scalar(255, 1, 2), 10, 8, 0);
                        drawContours(img, contours_bright, i, Scalar(0, 3, 255), -1, 8, hierarchy_bright);

                }
        }

        /*  bright blue */
        for (int i = 0; i < contours_dark.size(); i++) {
                box_dark[i] =  boundingRect(contours_dark[i]);

                if (box_dark[i].area() > 8000) {
                        circle(img, Point(box_bright[i].tl().x, box_bright[i].br().y), 8,Scalar(255, 120, 255),-1,8,0);
                        circle(img, Point(box_bright[i].br().x, box_bright[i].br().y), 8,Scalar(120, 255, 255),-1,8,0);
                        //rectangle(img, box_dark[i].tl(), box_dark[i].br(), Scalar(255, 1, 2), 1, 8, 0);
                        drawContours(img, contours_dark, i, Scalar(0, 3, 255), -1, 8, hierarchy_dark);
                }
        }


        /*
          bright white    
        for (int i = 0; i < contours_w.size(); i++) {
                box_w[i] =  boundingRect(contours_w[i]);
                if (box_w[i].area() > 20000 && (box_w[i].br().y-box_w[i].tl().y) > (box_w[i].br().x-box_w[i].tl().x) &&  box_w[i].height <= 2.0*box_w[i].width) {
//                                rectangle(img, box_w[i].tl(), box_w[i].br(), Scalar(0, 0, 0), 1, 8, 0);
//                                printf("Area: %d \t Aspect ratio: %f\n", box_w[i].area(), (double)box_w[i].height/box_w[i].width);
                        //drawContours(img, contours_w, i, Scalar(0, 0, 0), 50, 8, hierarchy_w);
                        //drawContours(img, contours_w, i, Scalar(0, 0, 0), -1, 8, hierarchy_w);
                }
        }
        */

        //imshow("removed obstacle",img);
        //waitKey(10);
}


Point centroid(Mat img, double a, double lam)
{
	double x1,x2;
	x1=a/2;
	x2=((img.rows*img.rows)/lam + a)/2;
	if(x1<0)
		x1=0;
	else if(x1>=img.cols)
		x1=img.cols-1;	
	if(x2<0)
		x2=0;
	else if(x2>=img.cols)
		x2=img.cols-1;
	Point centroid;
	centroid.x=(x1+x2)/2;
	centroid.y=img.rows/2;
	return centroid;		
}


//checking if waypoint is on obstacle in top_view
bool waypoint_on_obstacle(Mat img, vector<double> way)
{
	int x_temp=way[0];//+(DIST_CHECK*cos(way[2]));
	int y_temp=way[1];//+(DIST_CHECK*sin(way[2]));
  	int x1=img.rows-y_temp;
  	int y1=x_temp;
	//circle(img, Point(y1, x1),15,Scalar(255, 0, 255),-1,8,0);
	for(int i=-1 * (KERNEL_SIZE/2); i<(KERNEL_SIZE/2) ; i++)
	{
		for(int j=-1 * (KERNEL_SIZE/2); j < (KERNEL_SIZE/2); j++)

		{
			if(!(x1+i < img.rows || x1+i >= 0 || y1+j >= 0 || y1+j < img.rows)) continue;

				if(img.at<Vec3b>(x1+i,y1+j)[0] == 0 && img.at<Vec3b>(x1+i,y1+j)[1] == 3 && img.at<Vec3b>(x1+i,y1+j)[2] == 255)
				{
                                        return true;
				}
		}
	}
        return false;

}

vector<int> obstacle_coords(Mat img, vector<double> old_way) {


        obstacles.clear();
        vector<int> temp;
        int x_l = 0;
        int y_l = 0;
        int x_r = 0;
        int y_r = 0;
        int flag_l = 0;
        int flag_r = 0;

        int sub_factor = old_way[1];
        for (int i = img.rows; i >= 0 ; i--) {
                for (int j = 0; j < img.cols; j++) {
                        if (img.at<Vec3b>(i,j)[0] == 255 && img.at<Vec3b>(i,j)[1] == 120 && img.at<Vec3b>(i,j)[2] == 255 && flag_l == 0) {
                                cout << "x_left: " << j << endl;
                                x_l = j;
                                y_l = img.rows-i;

                                flag_l = 1;
                        }
                        else if (img.at<Vec3b>(i,j)[0] == 120 && img.at<Vec3b>(i,j)[1] == 255 && img.at<Vec3b>(i,j)[2] == 255 && flag_r == 0) {
                                cout << "x_right: " << j << endl;
                                x_r = j;
                                y_r = img.rows-i;
                                flag_r = 1;
                        }
                }
        }

        temp.push_back(x_l);
        temp.push_back(y_l);
        temp.push_back(x_r);
        temp.push_back(y_r);
        obstacles.push_back(temp);
        return obstacles[0];
}

double dist(Point p1,Point p2)
{
	return sqrt(pow((p1.x-p2.x),2)+pow((p1.y-p2.y),2));
	//return fabs(p1.y-p2.y);
}

// double min_dist(vector<int> c1, Point p)
// {
// 	//double d_min=0;
// 	double d=dist(c1[0],p);
// 	//int total=0;
// 	for(int i=0;i<c1.size();i++)
// 	{
		
// 			if(dist(c1[i],p)<d)
// 				d=dist(c1[i],p);
// 			//d_min+=dist(c1[i],c2[j]);
// 			//total++;
// 	}
// 	//return d_min/(total);
// 	return d;
// }

double min_dist(vector<int> obs, Point p)
{
	double d_min=0;
	int x=(obs[0]+obs[2])/2;
	int y=(obs[1]+obs[3])/2;

	double d=dist(Point(x,y),p);

	double d1 = min(dist(Point(obs[0],obs[1]),p),dist(Point(obs[2],obs[3]),p));
	return min(d,d1);
	//int total=0;
	// for(int i=0;i<c1.size();i++)
	// {
		
	// 		if(dist(c1[i],p)<d)
	// 			d=dist(c1[i],p);
	// 		//d_min+=dist(c1[i],c2[j]);
	// 		//total++;
	// }
	//return d_min/(total);
	//return d;
}

float dist_from_lane(float lam,float a,Point p)
{
	int x_l1=p.x;
	int y_l1=sqrt(lam*(x_l1-a));
	int y_l2=p.y;
	int x_l2=y_l2*y_l2/lam + a;
	return min(abs(x_l1-x_l2),abs(y_l1-y_l2));
}

//params as lam1,a,lam2,w

vector<double> waypoint_objectivefunc(float *param, vector<int> obs, vector<double> predicted_waypoint,Mat img)
{
	vector<double> way_p(3);
	// way_p[2]=predicted_waypoint[2];
	int x,y,theta_min;
	float min_obs_dist,d,objective,min_obj=FLT_MAX;
	for(int r=10;r<=40;r+=2)
	{ 
		for(float theta=0; theta<=PI;theta+=0.1)
		{
			x=r*cos(theta);
			y=r*sin(theta);
			if(fabs(param[3])>5000)
			{
				float d1=dist_from_lane(param[0],param[1],Point(predicted_waypoint[0]+x,predicted_waypoint[1]+y));
				float d2;
				if(predicted_waypoint[0]<(pow(predicted_waypoint[0],2)-param[0]*(predicted_waypoint[1]-param[1])))
				{
					d2=min(abs(predicted_waypoint[0]+x-359),abs(predicted_waypoint[1]+y-60));
					//d2=sqrt(pow(predicted_waypoint[0]+x-359,2)+pow(predicted_waypoint[1]+y-60,2));
				}
				else
				{
					d2 = min(abs(predicted_waypoint[0]+x-img.cols),abs(predicted_waypoint[1]+y-img.rows));
					//d2=sqrt(pow(predicted_waypoint[0]+x-img.cols,2)+pow(predicted_waypoint[1]+y-img.rows,2));
				}

				if(min(d1,d2)<60)
					d=1000;
				else
					d=1/min(d1,d2);
			}
			else
			{
				float d1=dist_from_lane(param[0],param[1],Point(predicted_waypoint[0]+x,predicted_waypoint[1]+y));
				float d2=dist_from_lane(param[2],param[3],Point(predicted_waypoint[0]+x,predicted_waypoint[1]+y));
				if(min(d1,d2)<60)
					d=1000;
				else
					d=1/min(d1,d2);
			}
			//cout << "Lane"<<d<<endl;
			// min_obs_dist = FLT_MAX;
			// for(int i=0;i<obs.size();i++)
			// {
			// 	float d1=min_dist(obs,Point(predicted_waypoint[0]+x,predicted_waypoint[1]+y));
			// 	if(d1<min_obs_dist)
			// 	{
			// 		min_obs_dist=d1; 
			// 	}= gen
			// }
			min_obs_dist=min_dist(obs,Point(predicted_waypoint[0]+x,predicted_waypoint[1]+y));
			//cout << "Obstacle"<<min_obs_dist<<endl;
			if(min_obs_dist<80)
				min_obs_dist=0.1;
			objective=10000/min_obs_dist+100*d;
			if(objective<min_obj)
			{
				min_obj=objective;
				cout << "Cost"<<d<<endl;
				// cout<< "Obstacle"<<1/min_obs_dist<<endl;
				// cout << "old way point  "<<predicted_waypoint[0]<< "  "<< predicted_waypoint[1]<<endl;
				// cout << "Obstacle left points  "<<obs[0] << "  "<<obs[1]<<endl;
				//cout<< "Lanes"<<d<<endl;
				way_p[0]=predicted_waypoint[0]+x;
				way_p[1]=predicted_waypoint[1]+y;
				//way_p[2]=theta;
				//cout << "Distance from point "<<sqrt(pow(predicted_waypoint[0]+x-359,2)+pow(predicted_waypoint[1]+y-60,2))<<endl;
			}
		} 
	}
	return way_p;
}

vector<double> waypoint_objectivefunc_l(float *param, vector<double> predicted_waypoint,Mat img)
{
	vector<double> way_p(3);
	way_p[2]=predicted_waypoint[2];
	int x,y,theta_min;
	float min_obs_dist,d,objective,min_obj=FLT_MAX;
	for(int r=10;r<=30;r+=2)
	{
		for(float theta=0; theta<=PI;theta+=0.1)
		{
			x=r*cos(theta);
			y=r*sin(theta);
			if(fabs(param[3])>5000)
			{
				
				float d1=dist_from_lane(param[0],param[1],Point(predicted_waypoint[0]+x,predicted_waypoint[1]+y));
				float d2;
				if(predicted_waypoint[0]<(pow(predicted_waypoint[0],2)-param[0]*(predicted_waypoint[1]-param[1])))
					{
						//d2=sqrt(pow(predicted_waypoint[0]+x-359,2)+pow(predicted_waypoint[1]+y-60,2));
						d2 = min(abs(predicted_waypoint[0]+x-359),abs(predicted_waypoint[1]+y-60));
					}
				else
					{
						//d2=sqrt(pow(predicted_waypoint[0]+x-img.cols,2)+pow(predicted_waypoint[1]+y-img.rows,2));
						d2 = min(abs(predicted_waypoint[0]+x-632),abs(predicted_waypoint[1]+y-30));
					}
				if(min(d1,d2)<60)
					d=1000;
				else
					d=1/min(d1,d2);

			}
			else
			{
				float d1=dist_from_lane(param[0],param[1],Point(predicted_waypoint[0]+x,predicted_waypoint[1]+y));
				float d2=dist_from_lane(param[2],param[3],Point(predicted_waypoint[0]+x,predicted_waypoint[1]+y));
				if(min(d1,d2)<60)
					d=1000;
				else
					d=1/min(d1,d2);
			}
			//cout << "Lane"<<d<<endl;
			// min_obs_dist = FLT_MAX;
			// for(int i=0;i<obs.size();i++)
			// {
			// 	float d1=min_dist(obs,Point(predicted_waypoint[0]+x,predicted_waypoint[1]+y));
			// 	if(d1<min_obs_dist)
			// 	{
			// 		min_obs_dist=d1; 
			// 	}
			// }
			//min_obs_dist=min_dist(obs,Point(predicted_waypoint[0]+x,predicted_waypoint[1]+y));
			//cout << "Obstacle"<<min_obs_dist<<endl;
			objective=1000*d;
			if(objective<min_obj)
			{
				min_obj=objective;
				cout << "Cost"<<d<<endl;
				// cout<< "Obstacle"<<1/min_obs_dist<<endl;
				// cout << "old way point  "<<predicted_waypoint[0]<< "  "<< predicted_waypoint[1]<<endl;
				// cout << "Obstacle left points  "<<obs[0] << "  "<<obs[1]<<endl;
				//cout<< "Lanes"<<d<<endl;
				way_p[0]=predicted_waypoint[0]+x;
				way_p[1]=predicted_waypoint[1]+y;
				//way_p[2]=theta;

				//cout << "Distance from point "<<sqrt(pow(predicted_waypoint[0]+x-359,2)+pow(predicted_waypoint[1]+y-60,2))<<endl;
			}
		} 
	}
	return way_p;
}