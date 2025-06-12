#include "MBFFOptimizer.h"
#include <algorithm>
#include "visualizer.h"
#include <cairo/cairo.h>
#include <unordered_set>

// Assuming 2bifffLibrary is a vector of BiFFFLibrary objects

// 1. Equality operator
template <typename T>
bool operator==(const Point2<T> &a, const Point2<T> &b)
{
    return a.x == b.x && a.y == b.y;
}

// 2. Hash function specialization
namespace std
{
    template <>
    struct hash<Point2<int>>
    {
        size_t operator()(const Point2<int> &p) const
        {
            return hash<int>()(p.x) ^ (hash<int>()(p.y) << 1);
        }
    };
}

bool MBFFOptimizer::placeLegal(Instance *inst, Point2<int> desPos)
{
    vector<Point2<int>> candidates;
    vector<Point2<int>> prevCandidates;
    std::unordered_set<Point2<int>> visited;

    candidates.push_back(desPos);
    visited.insert(desPos);
    int count = 0;

    auto isLegal = [&](int row, int col, int h, int w) -> bool
    {
        if (row + h > static_cast<int>(_occupiedMask.size()) ||
            col + w > static_cast<int>(_occupiedMask[0].size()))
            return false;

        for (int r = row; r < row + h; ++r)
        {
            for (int c = col; c < col + w; ++c)
            {
                if (_occupiedMask[r][c])
                    return false;
            }
        }
        return true;
    };

    auto markOccupied = [&](int row, int col, int h, int w)
    {
        for (int r = row; r < row + h; ++r)
        {
            for (int c = col; c < col + w; ++c)
            {
                _occupiedMask[r][c] = true;
            }
        }
    };

    while (count < 1000)
    {
        // if (count < 8) {
        //     std::cout << "count: " << count << ", size: " << candidates.size() << ": ";
        //     for (const auto& cand : candidates) {
        //         std::cout << "(" << cand.x << ", " << cand.y << ") ";
        //     }
        //     std::cout << std::endl;
        // }

        prevCandidates = candidates;
        candidates.clear();

        for (auto &candidate : prevCandidates)
        {
            int row = candidate.x;
            int col = candidate.y;

            const auto &lib = inst->pCellLibrary();
            int bit = lib->numBits();

            std::vector<std::pair<Point2<int>, std::string>> *checker = nullptr;
            if (bit == 1)
                checker = &FF1Checker;
            else if (bit == 2)
                checker = &FF2Checker;
            else if (bit == 4)
                checker = &FF4Checker;
            else
                continue;

            for (const auto &[gridSize, cellName] : *checker)
            {
                int h = gridSize.y;
                int w = gridSize.x;

                if (isLegal(row, col, h, w))
                {
                    inst->setX(_pPlacementRows[0]->x() + col * _pPlacementRows[0]->width());
                    inst->setY(_pPlacementRows[row]->y());
                    markOccupied(row, col, h, w);
                    inst->setCellLibrary(_name2pFFLibrary[cellName]);
                    return true;
                }
            }
        }

        for (auto &pos : prevCandidates)
        {
            std::vector<Point2<int>> neighbors = {
                {pos.x + 1, pos.y}, {pos.x, pos.y + 1}, {pos.x - 1, pos.y}, {pos.x, pos.y - 1}};
            for (auto &next : neighbors)
            {
                if (next.x >= 0 && next.y >= 0 &&
                    next.x < static_cast<int>(_occupiedMask.size()) &&
                    next.y < static_cast<int>(_occupiedMask[0].size()) &&
                    visited.find(next) == visited.end())
                {
                    candidates.push_back(next);
                    visited.insert(next);
                }
            }
        }

        ++count;
    }

    std::cout << "Cannot find legal placement for instance: " << inst->name()
              << ", bits :" << inst->pCellLibrary()->numBits() << '\n';
    if (inst->pCellLibrary()->numBits() == 1)
    {
        std::cout << "FUCKING ASSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS" << std::endl;
    }
    return false;
}

