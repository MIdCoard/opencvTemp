#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/tracking.hpp>
using namespace cv;
using namespace std;
Ptr<Tracker> tracker;

bool detectCircle(const Mat& frame, Rect2d&bbor,int dx,int dy,bool flag = false) {
	vector<Vec3f> circles;
	Mat gray;
	cvtColor(frame, gray, COLOR_BGR2GRAY);
	medianBlur(gray, gray, 5);
	int param2 = flag?40:54;
	if (!flag) {
		int left = 0;
		int right = 1000;
		int mid = (left + right )/ 2;
		while (left < right) {
			HoughCircles(gray, circles, HOUGH_GRADIENT, 2,
			             gray.rows / 16 <= 0 ? 1 : gray.rows /
			                                       16,  // change this value to detect circles with different distances to each other
			             100, mid, 50, 90// change the last two parameters
					// (min_radius & max_radius) to detect larger circles
			);
			if (circles.size() > 1 )
				left = mid + 1;
			else if (circles.empty())
				right = mid - 1;
			else {
				break;
			}
			mid = (left + right) / 2;
		}
	} else {
		int left = 40;
		int right = 60;
		int mid = (left + right )/ 2;
		while (left < right) {
			HoughCircles(gray, circles, HOUGH_GRADIENT, 2,
			             gray.rows ,  // change this value to detect circles with different distances to each other
			             100, mid, min(max(frame.rows,frame.cols)/4 - 5, 0) , min(frame.rows,frame.cols)/4 + 5// change the last two parameters
					// (min_radius & max_radius) to detect larger circles
			);
			if (circles.size() > 1 )
				left = mid + 1;
			else if (circles.empty())
				right = mid - 1;
			else {
				break;
			}
			mid = (left + right) / 2;
		}
	}
	if (circles.size() == 1) {
		Vec3i c = circles[0];
		bbor = Rect2d(c[0] - c[2] + dx,c[1] - c[2] + dy,c[2]*2,c[2]*2);
		return true;
	}
	cout<<"miss"<<endl;
	return false;
}

void initCircle(const Mat& frame,Rect2d&bbor) {
	tracker = TrackerKCF::create();
	tracker->init(frame,bbor);
}

void draw(const Mat& frame,Rect2d&bbor) {
	rectangle(frame,bbor,255,2);
}

bool trackCircleWithoutDraw(const Mat& frame,Rect2d&bbor) {
	return tracker->update(frame,bbor);
}

bool trackCircle(const Mat& frame,Rect2d&bbor) {
	bool ret = trackCircleWithoutDraw(frame,bbor);
	if (ret)
		draw(frame,bbor);
	return ret;
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

Rect getBborExpand(const Mat& frame,const Rect& bbor) {
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
	return {leftx,lefty,a,a};
}


int main(int argc, char **argv)
{
//	Mat f = imread("../h2.png");
//	Rect2d r;
//	bool fx = detectCircle(f,r,0,0);
//	cout<<fx<<endl;
//	rectangle(f,r,255,2);
//	imshow("fff",f);
//	waitKey();
//	return 0;
	string trackerTypes[8] = {"BOOSTING", "MIL", "KCF", "TLD","MEDIANFLOW", "GOTURN", "MOSSE", "CSRT"};
	// vector <string> trackerTypes(types, std::end(types));

	// Create a tracker
	string trackerType = trackerTypes[2];

	if (trackerType == "BOOSTING")
		tracker = TrackerBoosting::create();
	if (trackerType == "MIL")
		tracker = TrackerMIL::create();
	if (trackerType == "KCF")
		tracker = TrackerKCF::create();
	if (trackerType == "TLD")
		tracker = TrackerTLD::create();
	if (trackerType == "MEDIANFLOW")
		tracker = TrackerMedianFlow::create();
	if (trackerType == "GOTURN")
		tracker = TrackerGOTURN::create();
	if (trackerType == "MOSSE")
		tracker = TrackerMOSSE::create();
	if (trackerType == "CSRT")
		tracker = TrackerCSRT::create();
	VideoCapture capture("../a.mp4");
	if (!capture.isOpened()){
		//error in opening the video input
		cerr << "Unable to open file!" << endl;
		return 0;
	}
	Rect2d bbor;
	bool isDetected = false;
	Mat frame,roi;
	capture >> frame;
	if (detectCircle(frame, bbor,0,0)) {
		isDetected = true;
		initCircle(frame, bbor);
	}
	int lastx = -1,lasty = -1;
	int pos2 = 0;
	while(capture.read(frame)){
		int64 timer = getTickCount();
		if (bbor.width < 50 && bbor.height < 50)
			isDetected = false;
		Rect rect = getBborExpand(frame,bbor);
		if (isDetected) {
			if (!detectCircle(Mat(frame,rect),bbor,rect.x,rect.y,true)) {
				pos2++;
				if (pos2 == 10) {
					isDetected = false;
					pos2 = 0;
				}
				bool flag = trackCircle(frame, bbor);
				if (flag) {
					pos2 = 0;
					cout<<"tracked"<<endl;
				}
			} else {
				pos2 = 0;
				cout<<"capture"<<endl; // track twice
//				trackCircleWithoutDraw(frame,bbor);
				draw(frame,bbor);
			}
		} else {
			if (detectCircle(frame, bbor, 0, 0)) {
				isDetected = true;
				lasty = -1;
				lastx = -1;
				initCircle(frame, bbor);
				trackCircle(frame, bbor);
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


