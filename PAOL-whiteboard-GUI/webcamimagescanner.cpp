#include "webcamimagescanner.h"
#include <stdexcept>
#include <QtCore>

// Given the webcam number, construct a scanner to pull images from the webcam
WebcamImageScanner::WebcamImageScanner(int devNum)
{
    // Try to initialize the webcam, throw an exception if it fails
    // Reject negative input values
    if(devNum < 0) {
        throw std::invalid_argument("WebcamImageScanner(): The given device number was negative.");
    }

    // Initialize the camera and check if it is valid
    cam = VideoCapture(devNum);
    if(!cam.isOpened()) {
        throw std::invalid_argument("WebcamImageScanner(): Failed to open the webcam with the given device number.");
    }

    // Set webcam number
    webcamNum = devNum;

    //set parameters for camera
    cam.set(CV_CAP_PROP_FRAME_WIDTH, 1920);
    cam.set(CV_CAP_PROP_FRAME_HEIGHT, 1080);
}

WebcamImageScanner::~WebcamImageScanner() {
    if(cam.isOpened())
        cam.release();
}

bool WebcamImageScanner::getNextImage(Mat& destination, int& frameTime, int& devNum) {
    Mat temp;
    //grab 5 consecutive images to clear camera buffer
    for (int i = 0; i < 5;i++) {
        cam >> temp;
    }
    if(temp.data) {
        // Set device number
        devNum = webcamNum;

        // Set current frame time
        time_t curTime;
        time(&curTime);
        frameTime = (int)curTime;

        // Copy image to destination
        destination = temp.clone();

        qDebug("Successfully took a webcam picture.");
        return true;
    }
    else {
        qWarning("Failed to take a webcam picture.");
        return false;
    }
}
