#ifndef IMAGESCANNER_H
#define IMAGESCANNER_H

#include <opencv2/core/core.hpp>

class ImageScanner {
protected:
    // Boolean for whether to print normal (ie. not warning or error) debug statements
    bool printDebug;
public:
    // Constructor and destructor, so we can delete pointers to ImageScanners
    ImageScanner();
    virtual ~ImageScanner() {}

    // Get the next image and timestamp from the scanner, as well as the scanner's device number
    // Arguments:
    //    destination: Where to store the read image
    //    frameTime: Where to store the timestamp of the read image
    //    devNum: Where to store the device number that the scanner reads from,
    //            ie. the webcam number
    virtual bool getNextImage(cv::Mat& destination) = 0;

    // Set whether to print debug statements
    void setPrintDebug(bool toPrint);
};

#endif // IMAGESCANNER_H
