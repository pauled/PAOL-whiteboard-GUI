#include "uf.h"
#include <map>

/*
 * Adapted from kartik kukreja's implementation available at:
 * https://github.com/kartikkukreja/blog-codes/blob/master/src/Union%20Find%20(Disjoint%20Set)%20Data%20Structure.cpp
 */

UF::UF() {
    id = std::map<int,int>();
    sz = std::map<int,int>();
}

void UF::addClass(int c) {
    id[c] = c;
    sz[c] = 1;
}

int UF::find(int p) {
    int root = p;
    while (root != id[root])
        root = id[root];
    while (p != root) {
        int newp = id[p];
        id[p] = root;
        p = newp;
    }
    return root;
}

void UF::merge(int x, int y) {
    int i = find(x);
    int j = find(y);
    if (i == j) return;

    // make smaller root point to larger one
    if (sz[i] < sz[j])	{
        id[i] = j;
        sz[j] += sz[i];
    } else	{
        id[j] = i;
        sz[i] += sz[j];
    }
}

bool UF::connected(int x, int y) {
    return find(x) == find(y);
}
