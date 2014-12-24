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
public:
    VideoCapture cam;

    //image data
    Mat src;
    Mat mask;
    Mat maskMin;//mask shrunk by scale factor squared
    Mat displayMin;
    Mat display;

    static const int scale = 8;//scale factor for maskMin

    //image read variables
    int cameraNum;
    char readName[256];
    int countRead;
    int time;
    std::string dirOut;

    paolMat();
    ~paolMat();
    paolMat(paolMat* m);
    void copy(paolMat* m);
    void copyClean(paolMat* m);
    void copyMask(paolMat* m);
    void copyMaskMin(paolMat* m);

    void setCameraNum(int i);
    void takePicture();
    bool readNext(QWidget *fg);

    QImage convertToQImage();
    QImage convertMaskToQImage();
    QImage convertMaskMinToQImage();
    static QImage convertMatToQImage(const Mat& mat);
    static void displayMat(const Mat& mat, QLabel &location);
    void displayImage(QLabel& location);
    void displayMask(QLabel& location);
    void displayMaskMin(QLabel& location);

    //maskMin methods
    float differenceMin(paolMat *img, int thresh, int size);
    static void differenceMin2(Mat& maskMin, float &numDiff, const Mat& oldImg, const Mat& newImg, int thresh, int size);
    float shrinkMaskMin();
    static void shrinkMaskMin2(Mat& filteredDiffs, float& numDiff, const Mat& origDiffs);
    void extendMaskMinToEdges();
    static Mat extendMaskMinToEdges2(const Mat& orig);
    void sweepDownMin();
    static Mat sweepDownMin2(const Mat& orig);
    void keepWhiteMaskMin();
    static Mat keepWhiteMaskMin2(const Mat& orig);
    void growMin(int size);
    static Mat growMin2(const Mat& orig, int size);
    static Mat growMin3(const Mat& orig, int size);
    void findContoursMaskMin();
    static Mat findContoursMaskMin2(const Mat& orig);
    void maskMinToMaskBinary();
    static Mat maskMinToMaskBinary2(const Mat& orig);
    static Mat maskMinToMaskBinary3(const Mat& orig, int scale);

    //src and mask methods
    //blur size is pixels adjacent i.e. 1 would be a 3x3 square centered on each pixel
    void blur(int size);
    static Mat blur2(const Mat& orig, int size);
    //pDrift is y+-1 x+-1
    void pDrift();
    void grow(int blueThresh, int size);
    static Mat thresholdOnBlue(const Mat& orig, int blueThresh, int size);
    void nontextToWhite();
    void updateBackgroundMaskMin(paolMat *m, paolMat *foreground);
    void updateBack2(paolMat *foreground, paolMat *edgeInfo);
    static Mat updateBack3(const Mat& oldWboardModel, const Mat& newInfo, const Mat& mvmtInfo);
    void processText(paolMat *m);
    void darkenText();
    void averageWhiteboard(int size);
    void enhanceText();
    void dogEdges(int kerSize, int rad1, int rad2);
    void adjustLevels(int lo, int hi, double gamma);
    void invert();
    int **getConnectedComponents();
    void addComponentsFromMask(int **components);
    void binarizeMask(int threshold);
    void binarizeSrc(int threshold);
    void blurSrc(int blurRad);
    void laplaceEdges();
    void darkenText2(Mat marker);

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

    float countDifsMask(paolMat *newIm);

    void rectifyImage(paolMat *m);
    static Mat rectifyImage2(const Mat& orig);
    void findBoard(paolMat *m);
};

#endif // PAOLMAT_H
