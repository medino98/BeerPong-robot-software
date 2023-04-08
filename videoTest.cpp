/*

#include "opencv2/opencv.hpp"

using namespace cv;

int main(int, char**)
{
	VideoCapture cap(0); // open the default camera
	if (!cap.isOpened())  // check if we succeeded
		return -1;

	Mat edges;
	namedWindow("edges", 1);
	for (;;)
	{
		Mat frame;
		cap >> frame; // get a new frame from camera
		cvtColor(frame, edges, COLOR_BGR2GRAY);
		GaussianBlur(edges, edges, Size(7, 7), 1.5, 1.5);
		Canny(edges, edges, 0, 30, 3);

		RotatedRect rRect = RotatedRect(Point2f(100, 100), Size2f(100, 50), 30);

		Point2f vertices[4];
		rRect.points(vertices);
		for (int i = 0; i < 4; i++)
		{
			line(frame, vertices[i], vertices[(i + 1) % 4], Scalar(0, 255, 0));
		}
		Rect brect = rRect.boundingRect();
		rectangle(frame, brect, Scalar(255, 0, 0));



		imshow("frame", frame);
		imshow("edges", edges);

		if (waitKey(30) >= 0) break;
	}
	// the camera will be deinitialized automatically in VideoCapture destructor
	return 0;
}


*/