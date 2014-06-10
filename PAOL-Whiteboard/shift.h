#ifndef SHIFT_H
#define SHIFT_H

#include <vector>
#include <QtCore>

using namespace std;

class Shift
{
public:
    vector<int> yVals;
    vector<int> xVals;
    vector<vector<int> > points;
    int minX,minY,maxX,maxY;
    bool rangeFound;


    Shift();
    ~Shift();
    void addPoint(int x,int y);
    void printPoints();
    void sortY();
    void sortX();
    void setWindow();
    int getMinX();
    int getMaxX();
    int getMinY();
    int getMaxY();
    void removeY(int y);
    void removeX(int x);
    bool notReady();
    void cleanPoints();
};

#endif // SHIFT_H
