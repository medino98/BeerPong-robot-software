#include <conio.h>

#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/core.hpp"

#include <iostream>

using namespace std;
using namespace cv;

// Buffers
Mat frame;
Mat gray;
Mat GB;
Mat binary;
Mat threshold_output;
Mat summa;
Mat selected;
vector<vector<Point>> contours;

//Mouse handling variables
bool selectObject = false;
Rect selection;
Point origin;
Mat canvas;
vector<Rect> selections;

//Various variables
int thresh = 40;
bool start_process = false;

//General mouse handling for selection of ROI
static bool showSelection()
{
	rectangle(canvas, selection, Scalar(0, 255, 0), 2);

	return true;
}

static void onMouse(int event, int x, int y, int, void*)
{
	switch (event)
	{
	case EVENT_LBUTTONDOWN:
		origin = Point(x, y);
		selectObject = true;
		break;
	case EVENT_LBUTTONUP:
		selectObject = false;
		break;
	}

	if (selectObject)
	{
		selection.x = MIN(x, origin.x);
		selection.y = MIN(y, origin.y);
		selection.width = abs(x - origin.x) + 1;
		selection.height = abs(y - origin.y) + 1;
		//selection &= Rect(0, 0, frame.cols, frame.rows);

		if (selection.width > 0 && selection.height > 0)
		{
			canvas = Mat::zeros(frame.size(), frame.type());
			showSelection();
		}
	}
}

int main(int, char**)
{
	// Open the default camera
	VideoCapture cap(0);
	// Check if we succeeded
	if (!cap.isOpened())
	{
		_getch();
		return -1;
	} 

	// Windows
	namedWindow("Selection", WINDOW_AUTOSIZE);

	// GUI elements
	setMouseCallback("Selection", onMouse);

	// Main loop
	while (true)
	{
		// Get a new frame from camera
		cap >> frame;

		if (canvas.empty()) canvas = Mat::zeros(frame.size(), frame.type());

		imshow("Selection", frame+canvas);

		int key = waitKey(10);

		if (key == 's') // crops and saves selections
		{
			start_process = true;
		}

		if (start_process)
		{
			// Apply blur
			GaussianBlur(frame(selection), GB, Size(11, 11), 0, 0);

			// Convert to greyscale
			cvtColor(GB, gray, COLOR_BGR2GRAY);

			// Make a binary from greyscale w threshold function
			threshold(gray, binary, thresh, 255, THRESH_BINARY);

			selected = Mat::ones(frame.size(), CV_8U);
			binary.copyTo(selected(Rect(selection.x, selection.y, selection.width, selection.height)));

			imshow("test", selected);

			RotatedRect rRect = RotatedRect(Point2f(100, 100), Size2f(100, 50), 30);

			// Find contours
			findContours(selected, contours, RETR_TREE, CHAIN_APPROX_SIMPLE/*, Point(0, 0)*/);

			// Find the rotated rectangles and ellipses for each contour
			vector<RotatedRect> minRect(contours.size());
			vector<RotatedRect> minEllipse(contours.size());

			for (int i = 0; i < contours.size(); i++)
			{
				minRect[i] = minAreaRect(Mat(contours[i]));
				if (contours[i].size() > 5)
				{
					minEllipse[i] = fitEllipse(Mat(contours[i]));
				}
			}

			// Draw ellipses
			Mat drawing = Mat::zeros(frame.size(), CV_8UC3);
			// Magic number lol 80
			int sizeFilter = 20;
			for (int i = 0; i < contours.size(); i++)
			{
				if (minRect[i].size.width > sizeFilter && minRect[i].size.height > sizeFilter)
				{
					Scalar color = Scalar(0, 255, 0);
					// contour
					//drawContours(drawing, contours, i, color, 1, 8, vector<Vec4i>(), 0, Point());
					// ellipse
					ellipse(drawing, minEllipse[i], color, 2, 8);
					// rotated rectangle
					/*Point2f rect_points[4]; minRect[i].points(rect_points);
					for (int j = 0; j < 4; j++)
					{
						line(drawing, rect_points[j], rect_points[(j + 1) % 4], color, 1, 8);
					}*/
				}
			}
			add(frame, drawing, summa);

			//Show in a window
			imshow("Final", summa);
		}

		if (key == 27) break;
	}
	// The camera will be deinitialized automatically in VideoCapture destructor
	return 0;
}