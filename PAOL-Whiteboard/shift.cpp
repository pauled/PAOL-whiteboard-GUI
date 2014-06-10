#include "shift.h"

Shift::Shift()
{
    minX=0;
    minY=0;
    maxX=0;
    maxY=0;
    rangeFound=false;
}

Shift::~Shift(){
    xVals.clear();
    yVals.clear();
    points.clear();
}

void Shift::addPoint(int x, int y){
    vector<int> pt;

    pt.push_back(x);
    pt.push_back(y);
    points.push_back(pt);
}

void Shift::printPoints(){
    for (unsigned int i=0; i<points.size(); i++){
        qDebug("(%d,%d)",points[i][0],points[i][1]);
    }
    qDebug("x=(%d,%d) y=(%d,%d)",minX,maxX,minY,maxY);
}

int Shift::getMaxX(){
    return maxX;
}
int Shift::getMinX(){
    return minX;
}

int Shift::getMaxY(){
    return maxY;
}
int Shift::getMinY(){
    return minY;
}
void Shift::sortY(){
    int i;
    bool dif=true;

    for (i=0; i<points.size(); i++){
        yVals.push_back(points[i][1]);
    }
    sort(yVals.begin(),yVals.end());

    for (i=yVals.size()-1; i>0; i--){
        if (yVals[i]!=yVals[i-1]){
            if (dif){
                qDebug("remove %d",yVals[i]);
                removeY(yVals[i]);
                yVals.erase(yVals.begin()+i);
            }
            dif=true;
        } else {
            dif=false;
        }

    }

    if (yVals[1]!=yVals[0] && dif){
        removeY(yVals[0]);
        yVals.erase(yVals.begin());
    }
    maxY=yVals[yVals.size()-1];
    minY=yVals[0];
}
void Shift::removeY(int y){
    int i;

    for(i=points.size()-1; i>=0; i--){
        if (points[i][1]==y)
            points.erase(points.begin()+i);
    }
}

void Shift::sortX(){
    int i;
    bool dif=true;

    for (i=0; i<points.size(); i++){
        xVals.push_back(points[i][0]);
    }
    sort(xVals.begin(),xVals.end());

    for (i=xVals.size()-1; i>0; i--){
        if (xVals[i]!=xVals[i-1]){
            if (dif){
                qDebug("remove %d",xVals[i]);
                removeX(xVals[i]);
                xVals.erase(xVals.begin()+i);
            }
            dif=true;
        } else {
            dif=false;
        }

    }

    if (xVals[1]!=xVals[0] && dif){
        removeX(xVals[0]);
        xVals.erase(xVals.begin());
    }
    maxX=xVals[xVals.size()-1];
    minX=xVals[0];
    rangeFound=true;
}
void Shift::removeX(int X){
    int i;

    for(i=points.size()-1; i>=0; i--){
        if (points[i][0]==X)
            points.erase(points.begin()+i);
    }
}
bool Shift::notReady(){
    return !rangeFound;
}
void Shift::cleanPoints(){
    xVals.clear();
    yVals.clear();
    points.clear();
}
