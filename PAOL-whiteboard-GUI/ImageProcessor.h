#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <opencv2/core/core.hpp>

using namespace cv;

class ImageProcessor {
public:
    virtual ~ImageProcessor() {}
    // Process the next image. If it was successfully processed (ie. there
    // were enough differences from the previous frame), return the processed
    // frame. Otherwise, return an empty Mat.
    virtual Mat processCurFrame(const Mat& currentFrame) = 0;
    // Reset the state of this processor
    virtual void reset() = 0;
};

#endif // IMAGEPROCESSOR_H
