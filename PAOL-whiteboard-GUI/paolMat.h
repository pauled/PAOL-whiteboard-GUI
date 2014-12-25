#ifndef PAOLMAT_H
#define PAOLMAT_H

//Including C++ Libs
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <QLabel>
#include <QMainWindow>
#include <QtCore>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <QString>

#include <vector>
#include <QFileDialog>

/*
#include <iostream>
#include <iterator>
#include <queue>
#include <algorithm>
#include <cstdio>
#include <ctime>
*/

using namespace cv;

class paolMat
{
private:

    /// Constants
    // Scaling factor for processing on smaller versions of whiteboard
    static const int SCALE = 8;
    // How many frames to search for in data set if the next frame is missing
    static const int TIME_SKIP_LIMIT = 300;

    /// Webcam object
    VideoCapture cam;

    /// Image read variables
    // The location of the data set
    string dataSetDir;
    // The index of the next frame to read
    // (ie. ## from cameraIn##)
    int nextFrameIndex;
    // The timestamp of the next frame to read
    int nextFrameTime;
    // The data set camera number
    int datasetCamNum;

public:
    ~paolMat();

    /// Methods to read images from file or webcam
    bool initWebcam(int i);
    bool takePictureFromWebcam(Mat& destination);
    bool initDataSetReadProps(QString firstImageLoc);
    bool readNextInDataSet(Mat& destination);

    /// Methods to find and process differences (ie. find the lecturer)
    static void findAllDiffsMini(Mat& diffLocations, float& percentDiff, const Mat& oldImg, const Mat& newImg, int thresh, int size);
    static void filterNoisyDiffs(Mat& filteredDiffs, float& percentDiff, const Mat& origDiffs);
    static Mat replicateToImageEdges(const Mat& orig);
    static Mat sweepDown(const Mat& orig);
    static Mat borderWithGreen(const Mat& orig, int size);
    static Mat grow(const Mat& orig, int size);
    static Mat getImageContours(const Mat& orig);
    static Mat enlarge(const Mat& orig);
    static Mat expandDifferencesRegion(const Mat& differences);

    /// Methods to find the marker strokes
    // Used for both old method (marker borders) and new method (DoG and connected components)
    static Mat binarize(const Mat& orig, int threshold);
    static Mat thresholdOnBlueChannel(const Mat& orig, int blueThresh, int size);
    static Mat pDrift(const Mat& orig);

    // Used only for old method
    static Mat fillMarkerBorders(const Mat& grownEdges);
    static Mat findMarkerWithMarkerBorders(const Mat& orig);

    // Used only for new method
    static Mat getDoGEdges(const Mat& orig, int kerSize, int rad1, int rad2);
    static Mat adjustLevels(const Mat& orig, int lo, int hi, double gamma);
    static int** getConnectedComponents(const Mat& orig);
    static Mat filterConnectedComponents(const Mat& compsImg, const Mat& edgeImg);
    static Mat findMarkerWithCC(const Mat& orig);

    /// Methods to enhance the whiteboard
    static Mat boxBlur(const Mat& orig, int size);
    static Mat getAvgWhiteboardColor(const Mat& orig, int size);
    static Mat enhanceText(const Mat& orig);
    static Mat whitenWhiteboard(const Mat &orig, const Mat& marker);
    static Mat rectifyImage(const Mat& orig);
    static Mat findWhiteboardBorders(Mat& orig);

    /// Update the background (whiteboard) model
    static Mat updateWhiteboardModel(const Mat& oldWboardModel, const Mat& newInfo, const Mat& mvmtInfo);
};

#endif // PAOLMAT_H
