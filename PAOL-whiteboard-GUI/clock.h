#ifndef CLOCK_H
#define CLOCK_H

class Clock {
private:
    long int startTime;

    long int getCurrentTimeMS();

public:
    Clock();

    void resetClock();
    long int getElapsedTime();
};

#endif // CLOCK_H
