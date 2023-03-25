/* Jake Stringfellow
   CS 5330 Spring 2023
   Calibration and Augmented Reality
*/  

#include <stdio.h>
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <cstdio>
#include <cstring>
#include <vector>
#include <dirent.h>
#include <iostream>
#include <opencv2/calib3d.hpp>



int main(int argc, char *argv[]) {     
  
    cv::VideoCapture *capdev;
    // open the user video, set pointer to it
    capdev = new cv::VideoCapture(0);
    // If a video device is not being accessed alert the user
    if( !capdev->isOpened() ) {
        printf("Unable to open video device\n");
        return(0);
    }
    
    // Identify windows
    cv::namedWindow("Live Video", 1);
    //cv::namedWindow("Threshold Video", 1);

    // Initialize frame, will be streamed from the camera
    cv::Mat frame;
    
    for(;;) {
        *capdev >> frame; // get a new frame from the camera, treat as a stream
//        cv::Mat threshold = preprocess_threshold(frame);

	cv::Mat grayscaleFrame;
	
	// convert the frame to one color channel for ease of use in the cv functions
	cv::cvtColor(frame, grayscaleFrame, cv::COLOR_BGR2GRAY);

        std::vector<cv::Point2f> corner_set;
	cv::Size patternsize(9,6);
	
	// Without flags the fps slows considerably
	bool cornersFound = cv::findChessboardCorners(grayscaleFrame, patternsize, corner_set, cv::CALIB_CB_ADAPTIVE_THRESH 
						      + cv::CALIB_CB_NORMALIZE_IMAGE + cv::CALIB_CB_FAST_CHECK);

	std::cout << corner_set.size() << std::endl;


	if (cornersFound) {
	    cv::cornerSubPix(grayscaleFrame, corner_set, cv::Size(1,1), cv::Size(-1,-1), 
			 cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 30, 0.1));
	}

	cv::drawChessboardCorners(frame, patternsize, cv::Mat(corner_set), cornersFound);
	


        // If the frame is empty, alert the user, break the loop
        if( frame.empty() ) {
            printf("frame is empty\n");
            break;
        }
	
        else {
            cv::imshow("Live Video", frame);
	    //cv::imshow("Threshold Video", threshold_CU);
	}
    
        // see if there is a waiting keystroke
        char key = cv::waitKey(1);
        
        // If that keystroke is s, take a screenshot
        if( key == 's') {
            cv::imwrite("screenshot.jpg", frame);
	}
        // If that keystroke is q, quit the program
        else if( key == 'q') {
            break;
        }
    }
    
    // Delete the live video object
    delete capdev;
    return(0);
}

