#ifndef IMAGESCANNER_H
#define IMAGESCANNER_H

#include <opencv2/core/core.hpp>

class ImageScanner {
public:
    virtual ~ImageScanner() {}
    // Get the next image and timestamp from the scanner, as well as the scanner's device number
    // Arguments:
    //    destination: Where to store the read image
    //    frameTime: Where to store the timestamp of the read image
    //    devNum: Where to store the device number that the scanner reads from,
    //            ie. the webcam number
    virtual bool getNextImage(cv::Mat& destination) = 0;
};

#endif // IMAGESCANNER_H
