#include <imreg_fmt/image_registration.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
using namespace cv;
using namespace std;

bool pause() {
	int keyboard = waitKey(30);
	if (keyboard == 'q' || keyboard == 27)
		return true;
	else if (keyboard == ' ') {
		waitKey(0);
	}
	return false;
}

Mat frames[2];

void drawArc(Mat src, const Point& arcCenter, const Point& startPoint, const Point& endPoint, int Fill) {
	if (Fill <= 0) return;

	vector<Point> Dots;
	double Angle1 = atan2((startPoint.y - arcCenter.y), (startPoint.x - arcCenter.x));
	double Angle2 = atan2((endPoint.y - arcCenter.y), (endPoint.x - arcCenter.x));
	double Angle = Angle1 - Angle2;
	Angle = Angle * 180.0 / CV_PI;

	if (Angle < 0) Angle = 360 + Angle;
	if (Angle == 0) Angle = 360;
	int brim = floor(Angle / 10); // 向下取整

	Dots.push_back(startPoint);
	for (int i = 0; i < brim; i++) {
		double dSinRot = sin(-(10 * (i + 1)) * CV_PI / 180);
		double dCosRot = cos(-(10 * (i + 1)) * CV_PI / 180);
		int x = arcCenter.x + dCosRot * (startPoint.x - arcCenter.x) - dSinRot * (startPoint.y - arcCenter.y);
		int y = arcCenter.y + dSinRot * (startPoint.x - arcCenter.x) + dCosRot * (startPoint.y - arcCenter.y);
		Dots.push_back(Point(x, y));
	}
	Dots.push_back(endPoint);
	Scalar color = Scalar(0,0,255);
	for (int i = 0; i < Dots.size() - 1; i++) {
		line(src, Dots[i], Dots[i + 1], color, Fill);
	}
	Dots.clear();
}

void drawDirectionBase(const Mat& mat) {
	int x = mat.cols;
	int y = mat.rows;
	x -= 200;
	y -= 200;
	int startx = x + 100;
	int starty = y;
	int endx = x;
	int endy = y + 100;
	drawArc(mat,Point(x,y),Point(endx,endy),Point(startx,starty),2);
}

void drawPositiveArrowLine(Mat mat,double angle) {
	drawDirectionBase(mat);
	int x = mat.cols;
	int y = mat.rows;
	x -= 100;
	y -= 200;
	int x2 = x;
	int y2 = y - 100*angle;
	arrowedLine(mat, Point(x,y), Point(x2,y2), Scalar(0, 0, 255), 3);
}

void drawNegativeArrowLine(Mat mat,double angle) {
	drawDirectionBase(mat);
	int x = mat.cols;
	int y = mat.rows;
	x -= 200;
	y -= 100;
	int x2 = x - 100*angle;
	int y2 = y;
	arrowedLine(mat, Point(x,y), Point(x2,y2), Scalar(0, 0, 255), 3);
}

Mat middleFrame(const Mat& frame) {
	Rect part = Rect(frame.cols/2 - 75,frames->rows/2 - 75,150,150);
	return Mat(frame,part);
}

void drawMiddleFrame(Mat frame) {
	Rect part = Rect(frame.cols/2 - 75,frames->rows/2 - 75,150,150);
	rectangle(frame,part,255,2);
}

int main(int argc, char **argv)
{
	VideoCapture capture;
	capture = VideoCapture("../rotation.mp4");
//	capture = VideoCapture(0);
	if (!capture.isOpened())
	{
		cout << "Cannot open file " << argv[1] << endl;
		return 1;
	}
	int pos = 0;
	capture.read(frames[pos]);
	ImageRegistration image_registration(middleFrame(frames[pos]));
	Mat output;
	// x, y, rotation, scale
	vector<double> transform_params(4, 0.0);
	Mat registered_image;
	Mat previous_image;
	int lastdx = 0,lastdy = 0;
	double rotations = 0;
	double lastrotation = 0;
	while(capture.read(frames[pos = 1^pos])){
		auto timer = (double)getTickCount();
		image_registration.registerImage(middleFrame(frames[pos]), registered_image, transform_params, false);
		image_registration.next();
//		cout << "x: " << transform_params[0] << ", y: "
//		          << transform_params[1] << ", rotation: " << transform_params[2]
//		          << ", scale: " << transform_params[3] << endl;
		double rotation = transform_params[2];
		if (abs(lastrotation - rotation) > 2)
			rotation = lastrotation;
		int k = 10;
		int dx = transform_params[0] * k;
		int dy = transform_params[1] * k;
		if (abs(dx - lastdx) > 400)
			dx = lastdx;
		if (abs(dy - lastdy) > 400)
			dy = lastdy;
		int x = frames[pos^1].cols / 2;
		int y = frames[pos^1].rows / 2;
		arrowedLine(frames[pos^1], Point(x,y), Point(x + dx,y + dy), Scalar(0, 0, 255), 3);
		int fps = getTickFrequency() / ((double)getTickCount() - timer);
		stringstream ss;
		ss<<fps;
		putText(frames[pos^1], "FPS : " + ss.str(), Point(100,50), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50,170,50), 2);
		double value = sqrt(dx*dx+dy*dy)/k;
		stringstream ss2;
		ss2<<value;
		putText(frames[pos^1], "Velocity : " + ss2.str(), Point(100,20), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0,0,0), 2);
		stringstream  ss3;
		ss3<<rotation;
		putText(frames[pos^1], "Rotation : " + ss3.str(), Point(100,80), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0,0,0), 2);
		rotations += rotation*2;
		if (rotation > 0 )
			drawPositiveArrowLine(frames[pos^1],abs(rotation));
		else drawNegativeArrowLine(frames[pos^1],abs(rotation));
		stringstream  ss4;
		ss4<<rotations;
		putText(frames[pos^1], "Rotations : " + ss4.str(), Point(100,110), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0,0,0), 2);
		drawMiddleFrame(frames[pos^1]);
		imshow("img", frames[pos^1]);
		lastdx = dx;
		lastdy = dy;
		lastrotation = rotation;
		if (pause())
			break;
	}
	return 0;
}
