#ifndef SEGLIST_H
#define SEGLIST_H

#include <vector>
#include <QtCore>

using namespace std;

class SegList
{
public:
    SegList();
    ~SegList();
    int find(int x);
    int merge(int u,int l);
    int newPoint();
    void update();
    void print();
    int getCount();
    int getValue(int x);

private:
    int ccount;
    vector<int> sList;
    int rcount;
};

#endif // SEGLIST_H
