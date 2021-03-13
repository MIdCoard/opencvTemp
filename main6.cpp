//
// Created by 周蜀杰 on 2021/3/12.
//


#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/tracking.hpp>

using namespace cv;
using namespace std;

vector<Mat> mats;

int main() {
	Mat out = Mat(1080,1920,CV_8UC3);
	int a = 1080 / 4;
	int b = 1920 / 7;
	for (int i = 0;i<4;i++)
		for (int j = 0;j<7;j++){
			stringstream ss;
			ss<<(i*7+j+1);
			Mat mat = imread("../temp/" + ss.str() + ".png");
			cout<<"read it"<<endl;
			resize(mat,mat,Size(a,b));
			int startx = i * a;
			int starty = j * b;
			for (int k = 0;k<a;k++)
				for (int l = 0;l<b;l++)
					out.at<Vec3b>(startx + k,starty + l) = mat.at<Vec3b>(k,l);
			cout<<(i*7+j+1)<<endl;
		}
	imshow("FFF",out);
		imwrite("../temp.png",out);
	waitKey();
	return 0;
}