void MBFFOptimizer::initFFChecker()
{
    FF1Checker.clear();
    FF2Checker.clear();
    FF4Checker.clear();

    auto computeGridSize = [&](CellLibrary *lib) -> Point2<int>
    {
        int gridW = static_cast<int>(std::ceil(lib->width() / _pPlacementRows[0]->width()));
        int gridH = static_cast<int>(std::ceil(lib->height() / _pPlacementRows[0]->height()));
        return Point2<int>(gridW, gridH);
    };

    for (const auto &[cellName, lib] : _name2pFFLibrary)
    {
        int bitCount = lib->numBits();
        Point2<int> gridSize = computeGridSize(lib);

        if (bitCount == 1)
        {
            FF1Checker.emplace_back(gridSize, cellName);
        }
        else if (bitCount == 2)
        {
            FF2Checker.emplace_back(gridSize, cellName);
        }
        else if (bitCount == 4)
        {
            FF4Checker.emplace_back(gridSize, cellName);
        }
    }

    auto areaCmp = [&](const std::pair<Point2<int>, std::string> &a,
                       const std::pair<Point2<int>, std::string> &b)
    {
        double areaA = _name2pFFLibrary[a.second]->area();
        double areaB = _name2pFFLibrary[b.second]->area();
        return areaA < areaB;
    };

    auto scoreCmp = [&](const std::pair<Point2<int>, std::string> &a,
                        const std::pair<Point2<int>, std::string> &b)
    {
        double areaA = _name2pFFLibrary[a.second]->area();
        double areaB = _name2pFFLibrary[b.second]->area();
        double powerA = _name2pFFLibrary[a.second]->gatePower();
        double powerB = _name2pFFLibrary[b.second]->gatePower();
        double tnsA = _name2pFFLibrary[a.second]->qPinDelay();
        double tnsB = _name2pFFLibrary[b.second]->qPinDelay();
        double scoreA = _gamma * areaA + _beta * powerA + _alpha * tnsA; // Adjust power weight as needed
        double scoreB = _gamma * areaB + _beta * powerB + _alpha * tnsB; // Adjust power weight as needed
        return scoreA < scoreB;
    };

    auto sorter = scoreCmp;
    std::sort(FF1Checker.begin(), FF1Checker.end(), sorter);
    std::sort(FF2Checker.begin(), FF2Checker.end(), sorter);
    std::sort(FF4Checker.begin(), FF4Checker.end(), sorter);

    // print all name and score in each checker
    std::cout << "[INFO] FF1 Checkers:\n";
    for (const auto &[gridSize, cellName] : FF1Checker)
    {
        double area = _name2pFFLibrary[cellName]->area();
        double power = _name2pFFLibrary[cellName]->gatePower();
        double tns = _name2pFFLibrary[cellName]->qPinDelay();
        double score = _gamma * area + _beta * power + _alpha * tns;
        std::cout << "  " << cellName << " - Area: " << area
                  << ", Power: " << power
                  << ", TNS: " << tns
                  << ", Score: " << score << "\n";
    }
    std::cout << "[INFO] FF2 Checkers:\n";
    for (const auto &[gridSize, cellName] : FF2Checker)
    {
        double area = _name2pFFLibrary[cellName]->area();
        double power = _name2pFFLibrary[cellName]->gatePower();
        double tns = _name2pFFLibrary[cellName]->qPinDelay();
        double score = _gamma * area + _beta * power + _alpha * tns;
        std::cout << "  " << cellName << " - Area: " << area
                  << ", Power: " << power
                  << ", TNS: " << tns
                  << ", Score: " << score << "\n";
    }
    std::cout << "[INFO] FF4 Checkers:\n";
    for (const auto &[gridSize, cellName] : FF4Checker)
    {
        double area = _name2pFFLibrary[cellName]->area();
        double power = _name2pFFLibrary[cellName]->gatePower();
        double tns = _name2pFFLibrary[cellName]->qPinDelay();
        double score = _gamma * area + _beta * power + _alpha * tns;
        std::cout << "  " << cellName << " - Area: " << area
                  << ", Power: " << power
                  << ", TNS: " << tns
                  << ", Score: " << score << "\n";
    }
}

void MBFFOptimizer::init_occupied()
{
    size_t numRows = _pPlacementRows.size();
    size_t numSitesPerRow = (numRows > 0) ? _pPlacementRows[0]->numSites() : 0;

    _occupiedMask.resize(numRows);
    for (size_t row = 0; row < numRows; ++row)
    {
        _occupiedMask[row].resize(numSitesPerRow, false); // all unoccupied
    }

    // Mark gate instance area as occupied
    for (const auto &[name, instance] : _name2pInstances_gate)
    {
        int row = static_cast<int>((instance->y() - _pPlacementRows[0]->y()) / _pPlacementRows[0]->height());
        int col = static_cast<int>((instance->x() - _pPlacementRows[0]->x()) / _pPlacementRows[0]->width());
        int h = static_cast<int>(std::ceil(instance->pCellLibrary()->height() / _pPlacementRows[0]->height()));
        int w = static_cast<int>(std::ceil(instance->pCellLibrary()->width() / _pPlacementRows[0]->width()));

        for (int r = row; r < row + h; ++r)
        {
            for (int c = col; c < col + w; ++c)
            {
                if (r >= 0 && r < static_cast<int>(numRows) && c >= 0 && c < static_cast<int>(numSitesPerRow))
                {
                    _occupiedMask[r][c] = true;
                }
            }
        }
    }

    // ==== Plotting the occupancy mask ====
    int scale = 2; // adjust to zoom in/out
    int width = numSitesPerRow * scale;
    int height = numRows * scale;

    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t *cr = cairo_create(surface);

    // White background
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    // Draw occupied cells
    cairo_set_source_rgb(cr, 0, 0, 0);
    for (size_t r = 0; r < numRows; ++r)
    {
        for (size_t c = 0; c < numSitesPerRow; ++c)
        {
            if (_occupiedMask[r][c])
            {
                cairo_rectangle(cr, c * scale, r * scale, scale, scale);
            }
        }
    }
    cairo_fill(cr);

    cairo_surface_write_to_png(surface, "debug_occupied_mask.png");
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    std::cout << "[INFO] Occupied mask written to debug_occupied_mask.png\n";
}

