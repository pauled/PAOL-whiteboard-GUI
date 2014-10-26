#ifndef UF_H
#define UF_H

#include <map>

class UF    {
    std::map<int,int> id;
    std::map<int,int> sz;
public:
    // Create an empty union find data structure with N isolated sets.
    UF();
    // Add new class
    void addClass(int c);
    // Return the id of component corresponding to object p.
    int find(int p);
    // Replace sets containing x and y with their union.
    void merge(int x, int y);
    // Are objects x and y in the same set?
    bool connected(int x, int y);
};
#endif // UF_H
