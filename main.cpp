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

    int calib_counter = 1;

    cv::Vec3f point_set;
    std::vector<std::vector<cv::Vec3f>> point_list;
    std::vector<std::vector<cv::Point2f>> corner_list;

    // Boolean switch to make sure the points are calculated only once 
    bool points_calculated = false; 

    for(;;) {
	// Only create 5 calibration images
	if (calib_counter > 5) {
	    calib_counter = 1;
	}

        *capdev >> frame; // get a new frame from the camera, treat as a stream

	cv::Mat grayscaleFrame;
	
	// convert the frame to one color channel for ease of use in the cv functions
	cv::cvtColor(frame, grayscaleFrame, cv::COLOR_BGR2GRAY);

        std::vector<cv::Point2f> corner_set;
	std::vector<cv::Vec3f> point_set;

	int pattern_columns = 9;
	int pattern_rows = 6;

	cv::Size patternsize(pattern_columns,pattern_rows);
	bool cornersFound = false;
	
	// Without flags the fps slows considerably
	cornersFound = cv::findChessboardCorners(grayscaleFrame, patternsize, corner_set, cv::CALIB_CB_ADAPTIVE_THRESH 
						      + cv::CALIB_CB_NORMALIZE_IMAGE + cv::CALIB_CB_FAST_CHECK);

	//std::cout << corner_set.size() << std::endl;


	if (cornersFound) {
	    cv::cornerSubPix(grayscaleFrame, corner_set, cv::Size(11,11), cv::Size(-1,-1), 
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
	}
    
        // see if there is a waiting keystroke
        char key = cv::waitKey(1);
        
        // If that keystroke is s, take a screenshot
        if( key == 's') {
            cv::imwrite("screenshot.jpg", frame);

	    // Calculate the points if they haven't been calculated already
	    if (!points_calculated) {
                // Create the point set, should be the same for each calibration image  
                // In the form of top left = (0,0,0), to the right is (1,0,0), row below is (0,-1,0)
                for (int i=0; i<corner_set.size(); i++) {
                    for (int c=0; c<pattern_columns; c++) {
                        for (int r=0; r<pattern_rows; r++) {
                            // For a planar pattern like a chessboard, set z to zero
                            // Since we are going down in y-value, it is negative
                            cv::Vec3f point = cv::Vec3f(c,0-r,0);
                            point_set.push_back(point);
                        }
                    }
                }
                // Set the boolean to true
                points_calculated = true;
            }

		
	    if (cornersFound && calib_counter < 6) {
		// Counter string between 1 and 5 for image naming
	        std::string counterString = std::to_string(calib_counter);
	        std::string calibImgName = "calib"+counterString+".jpg";

		// Save the image
	        cv::imwrite(calibImgName, frame);

	        // Set the index of the vectors to the sets for the calibration images chosen
		point_list.push_back(point_set);
		corner_list.push_back(corner_set);

		// Alert the user to calibration being complete
		if (calib_counter == 5) {
		    std::cout << "Calibration complete." << std::endl;
		    std::cout << "Any additional snapshots will not be recorded." << std::endl;
		}
		
		// Increment the counter
		++calib_counter;
	    }
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

