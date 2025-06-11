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
    BucketList()
        : bucket_cnt(0),
          minX(0),
          maxX(0),
          w(0) {}

    void setBucket(std::size_t bucket_cnt, double min_x, double max_x)
    {
        bucket_cnt = bucket_cnt;
        minX = min_x;
        maxX = max_x;
        w = static_cast<double>(std::ceil((max_x - min_x) / bucket_cnt));
        buckets.assign(bucket_cnt, {}); // 清乾淨後重建
    }
    void insert(Instance *instance)
    {
        double x = instance->x();
        double y = instance->y();
        if (x < minX || x > maxX)
        {
            cout << "instance: " << instance->name() << " x: " << x << " y: " << y << std::endl;
            throw std::out_of_range("x out of allowed range");
        }

        auto &vec = buckets[index(x)];
        auto it = std::lower_bound(vec.begin(), vec.end(), y,
                                   [](Instance *a, double yy)
                                   { return a->y() < yy; });
        vec.insert(it, instance);
        if (x < 2000)
            cout << "insert " << instance->name() << " at bucket index " << index(x) << " with y = " << y << std::endl;
    }

    std::vector<Instance *> bucket_query(double x, int i) const
    {

        if (x < minX || x > maxX)
        {
            throw std::out_of_range("x out of allowed range");
            return {};
        }
        std::vector<Instance *> out;
        if (index(x) + i < bucket_cnt && index(x) + i >= 0)
        {
            for (Instance *instance : buckets[index(x) + i])
            {
                if (!instance->fixed())
                    out.push_back(instance);
            }
        }
        return out;
    }

private:
    std::size_t bucket_cnt;
    double maxX, minX, w;
    std::vector<std::vector<Instance *>> buckets;

    std::size_t index(double x) const { return static_cast<std::size_t>((x - minX) / w); }
};
