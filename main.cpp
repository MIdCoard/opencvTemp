#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
using namespace cv;
using namespace std;

TermCriteria term_crit(TermCriteria::EPS | TermCriteria::COUNT, 5, 1);


int channels[] = {0};
float range_[] = {0, 180};
const float* range[] = {range_};

bool checkParam2(bool flag,int param2) {
	if (flag)
		return param2 < 45;
	else return param2 < 60;
}

bool detectCircle(const Mat& frame, Rect&bbor,int dx,int dy,bool flag = false) {
	vector<Vec3f> circles;
	Mat gray;
	cvtColor(frame, gray, COLOR_BGR2GRAY);
	medianBlur(gray, gray, 5);
	int param2 = 40;
	do {
		cout<<param2<<endl;
		if (!flag)
		HoughCircles(gray, circles, HOUGH_GRADIENT, 1,
		             gray.rows / 16 <= 0 ? 1 : gray.rows / 16,  // change this value to detect circles with different distances to each other
		             100, param2++, 50, 100 // change the last two parameters
				// (min_radius & max_radius) to detect larger circles
		);
		else HoughCircles(gray, circles, HOUGH_GRADIENT, 1,
		                  gray.rows / 16 <= 0 ? 1 : gray.rows / 16,  // change this value to detect circles with different distances to each other
		                  100, param2++, 40, max(frame.rows,frame.cols) // change the last two parameters
					// (min_radius & max_radius) to detect larger circles
			);
		Mat copy;
		frame.copyTo(copy);
		for (auto & circle : circles) {
			Rect2d te = Rect2d(circle[0] - circle[2] + dx,circle[1] - circle[2] + dy,circle[2]*2,circle[2]*2);
			rectangle(copy,te,255,2);
		}
		imshow("DDD",copy);
		waitKey();
	} while(circles.size() > 1 && checkParam2(flag,param2));
	cout<<param2<<endl;
	if (circles.size() == 1) {
		Vec3i c = circles[0];
		bbor = Rect2d(c[0] - c[2] + dx,c[1] - c[2] + dy,c[2]*2,c[2]*2);
		return true;
	}
	cout<<"miss"<<endl;
	return false;
}

void initCircle(const Mat& frame,Rect&bbor,Mat&roi) {
	Mat hsv,mask;
	cvtColor(frame(bbor), hsv, COLOR_BGR2HSV);
	inRange(hsv, Scalar(0, 60, 32), Scalar(180, 255, 255), mask);
	int histSize[] = {180};
	calcHist(&hsv, 1, channels, mask, roi, 1, histSize, range);
	normalize(roi, roi, 0, 255, NORM_MINMAX);
}

void trackCircle(const Mat& frame,Rect&bbor,Mat&roi) {
	Mat hsv, destination;
	cvtColor(frame, hsv, COLOR_BGR2HSV);
	calcBackProject(&hsv, 1, channels, roi, destination, range);
	meanShift(destination, bbor, term_crit);
	rectangle(frame,bbor, 255, 2);
}


bool pause() {
	int keyboard = waitKey(30);
	if (keyboard == 'q' || keyboard == 27)
		return true;
	else if (keyboard == ' ') {
		waitKey(0);
	}
	return false;
}

Mat mat2(const Mat& frame,const Rect& bbor) {
	int b = max(bbor.width ,bbor.height);
	int leftx = bbor.x - b / 2;
	int rightx = bbor.x + b * 3 / 2;
	int lefty = bbor.y - b / 2;
	int righty = bbor.y + b * 3 / 2;
	leftx = leftx < 0 ? 0 : leftx;
	rightx = rightx > frame.cols ? frame.cols : rightx;
	lefty = lefty < 0 ? 0 : lefty;
	righty = righty > frame.rows ? frame.rows : righty;
	int a = min(rightx - leftx,righty - lefty);
	return Mat(frame,Rect(leftx,lefty,a,a));
}

bool first = true;

int main(int argc, char **argv)
{
//	Mat f = imread("../h2.png");
//	Rect bbor;
//	detectCircle(f,bbor,0,0);
//	rectangle(f,bbor,255,2);
//	imshow("fff",f);
//	waitKey();
//	return 0;
	VideoCapture capture("../a.mp4");
//	VideoCapture capture(0);
	if (!capture.isOpened()){
		//error in opening the video input
		cerr << "Unable to open file!" << endl;
		return 0;
	}
	Rect bbor;
	bool isDetected = false;
	Mat frame,roi;
	capture >> frame;
	if (detectCircle(frame, bbor,0,0)) {
		isDetected = true;
		first = false;
		initCircle(frame, bbor, roi);
	}
	int pos = 0;
	int lastx = -1,lasty = -1;
	int64 lasttimer = getTickCount();
	int pos2 = 0;
	while(capture.read(frame)){
		int64 timer = getTickCount();
		if (bbor.width < 50 && bbor.height < 50)
			isDetected = false;
		if (isDetected) {
			if (timer - lasttimer > 3000000) {
				if (!detectCircle(mat2(frame,bbor),bbor,bbor.x - bbor.width /2,bbor.y - bbor.height /2,true)) {
					pos2++;
					if (pos2 == 10) {
						isDetected = false;
						pos2 = 0;
					}
					trackCircle(frame,bbor,roi);
				} else {
					pos2 = 0;
					cout<<"capture"<<endl;
//					trackCircle(frame, bbor, roi); // track twice
				}
				lasttimer = timer;
			} else
			trackCircle(frame, bbor, roi);
		} else {
			pos++;
			if (pos > 15) {
				if (detectCircle(frame, bbor,0,0)) {
					isDetected = true;
					if (first) {
						first = false;
						initCircle(frame, bbor, roi);
					}
					trackCircle(frame,bbor,roi);
				}
				pos = 0;
			}
		}
		if (isDetected) {
			int x = bbor.x + bbor.width / 2;
			int y = bbor.y + bbor.height / 2;
			if (lastx != -1 && lasty != -1) {
				int k = 10;
				int dx = (x - lastx) * k;
				int dy = (y - lasty) * k;
				double value = sqrt(dx*dx+dy*dy)/k;
				arrowedLine(frame, Point(x, y), Point(x + dx, y + dy), Scalar(0, 0, 255), 3);
				stringstream ss;
				ss<<value;
				putText(frame, "Velocity : " + ss.str(), Point(100,20), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0,0,0), 2);
			}
			lastx = x;
			lasty = y;
		}
		int fps = getTickFrequency() / (getTickCount() - timer);
		stringstream ss;
		ss<<fps;
		putText(frame, "FPS : " + ss.str(), Point(100,50), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50,170,50), 2);
		imshow("img", frame);
		if (pause())
			break;
	}
}


