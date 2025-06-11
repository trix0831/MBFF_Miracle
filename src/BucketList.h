// bucket_list.hpp
#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>
#include <stdexcept>
#include "Instance.h"

struct Entry
{
    double x, y;
    bool fix;
    std::string name;
};

class BucketList
{
public:
    BucketList(std::size_t bucket_cnt, double min_x, double max_x)
        : bucket_cnt(bucket_cnt),
          minX(min_x),
          maxX(max_x),
          w(static_cast<double>(std::ceil((max_x - min_x) / bucket_cnt))),
          buckets(bucket_cnt) {}

    void insert(Instance *instance)
    {
        double x = instance->x();
        double y = instance->y();
        if (x < minX || x > maxX)
            throw std::out_of_range("x out of allowed range");
        auto &vec = buckets[index(x)];
        auto it = std::lower_bound(vec.begin(), vec.end(), y,
                                   [](Instance *a, double yy)
                                   { return a->y() < yy; });
        vec.insert(it, instance);
    }

    std::vector<Instance *> bucket_query(double x) const
    {

        if (x < minX || x > maxX)
        {
            throw std::out_of_range("x out of allowed range");
            return {};
        }
        std::vector<Instance *> out;
        for (Instance *instance : buckets[index(x)])
        {
            if (!instance->fixed())
                out.push_back(instance);
        }

        return out;
    }

private:
    std::size_t bucket_cnt;
    double maxX, minX, w;
    std::vector<std::vector<Instance *>> buckets;

    std::size_t index(double x) const { return static_cast<std::size_t>((x - minX) / w); }
};
