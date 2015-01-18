#include "datasetimagescanner.h"
#include <QtCore>
#include <opencv2/highgui/highgui.hpp>
#include <stdexcept>

using namespace cv;

// Given the path of the first image in the data set, construct a scanner to iterate
// through the data set
DatasetImageScanner::DatasetImageScanner(string firstImagePath)
{
    // Handle case where the given path is empty
    if(firstImagePath.length() == 0) {
        throw std::invalid_argument("The given file path was empty.");
    }

    // Get the data set directory by cutting off whatever is after the last "/"
    datasetPath = firstImagePath.substr(0, firstImagePath.find_last_of("/"));

    // Parse file name and get time and index of the next frame to read (ie. of the given image)
    // Set format of the first image location and use it to scan for next frame index and time
    string scanFormat = datasetPath + "/cameraIn%06d-%10d-%d.png";
    int scanResult = sscanf(firstImagePath.c_str(), scanFormat.c_str(),
                            &nextFrameIndex, &nextFrameTime, &datasetCamNum);

    if(scanResult != 3) {
        throw std::invalid_argument("The given file path was not formatted properly.");
    }
}

bool DatasetImageScanner::getNextImage(Mat &destination) {
    // Attempt to read the next file using the stored time and index of the next frame
    char nextFrameLoc[256];
    Mat temp;

    // Set the next index to search for up to INDEX_SKIP_LIMIT times
    for(int indexSkipCount = 0; indexSkipCount < INDEX_SKIP_LIMIT; indexSkipCount++) {
        int tempFrameIndex = nextFrameIndex + indexSkipCount;
        // Set the next timestamp to search for up to TIME_SKIP_LIMIT times
        for(int timeSkipCount = 0; timeSkipCount < TIME_SKIP_LIMIT; timeSkipCount++) {
            int tempFrameTime = nextFrameTime + timeSkipCount;

            // Attempt to read the file with the temp frame index and time
            sprintf(nextFrameLoc, "%s/cameraIn%06d-%10d-%d.png",
                    datasetPath.c_str(), tempFrameIndex, tempFrameTime, datasetCamNum);
            temp = imread(nextFrameLoc, CV_LOAD_IMAGE_COLOR);

            // Update destination, next frame index and time if read was successful
            if(temp.data) {
                // Save image
                destination = temp.clone();

                // Update next frame time and index
                nextFrameIndex = tempFrameIndex + 1;
                nextFrameTime = tempFrameTime + 1;

                if(printDebug)
                    qDebug("Successfully read %s", nextFrameLoc);
                return true;
            }
        }
    }
    if(printDebug)
        qDebug("Failed to find the next file.");
    return false;
}
