#include "clock.h"
#include <QtCore>
#include <sys/time.h>
#include <stdexcept>

Clock::Clock()
{
    resetClock();
}

void Clock::resetClock() {
    startTime = getCurrentTimeMS();
}

long int Clock::getElapsedTime() {
    if(startTime != -1)
        return getCurrentTimeMS() - startTime;
    else {
        qWarning("Toc was called before tic");
        return -1;
    }
}

long int Clock::getCurrentTimeMS() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}
