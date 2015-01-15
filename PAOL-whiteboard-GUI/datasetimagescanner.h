#ifndef DATASETIMAGESCANNER_H
#define DATASETIMAGESCANNER_H

#include "ImageScanner.h"
#include <string>

using namespace std;

class DatasetImageScanner : public ImageScanner
{
private:
    // Maximum number of seconds to skip in data set if the next frame is missing
    static const int TIME_SKIP_LIMIT = 200;
    // Maximum number of indices to skip in data set if the next frame is missing
    static const int INDEX_SKIP_LIMIT = 10;

    // The location of the data set
    string datasetPath;
    // The index of the next frame to read
    // (ie. ## from cameraIn##)
    int nextFrameIndex;
    // The timestamp of the next frame to read
    int nextFrameTime;
    // The data set camera number
    int datasetCamNum;

public:
    DatasetImageScanner(string firstImagePath);
    bool getNextImage(cv::Mat &destination);
};

#endif // DATASETIMAGESCANNER_H
