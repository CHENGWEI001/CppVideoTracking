#include <opencv2/opencv.hpp>

bool selectObject = false; // use for whether selected object or not
int trackObject = 0;       // 1 means has a tracking object, 0 means no object, and -1 means haven't calculated the Camshift property
cv::Rect selection;        // save selected region by mouse
cv::Mat image;             // cache image from video

// Callback function of mouse from OpenCV:
// void onMouse(int event, int x, int y, int flag, void *param)
// the fouth parameter `flag` represents additional state, 
// param means user parameter, we don't need them, so, no names.
void onMouse( int event, int x, int y, int, void* ) {
    static cv::Point origin;
    if(selectObject) {
        // determing selected height and width and top-left corner position
        selection.x = MIN(x, origin.x);
        selection.y = MIN(y, origin.y);
        selection.width = std::abs(x - origin.x);
        selection.height = std::abs(y - origin.y);

        // & is overwrited by cv::Rect
        // it means the intersection of two region, 
        // the main purpose here is to process the region outside selected region
        selection &= cv::Rect(0, 0, image.cols, image.rows);
    }

    switch(event) {
            // processing left button is pressed
        case CV_EVENT_LBUTTONDOWN:
            origin = cv::Point(x, y);
            selection = cv::Rect(x, y, 0, 0);
            selectObject = true;
            break;
            // processing left button is released
        case CV_EVENT_LBUTTONUP:
            selectObject = false;
            if( selection.width > 0 && selection.height > 0 )
                trackObject = -1; // tracking object haven't calculate Camshift property
            break;
    }
}

int main( int argc, const char** argv ) {
    cv::VideoCapture video("video.ogv");
    cv::namedWindow("CamShift at LabEx");
    cv::setMouseCallback("CamShift at LabEx", onMouse, NULL);

    cv::Mat frame, hsv, hue, mask, hist, backproj;
    cv::Rect trackWindow;             // tracking window
    int hsize = 16;                   // for histogram
    float hranges[] = {0,180};        // for histogram
    const float* phranges = hranges;  // for histogram

    while(true) {
        video >> frame;
        if(frame.empty()) break;
        frame.copyTo(image);

        // trasfer to HSV space
        cv::cvtColor(image, hsv, cv::COLOR_BGR2HSV);
        // processing when there is an object
        if(trackObject) {

            // only processing H: 0~180?S: 30~256?V: 10~256?filter the others and copy the rest part to mask
            cv::inRange(hsv, cv::Scalar(0, 30, 10), cv::Scalar(180, 256, 256), mask);
            // seperate channel h from hsv
            int ch[] = {0, 0};
            hue.create(hsv.size(), hsv.depth());
            cv::mixChannels(&hsv, 1, &hue, 1, ch, 1);

            // property extract if tracking object haven't been calculated
            if( trackObject < 0 ) {

                // setup channel h and mask ROI
                cv::Mat roi(hue, selection), maskroi(mask, selection);
                // calculate ROI histogram
                calcHist(&roi, 1, 0, maskroi, hist, 1, &hsize, &phranges);
                // normalization of histogram
                normalize(hist, hist, 0, 255, CV_MINMAX);

                // setting tracking object
                trackWindow = selection;

                // mark tracking object has been calculated
                trackObject = 1;
            }
            // back project histogram
            calcBackProject(&hue, 1, 0, hist, backproj, &phranges);
            // fetch common region 
            backproj &= mask;
            // call Camshift algorithm
            cv::RotatedRect trackBox = CamShift(backproj, trackWindow, cv::TermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ));
            // processing region is too small for draw
            if( trackWindow.area() <= 1 ) {
                int cols = backproj.cols, rows = backproj.rows, r = (MIN(cols, rows) + 5)/6;
                trackWindow = cv::Rect(trackWindow.x - r, trackWindow.y - r,
                                       trackWindow.x + r, trackWindow.y + r) & cv::Rect(0, 0, cols, rows);
            }
            // draw tracking area
            ellipse( image, trackBox, cv::Scalar(0,0,255), 3, CV_AA );

        }


        if( selectObject && selection.width > 0 && selection.height > 0 ) {
            cv::Mat roi(image, selection);
            bitwise_not(roi, roi);
        }
        imshow("CamShift at LabEx", image);
        int key = cv::waitKey(1000/15.0);
        if(key == 27) break;
    }
    cv::destroyAllWindows();
    video.release();
    return 0;
}