void MBFFOptimizer::PrintOutfile(fstream &outfile)
{
    std::unordered_map<std::string, int> ffUsageCount;

    // Traverse all original FFs and emit unmerged ones
    for (auto &[origName, inst] : _name2pInstances_ff)
    {
        if (!inst->merged)
        {
            // Give it a fresh new_inst_... name
            std::string newName = "new_inst_" + std::to_string(_instCnt++) + "_unmerged";
            inst->setName(newName);

            // Place legally
            placeLegal(inst, Point2<int>(
                                 std::floor((inst->y() - _pPlacementRows[0]->y()) / _pPlacementRows[0]->height()),
                                 std::floor((inst->x() - _pPlacementRows[0]->x()) / _pPlacementRows[0]->width())));
            _mergedInstances.push_back(inst);

            // Record pin mapping
            int bitWidth = inst->pCellLibrary()->numBits();
            for (int i = 0; i < bitWidth; ++i)
            {
                std::string dName = (bitWidth == 1) ? "D" : "D" + std::to_string(i);
                std::string qName = (bitWidth == 1) ? "Q" : "Q" + std::to_string(i);
                _pinMappings.emplace_back(origName + "/" + dName + " map " + newName + "/" + dName);
                _pinMappings.emplace_back(origName + "/" + qName + " map " + newName + "/" + qName);
            }
            _pinMappings.emplace_back(origName + "/CLK map " + newName + "/CLK");
        }
    }

    // Write all merged instances to file
    outfile << "CellInst " << _mergedInstances.size() << "\n";
    for (auto *inst : _mergedInstances)
    {
        outfile << "Inst " << inst->name() << " "
                << inst->pCellLibrary()->name() << " "
                << inst->x() << " " << inst->y() << "\n";

        // Count FF type
        ffUsageCount[inst->pCellLibrary()->name()]++;
    }

    // Write pin mappings
    for (const std::string &line : _pinMappings)
    {
        outfile << line << "\n";
    }

    // Print FF usage summary
    std::cout << "\n[FF Usage Summary]\n";
    for (const auto &[ffName, count] : ffUsageCount)
    {
        // name and width and height
        std::cout << "  " << ffName << " " << _name2pFFLibrary[ffName]->width() << "x"
                  << _name2pFFLibrary[ffName]->height() << " : " << count << "\n";
    }
    std::cout << std::endl;
}

vector<NeighInfo> MBFFOptimizer::kNearestSets(const std::vector<DisSet *> *Sets,
                                              unsigned elem1,
                                              unsigned k,
                                              bool manhattan)
{
    if (!Sets || elem1 >= Sets->size() || k == 0)
        return {};

    Instance *base = Sets->at(elem1)->getInstances();
    double x0 = base->x(), y0 = base->y();

    std::vector<NeighInfo> buf;
    buf.reserve(Sets->size() - 1);

    for (unsigned i = 0; i < Sets->size(); ++i)
    {
        if (i == elem1)
            continue; // skip itself
        Instance *inst = Sets->at(i)->getInstances();
        double dx = inst->x() - x0;
        double dy = inst->y() - y0;
        double d = manhattan ? std::abs(dx) + std::abs(dy)
                             : std::hypot(dx, dy);
        buf.push_back({i, d}); // remember *i*!
    }

    // Keep only the k smallest (average O(N))
    if (k < buf.size())
    {
        std::nth_element(buf.begin(),
                         buf.begin() + k,
                         buf.end());
        buf.resize(k);
    }

    std::sort(buf.begin(), buf.end()); // ascending by dist
    return buf;                        // each entry knows its original index
}

