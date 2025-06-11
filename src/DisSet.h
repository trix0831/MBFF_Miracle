#ifndef DISJOINTSET_H
#define DISJOINTSET_H

#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include "Instance.h"
#include "Net.h"
#include "Pin.h"
#include "Point.h"
using namespace std;

class DisSet
{
public:
    DisSet(Instance *ins, double x, double y)
    {
        instance = &(*ins);
        point = new Point2<double>(x, y);
    }
    ~DisSet() {}
    Instance * getInstances()
    {
        return instance;
    }
    Point2<double> getPoint()
    {
        return *point;
    }
    void setPoint(Point2<double> point1)
    {
        this->point = &point1;
    }

private:
    Point2<double> *point;
    Instance* instance;
};

#endif // DISJOINTSET_H