//
// main.cpp
//
#include <opencv2/opencv.hpp> // OpenCV head file

int main() {

    // create a video capsure object
    // OpenCV offers VideoCapture object and 
    // treat reading video from file as same as reading from camera.
    // when input parameter is a file path, it will read a video file;
    // if it is a identifier number of camera (usually it is 0), 
    // it will read the camera
    cv::VideoCapture video("video.ogv"); // reading from file
    // cv::VideoCapture video(0);        // reading from camera

    // container for the reading image frame, Mat object in OpenCV
    // The key class in OpenCV is Mat, which means Matrix
    // OpenCV use matrix to describe images
    cv::Mat frame;
    while(true) {

        // write video data to frame, >> is overwrited by OpenCV
        video >> frame;

        // when there is no frame, break the loop
        if(frame.empty()) break;

        // visualize current frame
        cv::imshow("test", frame);

        // video frame rate is 15, so we need wait 1000/15 for playing smoothly
        // waitKey(int delay) is a waiting function in OpenCV
        // at this point, the program will wait `delay` milsec for keyboard input
        int key = cv::waitKey(1000/15);

        // break the loop when click ECS button on keyboard
        if (key == 27) break;
    }
    // release memory
    cv::destroyAllWindows();
    video.release();
    return 0;

}