void MBFFOptimizer::Synthesize(vector<DisSet *> *Sets, vector<bool> *visited, fstream &outfile, int net_count)
{
    unsigned count = 0;
    unsigned elem1 = 0;
    unsigned elem2 = 0;
    int merge_num = 0;
    bool merge_success = false;

    while (count < visited->size())
    {
        bool flag = false;
        bool found = false;

        // find first unvisited element
        for (unsigned i = 0; i < Sets->size(); i++)
        {
            if (!(*visited)[i])
            {
                count++;
                elem1 = i;
                found = true;
                (*visited)[i] = true;
                break;
            }
        }

        if (!found)
            break;

        if (Sets->at(elem1)->getInstances()->totalTimingSlack() > 0) // if the selected FF has positive slack, find another one to merge
        {
            const unsigned K_NEIGHBOURS = 8; // or tune as you like
            auto neighList = kNearestSets(Sets, elem1, K_NEIGHBOURS, /*manhattan=*/true);
            // sort by timingslack
            std::sort(neighList.begin(), neighList.end(),
                      [Sets](const NeighInfo &a, const NeighInfo &b)
                      {
                          return Sets->at(a.setIdx)->getInstances()->name2pPins()["D"]->timingSlack() >
                                 Sets->at(b.setIdx)->getInstances()->name2pPins()["D"]->timingSlack();
                      });
            // for (int i = 0; i < neighList.size(); i++)
            // {
            //     cout << "neighList[" << i << "] = " << neighList[i].setIdx
            //          << ", timingSlack = " << Sets->at(neighList[i].setIdx)->getInstances()->name2pPins()["D"]->timingSlack() << endl;
            // }
            for (unsigned k = 0; k < neighList.size(); k++)
            {
                int j = neighList[k].setIdx;
                if ((j < Sets->size()) &&
                    (Sets->at(j)->getInstances()->totalTimingSlack() > 0) &&
                    (!(*visited)[j]) &&
                    ((abs(Sets->at(elem1)->getPoint().x - Sets->at(j)->getPoint().x) +
                      abs(Sets->at(elem1)->getPoint().y - Sets->at(j)->getPoint().y)) <
                     ((Sets->at(elem1)->getInstances()->totalTimingSlack() +
                       Sets->at(j)->getInstances()->totalTimingSlack()) /
                      _displacementDelay)) &&
                    (Sets->at(elem1)->getInstances()->pCellLibrary()->numBits() == Sets->at(j)->getInstances()->pCellLibrary()->numBits()))
                {
                    count++;
                    flag = true;
                    elem2 = j;
                    (*visited)[j] = true;
                    break;
                }
            }
        }

        if (!flag) // no suitable
        {
            // One FF only (1-bit)
            string name = "new_inst_" + to_string(_instCnt++) + "_" + to_string(net_count) + "_" + to_string(merge_num);
            Instance *inst = new Instance(name, Sets->at(elem1)->getInstances()->pCellLibrary(),
                                          Sets->at(elem1)->getInstances()->x(),
                                          Sets->at(elem1)->getInstances()->y(), Sets->at(elem1)->getInstances()->numPins());
            merge_success = placeLegal(inst, Point2<int>(floor((inst->y() - _pPlacementRows[0]->y()) / _pPlacementRows[0]->height()),
                                                         floor((inst->x() - _pPlacementRows[0]->x()) / _pPlacementRows[0]->width())));

            if (merge_success)
            {
                _mergedInstances.push_back(inst);
                Sets->at(elem1)->getInstances()->merged = true; // Mark the instance as merged

                std::string origName = Sets->at(elem1)->getInstances()->name();
                std::string newName = inst->name();
                int bitWidth = inst->pCellLibrary()->numBits();

                // Map D and Q pins
                for (int i = 0; i < bitWidth; ++i)
                {
                    std::string dName = (bitWidth == 1) ? "D" : "D" + std::to_string(i);
                    std::string qName = (bitWidth == 1) ? "Q" : "Q" + std::to_string(i);

                    _pinMappings.emplace_back(origName + "/" + dName + " map " + newName + "/" + dName);
                    _pinMappings.emplace_back(origName + "/" + qName + " map " + newName + "/" + qName);
                }

                // Always map CLK
                _pinMappings.emplace_back(origName + "/CLK map " + newName + "/CLK");

                cell_instance++;
            }
        }
        else
        {
            // Try to merge FF1 and FF2 (2-bit)
            Instance *instance;
            double x = (Sets->at(elem1)->getInstances()->x() * Sets->at(elem2)->getInstances()->totalTimingSlack() +
                        Sets->at(elem2)->getInstances()->x() * Sets->at(elem1)->getInstances()->totalTimingSlack()) /
                       (Sets->at(elem1)->getInstances()->totalTimingSlack() + Sets->at(elem2)->getInstances()->totalTimingSlack());

            double y = (Sets->at(elem1)->getInstances()->y() * Sets->at(elem2)->getInstances()->totalTimingSlack() +
                        Sets->at(elem2)->getInstances()->y() * Sets->at(elem1)->getInstances()->totalTimingSlack()) /
                       (Sets->at(elem1)->getInstances()->totalTimingSlack() + Sets->at(elem2)->getInstances()->totalTimingSlack());

            int row = floor((y - _pPlacementRows[0]->y()) / (_pPlacementRows[0]->height()));
            int index = floor((x - _pPlacementRows[0]->x()) / (_pPlacementRows[0]->width()));

            // create 2 instance that merged 2 ff, but don't use the merge2FF function
            // if 1 bit ff merge, get 2 bit ff, set the merged instance to best 2 bit ff
            CellLibrary *bestFFLib = nullptr;
            if (Sets->at(elem1)->getInstances()->pCellLibrary()->numBits() == 1 &&
                Sets->at(elem2)->getInstances()->pCellLibrary()->numBits() == 1)
            {
                bestFFLib = _name2pFFLibrary[FF2Checker[0].second];
            }
            else if (Sets->at(elem1)->getInstances()->pCellLibrary()->numBits() == 2 &&
                     Sets->at(elem2)->getInstances()->pCellLibrary()->numBits() == 2)
            {
                bestFFLib = _name2pFFLibrary[FF4Checker[0].second];
            }
            else
            {
                std::cout << "[ERROR] Cannot merge FFs with different bit widths/ two 4 bits: "
                          << Sets->at(elem1)->getInstances()->pCellLibrary()->numBits() << " and "
                          << Sets->at(elem2)->getInstances()->pCellLibrary()->numBits() << "\n";
            }

            instance = new Instance(
                "new_inst_" + std::to_string(_instCnt++) + "_" + std::to_string(net_count) + "_" + std::to_string(merge_num),
                bestFFLib,
                x, y, Sets->at(elem1)->getInstances()->numPins() + Sets->at(elem2)->getInstances()->numPins() - 1);

            if (instance)
            {
                merge_success = placeLegal(instance, Point2<int>(row, index));

                if (merge_success)
                {
                    _mergedInstances.push_back(instance);

                    auto *inst1 = Sets->at(elem1)->getInstances();
                    auto *inst2 = Sets->at(elem2)->getInstances();
                    inst1->merged = true;
                    inst2->merged = true;

                    // Generate pin mappings for all data bits
                    int bitCount = instance->pCellLibrary()->numBits();
                    for (int i = 0; i < bitCount; ++i)
                    {
                        std::string dPin = "D" + std::to_string(i);
                        std::string qPin = "Q" + std::to_string(i);
                        std::string origD, origQ, origCLK;

                        if (i == 0)
                        {
                            origD = inst1->name() + "/D";
                            origQ = inst1->name() + "/Q";
                            origCLK = inst1->name() + "/CLK";
                        }
                        else if (i == 1)
                        {
                            origD = inst2->name() + "/D";
                            origQ = inst2->name() + "/Q";
                            origCLK = inst2->name() + "/CLK";
                        }
                        else
                        {
                            // Optional: handle >2 FFs merged if merge2FF is extended
                            continue;
                        }

                        _pinMappings.emplace_back(origD + " map " + instance->name() + "/" + dPin);
                        _pinMappings.emplace_back(origQ + " map " + instance->name() + "/" + qPin);
                        _pinMappings.emplace_back(origCLK + " map " + instance->name() + "/CLK");
                    }

                    cell_instance++;
                }
            }
            else
            {
                // ── Fallback to 2 × 1-bit FFs ──────────────────────────────────────────────
                for (int i = 0; i < 2; ++i)
                {
                    int elem = (i == 0) ? elem1 : elem2;
                    int mergeId = merge_num + i;
                    auto *orig = Sets->at(elem)->getInstances();
                    int x = orig->x();
                    int y = orig->y();

                    std::string name = "new_inst_" + std::to_string(_instCnt++) + "_" +
                                       std::to_string(net_count) + "_" + std::to_string(mergeId);

                    Instance *inst = new Instance(
                        name,
                        orig->pCellLibrary(),
                        x,
                        y,
                        orig->numPins());

                    bool placed = placeLegal(inst, Point2<int>(
                                                       std::floor((y - _pPlacementRows[0]->y()) / _pPlacementRows[0]->height()),
                                                       std::floor((x - _pPlacementRows[0]->x()) / _pPlacementRows[0]->width())));

                    if (placed)
                    {
                        _mergedInstances.push_back(inst);
                        orig->merged = true;

                        std::string origName = orig->name();
                        std::string newName = inst->name();
                        int bitWidth = inst->pCellLibrary()->numBits();

                        for (int b = 0; b < bitWidth; ++b)
                        {
                            std::string d = (bitWidth == 1) ? "D" : "D" + std::to_string(b);
                            std::string q = (bitWidth == 1) ? "Q" : "Q" + std::to_string(b);
                            _pinMappings.emplace_back(origName + "/" + d + " map " + newName + "/" + d);
                            _pinMappings.emplace_back(origName + "/" + q + " map " + newName + "/" + q);
                        }
                        _pinMappings.emplace_back(origName + "/CLK map " + newName + "/CLK");

                        cell_instance++;
                    }
                }
            }
        }

        merge_num++;
    }
}

