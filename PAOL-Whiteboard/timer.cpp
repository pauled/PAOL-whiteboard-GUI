#include "timer.h"

//constructor for Timer
Timer::Timer(){
    time(&startTime);
}

//destructor for Timer
Timer::~Timer(){}

void Timer::printTime(int row,int end){
  int currentTime,totalTime,timeLeft;
  int ch,cm,cs,th,tm,ts,lh,lm,ls;

  time(&cTime);
  currentTime=cTime-startTime;
  totalTime=end*currentTime/row;
  timeLeft=totalTime-currentTime;

  //set hours
  ch=currentTime/3600;
  th=totalTime/3600;
  lh=timeLeft/3600;

  currentTime%=3600;
  totalTime%=3600;
  timeLeft%=3600;

  //set minutes
  cm=currentTime/60;
  tm=totalTime/60;
  lm=timeLeft/60;

  currentTime%=60;
  totalTime%=60;
  timeLeft%=60;

  //set seconds
  cs=currentTime;
  ts=totalTime;
  ls=timeLeft;

  qDebug("%d/%d current: %d:%02d.%02d total: %d:%02d.%02d   left: %d:%02d.%02d\n",row,end,ch,cm,cs,th,tm,ts,lh,lm,ls);
}
