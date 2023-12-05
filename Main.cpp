#include <conio.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <fstream>

#include <opencv2/opencv.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

#define WRITE_PARAMS_TO_FILE

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

// Defining the unit dimensions of checkerboard
int checkerboard[2]{5,8}; 

// Size of one square of the calibration chessboard
int sizeOfSquare = 30;

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


void CalibrateCamera()
{
  // Creating vector to store vectors of 3D points for each checkerboard image
  std::vector<std::vector<cv::Point3f>> objpoints;
 
  // Creating vector to store vectors of 2D points for each checkerboard image
  std::vector<std::vector<cv::Point2f>> imgpoints;
 
  // Defining the world coordinates for 3D points
  std::vector<cv::Point3f> objp;
  for(int i{0}; i<checkerboard[1]; i++)
  {
    for(int j{0}; j<checkerboard[0]; j++)
	{
      objp.push_back(cv::Point3f(j*sizeOfSquare,i*sizeOfSquare,0));
	}

  }
 
 
  // Extracting path of individual image stored in a given directory
  std::vector<cv::String> images;
  // Path of the folder containing checkerboard images
  std::string path = "C:\\Programming_workspace\\C_C++_workspace\\BeerPong-robot-software\\images";
 
  cv::glob(path, images);
 
  cv::Mat frame, gray, undist_frame;
  // vector to store the pixel coordinates of detected checker board corners 
  std::vector<cv::Point2f> corner_pts;
  bool success;
 
  // Looping over all the images in the directory
  for(int i{0}; i<images.size(); i++)
  {
    frame = cv::imread(images[i]);
    cv::cvtColor(frame,gray,cv::COLOR_BGR2GRAY);
 
    // Finding checker board corners
    // If desired number of corners are found in the image then success = true  
    success = cv::findChessboardCorners(gray, cv::Size(checkerboard[0], checkerboard[1]), corner_pts, CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_FAST_CHECK | CALIB_CB_NORMALIZE_IMAGE);
     
    /* 
     * If desired number of corner are detected,
     * we refine the pixel coordinates and display 
     * them on the images of checker board
    */
    if(success)
    {
      cv::TermCriteria criteria(cv::TermCriteria::EPS | cv::TermCriteria::MAX_ITER , 30, 0.001);
       
      // refining pixel coordinates for given 2d points.
      cv::cornerSubPix(gray,corner_pts,cv::Size(11,11), cv::Size(-1,-1),criteria);
       
      // Displaying the detected corner points on the checker board
      cv::drawChessboardCorners(frame, cv::Size(checkerboard[0], checkerboard[1]), corner_pts, success);
       
      objpoints.push_back(objp);
      imgpoints.push_back(corner_pts);
    }
	
    cv::imshow("Image",frame);
    cv::waitKey(0);
  }
 
  cv::destroyAllWindows();
 
  cv::Mat cameraMatrix,distCoeffs,R,T;
 
  /*
   * Performing camera calibration by 
   * passing the value of known 3D points (objpoints)
   * and corresponding pixel coordinates of the 
   * detected corners (imgpoints)
  */
  cv::calibrateCamera(objpoints, imgpoints, cv::Size(gray.rows,gray.cols), cameraMatrix, distCoeffs, R, T);

#ifdef WRITE_PARAMS_TO_FILE
	cv::FileStorage fs("camera_calibration.xml", cv::FileStorage::WRITE);
    fs << "cameraMatrix" << cameraMatrix;
    fs << "distortionCoeffs" << distCoeffs;
	fs << "Rotation vector : " << R;
	fs << "Translation vector : " << T;
    fs.release();
#else 
	cout << "cameraMatrix : " << cameraMatrix << endl;
	cout << "distCoeffs : " << distCoeffs << endl;
	cout << "Rotation vector : " << R << endl;
	cout << "Translation vector : " << T << endl;
#endif

	

}

int findEllpise()
{
	// Init video object
	VideoCapture cap;
	// Open the default camera
	cap.open(0);
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
		cap.read(frame);

		// check if we succeeded
        if (frame.empty()) {
            cerr << "ERROR! blank frame grabbed\n";
            break;
        }

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
		return 0;
	}
}


int main(int, char**)
{
	CalibrateCamera();
	// The camera will be deinitialized automatically in VideoCapture destructor
	return 0;
}
//random comment to push