void MBFFOptimizer::findFeasable_reg(Net *net, fstream &outfile, int net_count)
{
    vector<DisSet *> *Sets = new vector<DisSet *>;
    vector<bool> *visited = new vector<bool>(net->numPins() - 1, false);

    // First, collect all FFs in this net
    for (unsigned i = 1; i < net->numPins(); i++)
    {
        if (net->pPins()[i].pin->getinstance()->pCellLibrary()->numBits() != 4)
        { // dont merge 4 bits FFs
            DisSet *set = new DisSet(net->pPins()[i].pin->getinstance(), net->pPins()[i].pin->x(), net->pPins()[i].pin->y());
            Sets->push_back(set);
        }
    }

    // Process FFs that can be merged
    Synthesize(Sets, visited, outfile, net_count);

    // Clean up
    for (auto set : *Sets)
    {
        delete set;
    }
    delete Sets;
    delete visited;
}

void MBFFOptimizer::algorithm(std::string baseName)
{
    fstream outfile; // open file in write mode
    outfile.open(_outfile, ios::out);
    int net_count = 0;
    bool all_clk = true; // we have to check all output pins are clk case

    // plot every 10% of net
    int count = 0;
    for (auto net : _pNets)
    {
        all_clk = true;
        if (net->numPins() <= 1)
            continue;

        for (unsigned int i = 1; i < net->numPins(); i++)
        {
            if (net->pin(i).pin->name() != "CLK")
            {
                all_clk = false;
                break;
            }
        }

        if (all_clk)
            count++;
    }

    int ploting_interval = count / 10;
    std::cout << "Total nets to process: " << count << ", interval: " << ploting_interval << "\n";
    count = 0; // reset count for plotting progress

    for (auto net : _pNets) // at each net, we have to redo weight_matrix and at the end of
    {
        // cout << "123" << endl;
        all_clk = true; // each net, we have to reset all placementRow

        for (unsigned int i = 1; i < net->numPins(); i++)
        {
            if (net->pin(i).pin->name() != "CLK")
            {
                // cout << "123";
                all_clk = false;
                break;
            }
        }
        if (!all_clk)
        {
            // cout << "pass " << net->name() << endl;
            continue;
        }
        else if (all_clk && net->numPins() > 1) // run merging algorithm
        {
            // cout << "123\n";
            net_count++;
            findFeasable_reg(net, outfile, net_count);
        }

        count++;
        if (count % 100 == 0)
        {
            std::cout << "Processing net: " << count << "\n";
        }

        // if (count % ploting_interval == 0)
        // {
        //     // std::cout << "Plotting progress: " << count/ploting_interval*10 << "%\n";
        //     plotMerge(dieWidth(), dieHeight(), _name2pInstances_ff, _name2pInstances_gate, _mergedInstances, baseName, count /ploting_interval);
        // }
    }

    std::cout << "Total nets processed: " << count << "\n";

    PrintOutfile(outfile);
    // reset all placementRow
}

