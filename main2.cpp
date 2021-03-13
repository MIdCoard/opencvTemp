#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "cfloat"
#include "iostream"
using namespace cv;
using namespace std;

double calcLinesK(const Mat& frame) {
	Mat dst,cdst;
	Canny(frame, dst, 50, 200, 3);
	cvtColor(dst, cdst, COLOR_GRAY2BGR);
	vector<Vec4i> linesP; // will hold the results of the detection
	HoughLinesP(dst, linesP, 1, CV_PI/180, 50, 50, 10 ); // runs the actual detection
	// Draw the lines
	vector<double> ks;
	for(auto l : linesP){
		double k = (l[1] - l[3])/(double)(l[0] - l[2]);
		if (!isnan(k) && !isinf(k))
			ks.push_back(k);
	}
	sort(ks.begin(),ks.end());
	int start = ks.size() * 0.4;
	int end = ks.size() * 0.6;
	double ksum = 0;
	for (int i = start;i<=end;i++)
		ksum += ks[i];
	double aver = ksum / (end - start + 1);
	return aver;
}

int main(int argc, char** argv)
{
	VideoCapture capture("../rotation.mp4");
	if (!capture.isOpened()){
		//error in opening the video input
		cerr << "Unable to open file!" << endl;
		return 0;
	}
	Mat frame;
	Mat lastFrame;
	Mat dst;
	bool first = false;
	while(capture.read(frame)){
		auto timer = (double)getTickCount();
		double k = calcLinesK(frame);
		double verk = 1/k;
		double x = frame.cols/2;
		double y = frame.rows/2;
		double dx = 100;
		double dy = k * dx;
		cout<<k<<endl;
		arrowedLine(frame, Point(x,y), Point(x + dx,y + dy), Scalar(0, 0, 255), 3);
		double fps = getTickFrequency() / ((double)getTickCount() - timer);
		stringstream ss;
		ss<<fps;
		putText(frame, "FPS : " + ss.str(), Point(100,50), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50,170,50), 2);
		imshow("img", frame);
		int keyboard = waitKey(30);
		if (keyboard == 'q' || keyboard == 27)
			break;
		else if (keyboard == ' ') {
			waitKey(0);
		}
		lastFrame = frame;
		first = true;
	}
	return 0;
}
