
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include "cmath"
using namespace cv;
using namespace std;

bool pause() {
	int keyboard = waitKey(1);
	if (keyboard == 'q' || keyboard == 27)
		return true;
	else if (keyboard == ' ') {
		waitKey(0);
	}
	return false;
}

int main( int argc, char** argv )
{

	VideoCapture capture("../moving_updown.mp4");
	if (!capture.isOpened()){
		//error in opening the video input
		cerr << "Unable to open file!" << endl;
		return 0;
	}
	Mat frame,targetFrame,temp;
	Mat dst1,dst2;
	capture.read(frame);
	frame = Mat(frame,Rect(0,0,200,200));
	cvtColor( frame, targetFrame, COLOR_BGR2GRAY );
	targetFrame.convertTo(dst2,CV_32FC1);
	int lastdx = 0,lastdy = 0;
	while (capture.read(frame)) {
		temp = Mat(frame,Rect(0,0,200,200));
		auto timer = (double)getTickCount();
		cvtColor( temp, targetFrame, COLOR_BGR2GRAY );
		dst1 = dst2.clone();
		targetFrame.convertTo(dst2,CV_32FC1);
		Point2d phaseShift = phaseCorrelate(dst1,dst2);
		int k = 10;
		int dx = phaseShift.x * k;
		int dy = phaseShift.y * k;
		if (abs(dx - lastdx) > 400)
			dx = lastdx;
		if (abs(dy - lastdy) > 400)
			dy = lastdy;
		int x = frame.cols / 2;
		int y = frame.rows / 2;
		arrowedLine(frame, Point(x,y), Point(x + dx,y + dy), Scalar(0, 0, 255), 3);
		int fps = getTickFrequency() / ((double)getTickCount() - timer);
		stringstream ss;
		ss<<fps;
		putText(frame, "FPS : " + ss.str(), Point(100,50), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50,170,50), 2);
		double value = sqrt(dx*dx+dy*dy)/k;
		stringstream ss2;
		ss2<<value;
		putText(frame, "Velocity : " + ss2.str(), Point(100,20), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0,0,0), 2);
		imshow("img",frame);
		lastdx = dx;
		lastdy = dy;
		if (pause())
			break;
	}
	return 0;
}