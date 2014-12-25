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

    bool initWebcam(int i);
    bool takePicture2(Mat& destination);
    bool initDataSetReadProps(QString firstImageLoc);
    bool readNext2(Mat& destination);

    static void differenceMin2(Mat& maskMin, float &numDiff, const Mat& oldImg, const Mat& newImg, int thresh, int size);
    static void shrinkMaskMin2(Mat& filteredDiffs, float& numDiff, const Mat& origDiffs);
    static Mat extendMaskMinToEdges2(const Mat& orig);
    static Mat sweepDownMin2(const Mat& orig);
    static Mat keepWhiteMaskMin2(const Mat& orig);
    static Mat growMin2(const Mat& orig, int size);
    static Mat growMin3(const Mat& orig, int size);
    static Mat findContoursMaskMin2(const Mat& orig);
    static Mat maskMinToMaskBinary2(const Mat& orig);
    static Mat maskMinToMaskBinary3(const Mat& orig, int SCALE);
    static Mat blur2(const Mat& orig, int size);
    static Mat thresholdOnBlue(const Mat& orig, int blueThresh, int size);
    static Mat updateBack3(const Mat& oldWboardModel, const Mat& newInfo, const Mat& mvmtInfo);
    static Mat averageWhiteboard2(const Mat& orig, int size);
    static Mat enhanceText2(const Mat& orig);

    static Mat dogEdges2(const Mat& orig, int kerSize, int rad1, int rad2);
    static Mat adjustLevels2(const Mat& orig, int lo, int hi, double gamma);
    static Mat binarizeMask2(const Mat& orig, int threshold);
    static int** getConnectedComponents2(const Mat& orig);
    static Mat pDrift2(const Mat& orig);
    static Mat addComponentsFromMask2(const Mat& compsImg, const Mat& edgeImg);
    static Mat findMarker(const Mat& orig);
    static Mat fillMarkerBorders(const Mat& grownEdges);
    static Mat findMarker2(const Mat& orig);
    static Mat darkenText3(const Mat &orig, const Mat& marker);
    static Mat expandDifferencesRegion(const Mat& differences);
    static Mat rectifyImage2(const Mat& orig);
    static Mat findBoard2(Mat& orig);
};

#endif // PAOLMAT_H
