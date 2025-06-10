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
        instance = vector<Instance *>();
        instance.push_back(&(*ins));
        _size = 1;
        point = new Point2<double>(x, y);
    }
    ~DisSet() {}
    vector<Instance *> getInstances()
    {
        return instance;
    }
    void addInstance(Instance *ins)
    {
        instance.push_back(ins);
        _size++;
    }
    Point2<double> getPoint()
    {
        return *point;
    }
    void setPoint(Point2<double> point1)
    {
        this->point = &point1;
    }
    unsigned size()
    {
        return _size;
    }
    void setSize(unsigned size)
    {
        _size = size;
    }

private:
    Point2<double> *point;
    unsigned _size;
    vector<Instance *> instance;
};

#endif // DISJOINTSET_H