void MBFFOptimizer::printInput()
{
    cout << "alpha:" << _alpha << " " << "beta:" << _beta << " " << "gamma:" << _gamma << " " << "lambda:" << _lambda << endl;
    cout << "LEFTBOTTOM coordinates: " << _dieLeftBottom.x << " " << _dieLeftBottom.y << endl;
    cout << "RIGHTTOM coordinates: " << _dieRightTop.x << " " << _dieRightTop.y << endl;
    cout << "_numInputPINS: " << _numInputPins << endl;
    cout << "--------------INPUTPINS----------------" << endl;

    // cout << _name2pInputPins["in"]->name() << _name2pInputPins["in"]->x() << endl;
    // cout << _name2pInputPins["CLK"]->name() << endl;
    for (auto [key, value] : _name2pInputPins)
    {
        cout << key << " has name " << value->name() << " x-coordinates: " << value->x() << " y-coordinates: " << value->y() << endl;
    }

    cout << "_numOutputPins: " << _numOutputPins << endl;
    for (auto [key, value] : _name2pOutputPins)
    {
        cout << key << " has name " << value->name() << " x-coordinates: " << value->x() << " y-coordinates: " << value->y() << endl;
    }
    cout << "--------------FFLIBRARY----------------" << endl;
    for (auto &[key, value] : _name2pFFLibrary)
    {
        cout << key << " has name " << value->name() << " width: " << value->width() << " height: " << value->height() << " numBits: " << value->numBits() << " numPins: " << value->numPins() << endl;
        for (unsigned int i = 0; i < value->numPins(); i++)
        {
            cout << "Pin: " << i << endl;
            cout << " " << value->pPin(i)->name() << " x-coordinates: " << value->pPin(i)->x() << " y-coordinates: " << value->pPin(i)->y() << endl;
        }
    }
    cout << "--------------GATELIBRARY----------------" << endl;
    for (const auto &[key, value] : _name2pGateLibrary)
    {
        cout << key << " has name " << value->name() << " width: " << value->width() << " height: " << value->height() << " numBits: " << value->numBits() << " numPins: " << value->numPins() << endl;
        for (unsigned int i = 0; i < value->numPins(); i++)
        {
            cout << "Pin: " << i << endl;
            cout << " " << value->pPin(i)->name() << " x-coordinates: " << value->pPin(i)->x() << " y-coordinates: " << value->pPin(i)->y() << endl;
        }
    }
    cout << "--------------INSTANCES----------------" << endl;
    cout << "numInstances" << _numInstances << endl;
    for (const auto &[key, value] : _name2pInstances_ff)
    {
        cout << key << " has name " << value->name() << " x: " << value->x() << " y: " << value->y() << " Library " << value->pCellLibrary()->name() << endl;
    }
    for (const auto &[key, value] : _name2pInstances_gate)
    {
        cout << key << " has name " << value->name() << " x: " << value->x() << " y: " << value->y() << " Library " << value->pCellLibrary()->name() << endl;
    }
    cout << "-------------------Net-------------------" << endl;
    cout << "numNets" << _numNets << endl;
    for (auto &net : _pNets)
    {
        // cout << net->name() << endl;
        cout << net->pin(0).pin->name() << endl;
        for (unsigned int i = 0; i < net->numPins(); i++)
        {
            cout << net->pin(i).inst_name << " " << net->pin(i).pin->getinstance() << " ";
        }
        cout << endl;
    }
    cout << "------------------------Bin Parameters-------------------" << endl;
    cout << "BinWidth: " << _binWidth << endl;
    cout << "BinHeight: " << _binHeight << endl;
    cout << "BinMaxUtil: " << _binMaxUtil << endl;
    cout << "--------------------DISPLACEMENTDELAY-------------------" << endl;
    cout << "DisplacementDelay: " << _displacementDelay << endl;
    cout << "--------------------PlacementRows-------------------" << endl;
    for (const auto &placementRow : _pPlacementRows)
    {
        cout << "PlacementRow: x_coor: " << placementRow->x() << " y_coor " << placementRow->y() << " width " << placementRow->width() << " height " << placementRow->height() << " numSites " << placementRow->numSites() << endl;
    }
    cout << "--------------------TimingSlack-------------------" << endl;
    for (const auto &[key, value] : _name2pInstances_ff)
    {
        cout << key << endl;
        for (const auto &[key1, value1] : value->TimingSlack())
        {
            cout << key1 << " has slack " << value1 << "  ";
        }
        cout << endl;
    }
    for (const auto net : _pNets)
    {
        cout << net->name() << endl;
        for (unsigned int i = 0; i < net->numPins(); i++)
        {
            cout << net->pin(i).pin->name() << " x_coor: " << net->pin(i).pin->x() << " y_coor: " << net->pin(i).pin->y() << " Time Slack: " << net->pin(i).pin->timingSlack() << " " << endl;
        }
        cout << endl;
    }
    cout << "--------------------GatePower-------------------" << endl;
    for (const auto &[key, value] : _name2pFFLibrary)
    {

        cout << key << " has power " << value->gatePower() << endl;
    }

    for (const auto &[key, value] : _name2pGateLibrary)
    {

        cout << key << " has power " << value->gatePower() << endl;
    }
};

