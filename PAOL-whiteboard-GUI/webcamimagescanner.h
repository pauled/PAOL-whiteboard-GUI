#ifndef WEBCAMIMAGESCANNER_H
#define WEBCAMIMAGESCANNER_H

#include "ImageScanner.h"
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

class WebcamImageScanner : public ImageScanner
{
private:
    // Handle to the webcam
    VideoCapture cam;
    // The device number of the webcam
    int webcamNum;

public:
    WebcamImageScanner(int devNum);
    ~WebcamImageScanner();
    bool getNextImage(Mat& destination);
};

#endif // WEBCAMIMAGESCANNER_H
