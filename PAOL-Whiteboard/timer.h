#ifndef TIMER_H
#define TIMER_H

#include <time.h>
#include <QtCore>

class Timer
{
public:
    time_t startTime,cTime;

    Timer();
    ~Timer();
    void printTime(int row,int end);
};

#endif // TIMER_H
