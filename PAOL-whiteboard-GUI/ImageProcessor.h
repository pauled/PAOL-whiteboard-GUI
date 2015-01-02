#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <opencv2/core/core.hpp>

using namespace cv;

class ImageProcessor {
public:
    virtual ~ImageProcessor() {}
    // Process the next image
    virtual void processCurFrame(const Mat& currentFrame, vector<Mat>& frameOutput) = 0;
    // Reset the state of this processor
    virtual void reset() = 0;
};

#endif // IMAGEPROCESSOR_H
