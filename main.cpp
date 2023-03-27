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
#include <fstream>


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

    // Initialize frame, will be streamed from the camera
    cv::Mat frame;

    int calib_counter = 1;

    std::vector<cv::Vec3f> point_set;
    std::vector<std::vector<cv::Vec3f>> point_list;
    std::vector<std::vector<cv::Point2f>> corner_list;

    int pattern_columns = 6;
    int pattern_rows = 9;

    // Calculate the points within the pattern, only done once
    // Create the point set, should be the same for each calibration image  
    // In the form of top left = (0,0,0), to the right is (1,0,0), row below is (0,-1,0)
    for (int c=0; c<pattern_columns; c++) {
        for (int r=0; r<pattern_rows; r++) {
            // For a planar pattern like a chessboard, set z to zero
            // Since we are going down in y-value, it is negative
            cv::Vec3f point = cv::Vec3f(c,0-r,0);
            std::string point_string = "(" + std::to_string(point[0]) + "," + std::to_string(point[1]) + ","
                                           + std::to_string(point[2]) + ")";
            //std::cout << point_string << std::endl;
            point_set.push_back(point);
        }
    }

    double center_col = frame.cols/2;
    double center_row = frame.rows/2;
    double camera_matrix_data[9] = {1, 0, center_col, 0, 1, center_row, 0, 0, 1};
    cv::Mat camera_matrix = cv::Mat(3,3, CV_64FC1, &camera_matrix_data);
    cv::Mat dist_coeffs = cv::Mat::zeros(8, 1, CV_64F);

    for(;;) {
	// Require 20 calibration images
	//if (calib_counter == 17) {
	//    calib_counter = 1;
	//}

        *capdev >> frame; // get a new frame from the camera, treat as a stream

	cv::Mat grayscaleFrame;
	
	// convert the frame to one color channel for ease of use in the cv functions
	cv::cvtColor(frame, grayscaleFrame, cv::COLOR_BGR2GRAY);

        std::vector<cv::Point2f> corner_set;

	int pattern_columns = 9;
	int pattern_rows = 6;

	cv::Size patternsize(pattern_columns,pattern_rows);
	bool cornersFound = false;
	

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
	    
            // Without flags the fps slows considerably
            cornersFound = cv::findChessboardCorners(grayscaleFrame, patternsize, corner_set, cv::CALIB_CB_ADAPTIVE_THRESH
                                                      + cv::CALIB_CB_NORMALIZE_IMAGE + cv::CALIB_CB_FAST_CHECK);
	
	    if (cornersFound) {
                cv::cornerSubPix(grayscaleFrame, corner_set, cv::Size(11,11), cv::Size(-1,-1),
                                 cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 30, 0.1));
            }

            cv::drawChessboardCorners(frame, patternsize, cv::Mat(corner_set), cornersFound);
	
	    if (cornersFound && calib_counter < 21) {
		// Counter string between 1 and 20 for image naming
	        std::string counterString = std::to_string(calib_counter);
	        std::string calibImgName = "calib"+counterString+".jpg";
		std::string remainder_string = std::to_string(20-calib_counter);
		std::cout << "Calibration image saved" << " " + remainder_string + " remaining." << std::endl;

		// Save the image
	        cv::imwrite(calibImgName, frame);

	        // Set the index of the vectors to the sets for the calibration images chosen
		point_list.push_back(point_set);
		corner_list.push_back(corner_set);
		
		//std::cout << point_set.size() << std::endl;
		//std::cout << corner_set.size() << std::endl;


		// Alert the user to calibration being complete
		if (calib_counter == 20) {
		    std::cout << "Calibration image collection complete." << std::endl;
		    std::cout << "Any additional snapshots will not be recorded." << std::endl;
		    std::cout << "Press 'c' to calibrate the camera ..." << std::endl << std::endl;
		    //std::cout << point_list.size() << std::endl;
		    //std::cout << corner_list.size() << std::endl;
		}
		
		// Increment the counter
		++calib_counter;
	    }
	}
	// If the user get's all 5 calibration images they are ready to run calibration
	else if( key == 'c' && calib_counter >= 20) {

	    std::vector<cv::Mat> rvecs, tvecs;

	    // Make the camera_matrix a 3x3 cv::Mat of type CV_64FC1
            // The 64F type is a double
    	    // You will want to initialize the 3x3 camera_matrix to something like
            // [1, 0, frame.cols/2]
            // [0, 1, frame.rows/2]
            // [0, 0, 1]
	    //double center_col = frame.cols/2;
	    //double center_row = frame.rows/2;
	    //double camera_matrix_data[9] = {1, 0, center_col, 0, 1, center_row, 0, 0, 1};
	    //cv::Mat camera_matrix = cv::Mat(3,3, CV_64FC1, &camera_matrix_data); 
	    //(camera_matrix << 1, 0, frame.cols/2, 0, 1, frame.rows/2, 0, 0, 1);
	    //std::cout << camera_matrix.size() << std::endl;
	    
	    // Initialize the distortion matrix to zeros
	    //cv::Mat dist_coeffs = cv::Mat::zeros(8, 1, CV_64F);

	    std::cout << "Camera matrix pre-calibration: " << std::endl << " " << camera_matrix << std::endl << std::endl;
            std::cout << "Distortion coefficients pre-calibration: " << std::endl << " " << dist_coeffs << std::endl << std::endl;	

	    // Use the cv::calibrateCamera function to generate the calibration
	    // The parameters to the function are the point_list, corner_list, the size of the calibration images,
            // the camera_matrix, the distortion_coefficients, the rotations, and the translations.
	    // The function also has arguments for information about how well each parameter was estimated.
	    // You may want to use the flag CV_CALIB_FIX_ASPECT_RATION, which specifies that the pixels are assumed to be square 
            // Radial distortion is optional, and you may want to have it turned off at first.
	    double rms = cv::calibrateCamera(point_list, corner_list, frame.size(), camera_matrix, dist_coeffs, rvecs, tvecs,
					     cv::CALIB_FIX_ASPECT_RATIO + cv::CALIB_FIX_INTRINSIC + cv::CALIB_FIX_PRINCIPAL_POINT);

	    std::cout << "Camera matrix post-calibration: " << std::endl << " " << camera_matrix << std::endl << std::endl;
	    std::cout << "Distortion coefficients post-calibration: " << std::endl << " " << dist_coeffs << std::endl << std::endl;

	    // The average re-projection error 
	    std::cout << "Re-projection error: " << rms << std::endl << std::endl;

	    // Display next step to user
	    std::cout << "Calibration complete, press 'w' to create save calibration" << std::endl;

	    // Print out the camera matrix and distortion coefficients before and after the calibration,
	    // along with the final re-projection error. The two focal lengths should be the same value,
	    // and the u0, v0 values should be close to the initial estimates of the center of the image.
	    // Your error should be less than a half-pixel, if everything is working well. 
	    // For large images or cell phone images, the per-pixel error may be more like 2-3 pixels.
	    // Include the error estimate in your report.

	    // The calibrateCamera function also returns the rotations and translations associated with each calibration image.
	    // If you saved the calibration images, you might also want to save these rotations and translations with them.
	    
	    // Enable the user to write out the intrinsic parameters to a file: both the camera_matrix and the distortion_coefficients.

	}
	// Writing a text file to be used for calibration params
	else if( key == 'w') {
	    //Write the intrinsic parameters to a file
	    std::ofstream param_file("calibration.txt");
	    if (param_file.is_open()) {
		param_file << "Camera matrix: " << std::endl;
		param_file << camera_matrix << std::endl << std::endl;
		param_file << "Distortion coefficients: " << std::endl;
		param_file << dist_coeffs << std::endl;

		std::cout << "Calibration saved, press 'r' to read file and detect chessboard pose.";	

	 	param_file.close();

	    }
	    else std::cout << "Unable to write parameters to file." << std::endl;
	}

	else if (key == 'r') {
	    std::ifstream file;
	    file.open("calibration.txt");

	    // if the file exists
	    if (file) {
	    	// Read in the file
            
		// detect a chessboard in the live video 
            	// if (findChessboardCorners) {
                    //grabs corner locations, 
                    // solvePNP to get board pose (roation and translation)
                    // print out rotation and translation data in real time
                //}
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

