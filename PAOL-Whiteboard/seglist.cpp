#include "seglist.h"

SegList::SegList()
{
    ccount=0;
    rcount=-1;
    sList.push_back(0);
}

SegList::~SegList(){
    sList.clear();
}

int SegList::find(int x){
    if (sList[x]==x)
        return x;
    else {
        sList[x]=find(sList[x]);
        return sList[x];
    }
}

int SegList::merge(int u, int l){
    int uv,lv;

    uv=find(u);
    lv=find(l);
    if(uv<lv){
        sList[lv]=uv;
        return uv;
    } else {
        sList[uv]=lv;
        return lv;
    }
}

int SegList::newPoint(){
    ccount++;
    sList.push_back(ccount);
    return ccount;
}

void SegList::update(){
    rcount=0;
    int x;
    for (x=1;x<sList.size();x++){
        if(sList[x]==x){
            rcount++;
            sList[x]=rcount;
        } else {
            sList[x]=sList[sList[x]];
        }
    }
}

void SegList::print(){
    for (int x=0;x<sList.size();x++){
        qDebug("[%d]->%d",x,sList[x]);
    }
}

int SegList::getCount(){
    return rcount;
}

int SegList::getValue(int x){
    return sList[x];
}