double MBFFOptimizer::HPWL(vector<Instance *> *pInstances)
{
    double HPWL = 0.0;
    double x_min = numeric_limits<double>::max();
    double x_max = numeric_limits<double>::min();
    double y_min = numeric_limits<double>::max();
    double y_max = numeric_limits<double>::min();

    for (const auto &instance : *pInstances)
    {
        if (instance->x() < x_min)
            x_min = instance->x();
        if (instance->x() > x_max)
            x_max = instance->x();
        if (instance->y() < y_min)
            y_min = instance->y();
        if (instance->y() > y_max)
            y_max = instance->y();
    }
    HPWL = (x_max - x_min) + (y_max - y_min);
    return HPWL;
}

double MBFFOptimizer::score(string InstanceName, int k)
{
    double score = 0.0;
    double x = _name2pInstances_ff[InstanceName]->x();
    double y = _name2pInstances_ff[InstanceName]->y();
    int num_buckets = 400;
    int bucket_index = floor(x / num_buckets);
    vector<Instance *> L;
    vector<Instance *> target2k, targetk;
    target2k = findknear(InstanceName, k * 2);
    for (int i = 0; i < k; i++)
    {
        // cout << "i: " << i << endl;
        L.push_back(target2k[i]);
    }
    // cout << "target2k size: " << target2k.size() << endl;
    // for(int i = 0; i < target2k.size(); i++){
    //     cout << target2k[i]->name() << endl;
    // }
    // cout << "targetk size: " << L.size() << endl;
    // for(int i = 0; i < L.size(); i++){
    //     cout << L[i]->name() << endl;
    // }
    double L2k = HPWL(&target2k);
    double Lk = HPWL(&L);
    // cout << "L2k: " << L2k << " Lk: " << Lk << endl;
    score += L2k - Lk;
    // cout << "c: " << c1 / c2 * (1 / (1 + exp(_name2pInstances_ff[InstanceName]->TimingSlack("D")))) <<endl;
    score += c1 / c2 * (1 / (1 + exp(_name2pInstances_ff[InstanceName]->TimingSlack("D"))));
    return score;
}

