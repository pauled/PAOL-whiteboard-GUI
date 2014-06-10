#ifndef PAOLMAT_H
#define PAOLMAT_H

//Including C++ Libs
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
#include "timer.h"
#include "shift.h"
#include "seglist.h"
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
    VideoWriter outVid;
    SegList *sList;

    Mat src;
    Mat mask;
    Mat maskMin;
    Mat displayMin;
    Mat display;
    int count;
    int time;
    int difs;
    int scale;
    std::string name;
    Point camera;
    Point prof;
    bool lectFound;
    int r,g,b;

    Shift *disp;
    char readName[256];
    int countRead;
    int cameraNum;
    std::string dirOut;

    paolMat();
    ~paolMat();
    paolMat(paolMat* m);

    void setCameraNum(int i);
    void openVideo(QWidget *fg);
    void takePicture();
    bool getFrame();
    bool readNext(QWidget *fg);
    double getFrameRate();
    QImage convertToQImage();
    QImage convertMaskToQImage();
    QImage convertMaskMinToQImage();
    void displayImage(QLabel& location);
    void displayMask(QLabel& location);
    void displayMaskMin(QLabel& location);
    void toChromaticity();
    void toHSV();
    void maskToRed();
    void maskMinToMaskBinary();
    void updateBack2(paolMat *m);
    void updateBack2Min(paolMat *m);
    void contoursToMask();
    void findContoursMask();
    void findContoursMaskMin();
    void loadImage(QWidget *fg);
    void saveSrc(QWidget *fg);

    void copy(paolMat* m);
    void copyNoSrc(paolMat *m);
    void copyMask(paolMat *m);
    void maskToSrc();
    void binarize(paolMat *in);
    void connectedComponent(paolMat *im, bool binary);
    void colorConnected(paolMat *connected);
    int comparePix(paolMat *im,int x1,int y1,int x2,int y2, bool binary,int thresh);
    bool pixSame(paolMat *im,int x1,int y1,int x2,int y2,int thresh);
    int pixToNum(int x,int y);
    void numToPix(int x,int y,int num);
    void copyInDif(paolMat *in);
    void read(std::string fullName, std::string fileName, int countIn, int timeIn);
    void capture(CvCapture* capture, int count);
    void write();
    void write(std::string outDir);
    void write2(std::string outDir,std::string nameOut,int camNum);
    void writeByCount(std::string outDir);
    void writeMask();
    void print();
    void edges();
    paolMat *returnEdges();
    void invert();
    void blockDiff(paolMat *imIn);
    void createBackgroundImg(int kernalSize);
    paolMat *returnCreateBackgroundImg(int kernalSize);
    void improveInputImg(paolMat *background);
    paolMat *returnImproveInputImg(paolMat *background);
    void removeProf(paolMat *oldImg);
    paolMat *returnRemoveProf(paolMat *oldImg);
    void createContrast();
    paolMat *returnCreateContrast();
    void sharpen();
    paolMat *returnSharpen();
    void shrink();
    paolMat *returnShrink();
    //One to One comparison of colors in each pixel of src
    void difference(paolMat *img);
    //Ont to One comparison of the colors in each pixel of mask
    void maskDifference(paolMat *img);
    //difference for computer proc
    float difference(paolMat *img, int thresh, int size, int maskBottom);
    float differenceMin(paolMat *img, int thresh, int size, int maskBottom);
    // thresh: old rgb - new rgb > thresh // size: downsample x/y+=size
    void differenceLect(paolMat *inImg, int thresh, int size);
    void localizeSpeaker();
    void decimateMask();
    void decimateMask(int thresh);
    //Grow mask by adding values to bordering pixels
    void growMask();
    void shrinkMask();
    void shrinkMask2();
    void shrinkMaskMin2();
    void connected();
    //Must be the same size as differenceLect
    void connected(int size);
    void keepMask(int blueThresh);
    void lectArea();
    paolMat *crop(int x, int y, int width, int height);
    paolMat *cropFrame(int width, int height);
    vector<int> vertMaskHistogram();
    vector<int> horMaskHistogram();
    void decimateMaskByHistogram(int hThresh, int vThresh);
    //drift is y+1, x+1
    void drift();
    void driftWAverage();
    void sweepMask();
    void sweepDown();
    void sweepDownMin();
    void keepWhite();
    void keepWhiteMin();
    //Scan left to right, top to botton toggling mask everytime the src color changes
    void intensityMask(int thresh);
    //Scan a rectangle around a pixel and change it to white,black, or red.
    void maskToWhite(int thresh);
    void average();
    void blackSrcByMask();
    //blur size is pixels adjacent i.e. 1 would be a 3x3 square centered on each pixel
    void blur(int size);
    //pDrift is y+-1 x+-1
    void pDrift();
    //Grow by blue thresh and by size
    void grow(int blueThresh, int size);
    void growMin(int blueThresh, int size);
    //Shrink by blue thresh and size
    void shrink(int blueThresh, int size);
    //threshedDifference, only where both masks blue > 30
    void threshedDifference(paolMat *old);
    void getCombine(paolMat *img);
    void blackMaskByMask(paolMat *img);
    void updateBackground(paolMat *alt, paolMat *img);
    void updateBackground();
    void darken();
    void cleanBackground(paolMat *img);
    void differenceDarken(paolMat *img);
    void maskGrowRed(int size);
    void countDiffsMasked(paolMat *img);
    void finalWBUpdate(paolMat *current);

};

#endif // PAOLMAT_H
