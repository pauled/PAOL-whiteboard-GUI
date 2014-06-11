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

    int scale;//scale factor for maskMin

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
    void displayImage(QLabel& location);
    void displayMask(QLabel& location);
    void displayMaskMin(QLabel& location);

    //maskMin methods
    float differenceMin(paolMat *img, int thresh, int size);
    float shrinkMaskMin();
    void extendMaskMinToEdges();
    void sweepDownMin();
    void keepWhiteMaskMin();
    void growMin(int size);
    void findContoursMaskMin();
    void maskMinToMaskBinary();

    //src and mask methods
    //blur size is pixels adjacent i.e. 1 would be a 3x3 square centered on each pixel
    void blur(int size);
    //pDrift is y+-1 x+-1
    void pDrift();
    void grow(int blueThresh, int size);
    void nontextToWhite();
    void updateBackgroundMaskMin(paolMat *m, paolMat *foreground);
    void updateBack2(paolMat *foreground, paolMat *edgeInfo);
    void processText(paolMat *m);
    void darkenText();
    void averageWhiteboard(int size);
    void enhanceText();
    float countDifsMask(paolMat *newIm);
};

#endif // PAOLMAT_H