vector<Instance *> MBFFOptimizer::findknear(string InstanceName, int k)
{
    vector<Instance *> L;
    vector<Instance *> target;
    double x = _name2pInstances_ff[InstanceName]->x();
    double y = _name2pInstances_ff[InstanceName]->y();
    L.push_back(_name2pInstances_ff[InstanceName]);
    int left = 0;
    int right = _bucket.bucket_query(x, 0).size() - 1;
    int index;
    // cout << "left0: " << left << " right0: " << right << endl;
    while (left <= right)
    {
        int mid = (left + right) / 2;
        if (_bucket.bucket_query(x, 0)[mid]->y() < y)
        {
            left = mid + 1;
        }
        else if (_bucket.bucket_query(x, 0)[mid]->y() > y)
        {
            right = mid - 1;
        }
        else
        {
            index = mid;
            break;
        }
        index = mid;
    }
    if (index + 1 < _bucket.bucket_query(x, 0).size())
    {
        L.push_back(_bucket.bucket_query(x, 0)[index + 1]);
    }
    if (index - 1 >= 0)
    {
        L.push_back(_bucket.bucket_query(x, 0)[index - 1]);
    }

    int find_right = 1;
    left = 0;
    right = _bucket.bucket_query(x, find_right).size() - 1;
    while (right == -1 && find_right < 5)
    {
        find_right++;
        right = _bucket.bucket_query(x, find_right).size() - 1;
    }
    // cout << "left1: " << left << " right1: " << right << endl;
    if (right != -1)
    {
        while (left <= right)
        {
            int mid = (left + right) / 2;
            // cout << "mid: " << mid << endl;
            // cout << _bucket.bucket_query(x, find_right).size() <<endl;
            if (_bucket.bucket_query(x, find_right)[mid]->y() < y)
            {
                left = mid + 1;
            }
            else if (_bucket.bucket_query(x, find_right)[mid]->y() > y)
            {
                right = mid - 1;
            }
            else
            {
                index = mid;
                break;
            }
            index = mid;
        }
        // cout <<"hey"<<endl;
        L.push_back(_bucket.bucket_query(x, find_right)[index]);
        if (index + 1 < _bucket.bucket_query(x, find_right).size())
        {
            L.push_back(_bucket.bucket_query(x, find_right)[index + 1]);
        }
        if (index - 1 >= 0)
        {
            L.push_back(_bucket.bucket_query(x, find_right)[index - 1]);
        }
    }

    int find_left = -1;
    left = 0;
    right = _bucket.bucket_query(x, -1).size() - 1;
    while (right == -1 && find_left > -5)
    {
        find_left--;
        right = _bucket.bucket_query(x, find_left).size() - 1;
    }
    // cout << "left-1: " << left << " right-1: " << right << endl;
    if (right != -1)
    {
        while (left <= right)
        {
            int mid = (left + right) / 2;
            if (_bucket.bucket_query(x, find_left)[mid]->y() < y)
            {
                left = mid + 1;
            }
            else if (_bucket.bucket_query(x, find_left)[mid]->y() > y)
            {
                right = mid - 1;
            }
            else
            {
                index = mid;
                break;
            }
            index = mid;
        }
        L.push_back(_bucket.bucket_query(x, find_left)[index]);
        if (index + 1 < _bucket.bucket_query(x, find_left).size())
        {
            L.push_back(_bucket.bucket_query(x, find_left)[index + 1]);
        }
        if (index - 1 >= 0)
        {
            L.push_back(_bucket.bucket_query(x, find_left)[index - 1]);
        }
    }

    if (L.size() < k)
    {
        cout << "not enough elements in bucket\n";
        return L;
    }

    // Sort the vector based on the distance to the target point
    sort(L.begin(), L.end(), [x, y](Instance *a, Instance *b)
         {
        double dist_a = (a->x() - x) * (a->x() - x) + (a->y() - y) * (a->y() - y);
        double dist_b = (b->x() - x) * (b->x() - x) + (b->y() - y) * (b->y() - y);
        return dist_a < dist_b; });

    // cout <<"k: "<<k<<endl;
    for (int i = 0; i < L.size(); i++)
    {
        if (i >= k)
        {
            break;
        }
        target.push_back(L[i]);
    }

    return target;
}
