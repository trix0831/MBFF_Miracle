#include "MBFFOptimizer.h"
#include <algorithm>
#include "visualizer.h"
#include <cairo/cairo.h>
#include <unordered_set>

// Assuming 2bifffLibrary is a vector of BiFFFLibrary objects

// 1. Equality operator
template<typename T>
bool operator==(const Point2<T>& a, const Point2<T>& b) {
    return a.x == b.x && a.y == b.y;
}

// 2. Hash function specialization
namespace std {
    template<>
    struct hash<Point2<int>> {
        size_t operator()(const Point2<int>& p) const {
            return hash<int>()(p.x) ^ (hash<int>()(p.y) << 1);
        }
    };
}

bool MBFFOptimizer::placeLegal(Instance* inst, Point2<int> desPos) {
    vector<Point2<int>> candidates;
    vector<Point2<int>> prevCandidates;
    std::unordered_set<Point2<int>> visited;

    //change the FF type of the instance to the best of the checkers
    if (inst->pCellLibrary()->numBits() == 1) {
        inst->setCellLibrary(_name2pFFLibrary[FF1Checker[0].second]);
    } else if (inst->pCellLibrary()->numBits() == 2) {
        inst->setCellLibrary(_name2pFFLibrary[FF2Checker[0].second]);
    } else if (inst->pCellLibrary()->numBits() == 4) {
        inst->setCellLibrary(_name2pFFLibrary[FF4Checker[0].second]);
    }

    candidates.push_back(desPos);
    visited.insert(desPos);
    int count = 0;

    auto isLegal = [&](int row, int col, int h, int w) -> bool {
        if (row + h > static_cast<int>(_occupiedMask.size()) ||
            col + w > static_cast<int>(_occupiedMask[0].size()))
            return false;

        for (int r = row; r < row + h; ++r) {
            for (int c = col; c < col + w; ++c) {
                if (_occupiedMask[r][c]) return false;
            }
        }
        return true;
    };

    auto markOccupied = [&](int row, int col, int h, int w) {
        for (int r = row; r < row + h; ++r) {
            for (int c = col; c < col + w; ++c) {
                _occupiedMask[r][c] = true;
            }
        }
    };

    while (count < 1000) {
        // if (count < 8) {
        //     std::cout << "count: " << count << ", size: " << candidates.size() << ": ";
        //     for (const auto& cand : candidates) {
        //         std::cout << "(" << cand.x << ", " << cand.y << ") ";
        //     }
        //     std::cout << std::endl;
        // }

        prevCandidates = candidates;
        candidates.clear();

        for (auto& candidate : prevCandidates) {
            int row = candidate.x;
            int col = candidate.y;

            const auto& lib = inst->pCellLibrary();
            int bit = lib->numBits();

            std::vector<std::pair<Point2<int>, std::string>>* checker = nullptr;
            if (bit == 1) checker = &FF1Checker;
            else if (bit == 2) checker = &FF2Checker;
            else if (bit == 4) checker = &FF4Checker;
            else continue;

            for (const auto& [gridSize, cellName] : *checker) {
                int h = gridSize.y;
                int w = gridSize.x;

                if (isLegal(row, col, h, w)) {
                    inst->setX(_pPlacementRows[0]->x() + col * _pPlacementRows[0]->width());
                    inst->setY(_pPlacementRows[row]->y());
                    markOccupied(row, col, h, w);
                    inst->setCellLibrary(_name2pFFLibrary[cellName]);
                    return true;
                }
            }
        }

        for (auto& pos : prevCandidates) {
            std::vector<Point2<int>> neighbors = {
                {pos.x + 1, pos.y}, {pos.x, pos.y + 1},
                {pos.x - 1, pos.y}, {pos.x, pos.y - 1}
            };
            for (auto& next : neighbors) {
                if (next.x >= 0 && next.y >= 0 &&
                    next.x < static_cast<int>(_occupiedMask.size()) &&
                    next.y < static_cast<int>(_occupiedMask[0].size()) &&
                    visited.find(next) == visited.end()) {
                    candidates.push_back(next);
                    visited.insert(next);
                }
            }
        }

        ++count;
    }

    std::cout << "Cannot find legal placement for instance: " << inst->name()
              << ", bits :" << inst->pCellLibrary()->numBits() << '\n';
    if (inst->pCellLibrary()->numBits() == 1) {
        std::cout << "FUCKING ASSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS" << std::endl;
    }
    return false;
}


void MBFFOptimizer::initFFChecker() {
    FF1Checker.clear();
    FF2Checker.clear();
    FF4Checker.clear();

    auto computeGridSize = [&](CellLibrary* lib) -> Point2<int> {
        int gridW = static_cast<int>(std::ceil(lib->width() / _pPlacementRows[0]->width()));
        int gridH = static_cast<int>(std::ceil(lib->height() / _pPlacementRows[0]->height()));
        return Point2<int>(gridW, gridH);
    };

    for (const auto& [cellName, lib] : _name2pFFLibrary) {
        int bitCount = lib->numBits();
        Point2<int> gridSize = computeGridSize(lib);

        if (bitCount == 1) {
            FF1Checker.emplace_back(gridSize, cellName);
        } else if (bitCount == 2) {
            FF2Checker.emplace_back(gridSize, cellName);
        } else if (bitCount == 4) {
            FF4Checker.emplace_back(gridSize, cellName);
        }
    }

    auto areaCmp = [&](const std::pair<Point2<int>, std::string>& a,
                       const std::pair<Point2<int>, std::string>& b) {
        double areaA = _name2pFFLibrary[a.second]->area();
        double areaB = _name2pFFLibrary[b.second]->area();
        return areaA < areaB;
    };

    double maxVal = std::max({_alpha, _beta, _gamma});
    double expAlpha = std::exp(_alpha - maxVal);
    double expBeta  = std::exp(_beta  - maxVal);
    double expGamma = std::exp(_gamma - maxVal);
    double sum = expAlpha + expBeta + expGamma;

    double normAlpha = expAlpha / sum;
    double normBeta  = expBeta  / sum;
    double normGamma = expGamma / sum;

    auto scoreCmp = [&](const std::pair<Point2<int>, std::string>& a,
                       const std::pair<Point2<int>, std::string>& b) {
        double areaA = _name2pFFLibrary[a.second]->area();
        double areaB = _name2pFFLibrary[b.second]->area();
        double powerA = _name2pFFLibrary[a.second]->gatePower();
        double powerB = _name2pFFLibrary[b.second]->gatePower();
        double scoreA = _gamma*areaA + _beta*powerA; // Adjust power weight as needed
        double scoreB = _gamma*areaB + _beta*powerB; // Adjust power weight as needed
        return scoreA < scoreB;
    };

    std::cout <<"checker 1 size" << FF1Checker.size() << std::endl;

    auto sorter = scoreCmp;
    auto keepTopHalf = [](auto& checker, auto& cmp) {
        std::sort(checker.begin(), checker.end(), cmp);
        // size_t keepSize = (checker.size() / 3) >10 ? (checker.size() / 3) : 10; // Keep at least 5 or 20% of the elements
        size_t keepSize = checker.size();
        checker.resize(keepSize);
    };

    keepTopHalf(FF1Checker, sorter);
    keepTopHalf(FF2Checker, sorter);
    keepTopHalf(FF4Checker, sorter);
    std::cout <<"checker 1 size" << FF1Checker.size() << std::endl;
}





void MBFFOptimizer::init_occupied() {
    size_t numRows = _pPlacementRows.size();
    size_t numSitesPerRow = (numRows > 0) ? _pPlacementRows[0]->numSites() : 0;

    _occupiedMask.resize(numRows);
    for (size_t row = 0; row < numRows; ++row) {
        _occupiedMask[row].resize(numSitesPerRow, false); // all unoccupied
    }

    // Mark gate instance area as occupied
    for (const auto& [name, instance] : _name2pInstances_gate) {
        int row = static_cast<int>((instance->y()-_pPlacementRows[0]->y()) / _pPlacementRows[0]->height());
        int col = static_cast<int>((instance->x()-_pPlacementRows[0]->x()) / _pPlacementRows[0]->width());
        int h = static_cast<int>(std::ceil(instance->pCellLibrary()->height() / _pPlacementRows[0]->height()));
        int w = static_cast<int>(std::ceil(instance->pCellLibrary()->width() / _pPlacementRows[0]->width()));

        for (int r = row; r < row + h; ++r) {
            for (int c = col; c < col + w; ++c) {
                if (r >= 0 && r < static_cast<int>(numRows) && c >= 0 && c < static_cast<int>(numSitesPerRow)) {
                    _occupiedMask[r][c] = true;
                }
            }
        }
    }

    // ==== Plotting the occupancy mask ====
    int scale = 2;  // adjust to zoom in/out
    int width = numSitesPerRow * scale;
    int height = numRows * scale;

    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t *cr = cairo_create(surface);

    // White background
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    // Draw occupied cells
    cairo_set_source_rgb(cr, 0, 0, 0);
    for (size_t r = 0; r < numRows; ++r) {
        for (size_t c = 0; c < numSitesPerRow; ++c) {
            if (_occupiedMask[r][c]) {
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


void Union(DisSet *b, DisSet *c)
{
    if (b->size() >= b->size())
    {
        // caculate new point
        Point2<double> point = Point2<double>((b->getPoint().x * b->size() + c->getPoint().x * c->size()) / (b->size() + c->size()), (b->getPoint().y * b->size() + c->getPoint().y * c->size()) / (b->size() + c->size()));
        for (auto &ins : c->getInstances())
        {
            b->addInstance(ins);
        }
        b->setPoint(point);
        delete (c);
    }
    else
    {
        Point2<double> point = Point2<double>((b->getPoint().x * b->size() + c->getPoint().x * c->size()) / (b->size() + c->size()), (b->getPoint().y * b->size() + c->getPoint().y * c->size()) / (b->size() + c->size()));
        for (auto &ins : b->getInstances())
        {
            c->addInstance(ins);
        }
        c->setPoint(point);
        delete (b);
    }
}
void MBFFOptimizer::set_best1bitff()

{

    if (best1bitff == nullptr)
    {
        best1bitff = new CellLibrary("best1bitff", "", 1, 1000000, 1000000, 1, 1, 1);
    }
    for (auto &[name, cell] : _name2pFFLibrary)
    {
        if (cell->numBits() == 1)
        {
            double area = cell->width() * cell->height();
            if (area <= get_best1bitff()->width() * get_best1bitff()->height())
            {
                best1bitff = cell;
            }
        }
    }
}
Instance *MBFFOptimizer::merge1BitFF(Instance *FF1, int x, int y, int merge_num, int net_count)
{
    // output Flipflop
    // pint points
    int placement_X = x; // output placement points
    int placement_Y = y;

    string name = "new_inst_" + to_string(_instCnt++) + "_" + to_string(net_count) + "_" + to_string(merge_num);
    // Instance *outputFF = nullptr;
    // cout << "check\n";
    CellLibrary *bestFF = get_best1bitff();
    // cout << bestFF->name() << endl;
    // cout << "check\n";

    Instance *outputFF = new Instance(name, bestFF, placement_X, placement_Y, bestFF->numPins());

    int d0_x = placement_X + bestFF->pPin("D")->x();
    int d0_y = placement_Y + bestFF->pPin("D")->y();
    // cout << d0_x << " " << d0_y << endl;

    InstancePin *D = new InstancePin("", "D", d0_x, d0_y, 0);
    outputFF->addPin(D);

    int q0_x = placement_X + bestFF->pPin("Q")->x();
    int q0_y = placement_Y + bestFF->pPin("Q")->y();

    InstancePin *Q = new InstancePin("", "Q", q0_x, q0_y, 0);
    outputFF->addPin(Q);

    return outputFF;
}

void MBFFOptimizer::placement()
{

    for (const auto &[key, instance] : _name2pInstances_ff) // FF placed, -1000 for weight
    {
        Point2<double> LEFTBOTTOM = Point2<double>(instance->x(), instance->y());
        Point2<double> RIGHTTOP = Point2<double>(instance->x() + instance->pCellLibrary()->width(), instance->y() + instance->pCellLibrary()->height());
        for (const auto &PlacementRow : _pPlacementRows)
        {

            if (PlacementRow->x() <= LEFTBOTTOM.x && RIGHTTOP.y > PlacementRow->y())
            {
                for (unsigned i = 0; i < PlacementRow->numSites(); i++)
                {
                    if (!(PlacementRow->x() + i * PlacementRow->width() > RIGHTTOP.x &&
                          PlacementRow->x() + (i + 1) * PlacementRow->width() > RIGHTTOP.x) &&
                        ((PlacementRow->x() + i * PlacementRow->width() >= LEFTBOTTOM.x &&
                          PlacementRow->x() + (i + 1) * PlacementRow->width() <= RIGHTTOP.x) ||
                         (PlacementRow->x() + i * PlacementRow->width() < RIGHTTOP.x &&
                          PlacementRow->x() + (i + 1) * PlacementRow->width() >= RIGHTTOP.x)))
                    {
                        // cout << i;
                        PlacementRow->setWeight(i, 0); // Flipflop weight placed
                        PlacementRow->Place(i);
                    }
                }
            }
        }
    }
    for (const auto &[key, instance] : _name2pInstances_gate) // Gate placed, -100000 for weight
    {
        Point2<double> LEFTBOTTOM = Point2<double>(instance->x(), instance->y());
        Point2<double> RIGHTTOP = Point2<double>(instance->x() + instance->pCellLibrary()->width(), instance->y() + instance->pCellLibrary()->height());
        for (const auto &PlacementRow : _pPlacementRows)
        {
            if (PlacementRow->x() <= LEFTBOTTOM.x && RIGHTTOP.y > PlacementRow->y())
            {
                for (unsigned i = 0; i < PlacementRow->numSites(); i++)
                {
                    if (!(PlacementRow->x() + i * PlacementRow->width() > RIGHTTOP.x &&
                          PlacementRow->x() + (i + 1) * PlacementRow->width() > RIGHTTOP.x) &&
                        ((PlacementRow->x() + i * PlacementRow->width() >= LEFTBOTTOM.x &&
                          PlacementRow->x() + (i + 1) * PlacementRow->width() <= RIGHTTOP.x) ||
                         (PlacementRow->x() + i * PlacementRow->width() < RIGHTTOP.x &&
                          PlacementRow->x() + (i + 1) * PlacementRow->width() >= RIGHTTOP.x)))
                    {
                        PlacementRow->setWeight(i, -100000);
                        PlacementRow->Place(i);
                    }
                }
            }
        }
    }
}
void MBFFOptimizer::printPlacement()
{
    for (const auto &PlacementRow : _pPlacementRows)
    {
        cout << "PlacementRow: " << PlacementRow->x() << " " << PlacementRow->y() << " " << PlacementRow->width() << " " << PlacementRow->height() << " " << PlacementRow->numSites() << endl;
        for (unsigned i = 0; i < PlacementRow->numSites(); i++)
        {
            // cout << i << "lattice:" << PlacementRow->isPlaced(i) << " ";
            cout << PlacementRow->isPlaced(i) << " ";
        }
        cout << endl;
    }
}
void MBFFOptimizer ::printWeight()
{
    for (const auto &PlacementRow : _pPlacementRows)
    {
        cout << "PlacementRow: " << PlacementRow->x() << " " << PlacementRow->y() << " " << PlacementRow->width() << " " << PlacementRow->height() << " " << PlacementRow->numSites() << endl;
        for (unsigned i = 0; i < PlacementRow->numSites(); i++)
        {
            // cout << i << "lattice:" << PlacementRow->isPlaced(i) << " ";
            cout << PlacementRow->weight(i) << " ";
        }
        cout << endl;
    }
    cout << "Finish printWeight\n";
}
void MBFFOptimizer::printCliqueGraph(Graph *graph)
{
    for (auto &vertex : graph->vertex())
    {
        cout << vertex->name() << " ";
    }
    cout << graph->adj().size() << endl;
    for (auto &[key, value] : graph->adj())
    {
        cout << key->name() << " ";
        for (auto &vertex : value)
        {
            cout << vertex->name() << " ";
        }
        cout << endl;
    }
}

void MBFFOptimizer::PrintOutfile(fstream &outfile)
{
    // //transverse through all the instances, if instance->merged is false, give it a new name and pushback as those mergedInstances
    for (auto & [origName, inst] : _name2pInstances_ff)
    {
        if (!inst->merged)
        {
            // give it a fresh new_inst_... name
            string newName = "new_inst_" 
                             + to_string(_instCnt++) 
                             + "_unmerged";
            inst->setName(newName);

            // push it into the merged list so it's printed below
            placeLegal(inst, Point2<int>(floor((inst->y() - _pPlacementRows[0]->y()) / _pPlacementRows[0]->height()),
                                                        floor((inst->x() - _pPlacementRows[0]->x()) / _pPlacementRows[0]->width())));
            _mergedInstances.push_back(inst);

            // if you need pin-mapping lines, do it here:
            _pinMappings.emplace_back(origName + "/D map " + newName + "/D");
            _pinMappings.emplace_back(origName + "/Q map " + newName + "/Q");
            _pinMappings.emplace_back(origName + "/CLK map " + newName + "/CLK");
        }
    }

    outfile << "CellInst " << _mergedInstances.size() << "\n";
    for (auto* inst : _mergedInstances)
    {
        outfile << "Inst " << inst->name() << " "
                << inst->pCellLibrary()->name() << " "
                << inst->x() << " " << inst->y() << "\n";
    }
    for (const std::string& line : _pinMappings)
    {
        outfile << line << "\n";
    }
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

        if (!found) break;

        if (Sets->at(elem1)->getInstances()[0]->name2pPins()["D"]->timingSlack() > 0)
        {
            for (unsigned j = elem1 + 1; j < Sets->size(); j++)
            {
                if ((j < Sets->size()) &&
                    (Sets->at(j)->getInstances()[0]->name2pPins()["D"]->timingSlack() > 0) &&
                    (!(*visited)[j]) &&
                    ((abs(Sets->at(elem1)->getPoint().x - Sets->at(j)->getPoint().x) +
                      abs(Sets->at(elem1)->getPoint().y - Sets->at(j)->getPoint().y)) <
                     ((Sets->at(elem1)->getInstances()[0]->name2pPins()["D"]->timingSlack() +
                       Sets->at(j)->getInstances()[0]->name2pPins()["D"]->timingSlack()) / _displacementDelay)))
                {
                    count++;
                    flag = true;
                    elem2 = j;
                    (*visited)[j] = true;
                    break;
                }
            }
        }

        if (!flag)
        {
            // One FF only (1-bit)
            Instance *instance;
            int x = Sets->at(elem1)->getInstances()[0]->x();
            int y = Sets->at(elem1)->getInstances()[0]->y();
            string name = "new_inst_" + to_string(_instCnt++) + "_" + to_string(net_count) + "_" + to_string(merge_num);
            instance = merge1BitFF(Sets->at(elem1)->getInstances()[0], x, y, merge_num, net_count);
            instance->setName(name);  // Set the new name for the instance
            merge_success = placeLegal(instance, Point2<int>(floor((y - _pPlacementRows[0]->y()) / _pPlacementRows[0]->height()),
                                                        floor((x - _pPlacementRows[0]->x()) / _pPlacementRows[0]->width())));

            if(merge_success){
                _mergedInstances.push_back(instance);
                Sets->at(elem1)->getInstances()[0]->merged = true; // Mark the instance as merged
                _pinMappings.emplace_back(Sets->at(elem1)->getInstances()[0]->name() + "/D map " + instance->name() + "/D");
                _pinMappings.emplace_back(Sets->at(elem1)->getInstances()[0]->name() + "/Q map " + instance->name() + "/Q");
                _pinMappings.emplace_back(Sets->at(elem1)->getInstances()[0]->name() + "/CLK map " + instance->name() + "/CLK");

                cell_instance++;
            }
        }
        else
        {
            // Try to merge FF1 and FF2 (2-bit)
            Instance *instance;
            double x = (Sets->at(elem1)->getInstances()[0]->name2pPins()["D"]->x() * Sets->at(elem2)->getInstances()[0]->name2pPins()["D"]->timingSlack() +
                        Sets->at(elem2)->getInstances()[0]->name2pPins()["D"]->x() * Sets->at(elem1)->getInstances()[0]->name2pPins()["D"]->timingSlack()) /
                       (Sets->at(elem1)->getInstances()[0]->name2pPins()["D"]->timingSlack() + Sets->at(elem2)->getInstances()[0]->name2pPins()["D"]->timingSlack());

            double y = (Sets->at(elem1)->getInstances()[0]->name2pPins()["D"]->y() * Sets->at(elem2)->getInstances()[0]->name2pPins()["D"]->timingSlack() +
                        Sets->at(elem2)->getInstances()[0]->name2pPins()["D"]->y() * Sets->at(elem1)->getInstances()[0]->name2pPins()["D"]->timingSlack()) /
                       (Sets->at(elem1)->getInstances()[0]->name2pPins()["D"]->timingSlack() + Sets->at(elem2)->getInstances()[0]->name2pPins()["D"]->timingSlack());

            int row = floor((y - _pPlacementRows[0]->y()) / (_pPlacementRows[0]->height()));
            int index = floor((x - _pPlacementRows[0]->x()) / (_pPlacementRows[0]->width()));

            instance = merge2BitFF(Sets->at(elem1)->getInstances()[0], Sets->at(elem2)->getInstances()[0], row, index, merge_num, net_count);

            if (instance)
            {
                string name = "new_inst_" + to_string(_instCnt++) + "_" + to_string(net_count) + "_" + to_string(merge_num);
                instance->setName(name);  // Set the new name for the instance
                merge_success = placeLegal(instance, Point2<int>(floor((y - _pPlacementRows[0]->y()) / _pPlacementRows[0]->height()),
                                                        floor((x - _pPlacementRows[0]->x()) / _pPlacementRows[0]->width())));
                if (merge_success){
                    _mergedInstances.push_back(instance);
                    Sets->at(elem1)->getInstances()[0]->merged = true; // Mark the instance as merged
                    Sets->at(elem2)->getInstances()[0]->merged = true; // Mark the instance as merged
                    _pinMappings.emplace_back(Sets->at(elem1)->getInstances()[0]->name() + "/D map " + instance->name() + "/D0");
                    _pinMappings.emplace_back(Sets->at(elem1)->getInstances()[0]->name() + "/Q map " + instance->name() + "/Q0");
                    _pinMappings.emplace_back(Sets->at(elem2)->getInstances()[0]->name() + "/D map " + instance->name() + "/D1");
                    _pinMappings.emplace_back(Sets->at(elem2)->getInstances()[0]->name() + "/Q map " + instance->name() + "/Q1");
                    _pinMappings.emplace_back(Sets->at(elem1)->getInstances()[0]->name() + "/CLK map " + instance->name() + "/CLK");
                    _pinMappings.emplace_back(Sets->at(elem2)->getInstances()[0]->name() + "/CLK map " + instance->name() + "/CLK");

                    cell_instance++;
                }
            }
            else
            {
                // Fallback to 2 × 1-bit FFs
                int x1 = Sets->at(elem1)->getInstances()[0]->x();
                int y1 = Sets->at(elem1)->getInstances()[0]->y();
                string name1 = "new_inst_" + to_string(_instCnt++) + "_" + to_string(net_count) + "_" + to_string(merge_num);
                Instance *inst1 = merge1BitFF(Sets->at(elem1)->getInstances()[0], x1, y1, merge_num, net_count);
                inst1->setName(name1);  // Set the new name for the instance
                placeLegal(inst1, Point2<int>(floor((y1 - _pPlacementRows[0]->y()) / _pPlacementRows[0]->height()),
                                                        floor((x1 - _pPlacementRows[0]->x()) / _pPlacementRows[0]->width())));
                _mergedInstances.push_back(inst1);
                Sets->at(elem1)->getInstances()[0]->merged = true; // Mark the instance as merged
                _pinMappings.emplace_back(Sets->at(elem1)->getInstances()[0]->name() + "/D map " + inst1->name() + "/D");
                _pinMappings.emplace_back(Sets->at(elem1)->getInstances()[0]->name() + "/Q map " + inst1->name() + "/Q");
                _pinMappings.emplace_back(Sets->at(elem1)->getInstances()[0]->name() + "/CLK map " + inst1->name() + "/CLK");
                cell_instance++;

                int x2 = Sets->at(elem2)->getInstances()[0]->x();
                int y2 = Sets->at(elem2)->getInstances()[0]->y();
                string name2 = "new_inst_" + to_string(_instCnt++) + "_" + to_string(net_count) + "_" + to_string(merge_num + 1);
                Instance *inst2 = merge1BitFF(Sets->at(elem2)->getInstances()[0], x2, y2, merge_num + 1, net_count);
                inst2->setName(name2);  // Set the new name for the instance
                placeLegal(inst2, Point2<int>(floor((y2 - _pPlacementRows[0]->y()) / _pPlacementRows[0]->height()),
                                                        floor((x2 - _pPlacementRows[0]->x()) / _pPlacementRows[0]->width())));
                _mergedInstances.push_back(inst2);
                Sets->at(elem2)->getInstances()[0]->merged = true; // Mark the instance as merged
                _pinMappings.emplace_back(Sets->at(elem2)->getInstances()[0]->name() + "/D map " + inst2->name() + "/D");
                _pinMappings.emplace_back(Sets->at(elem2)->getInstances()[0]->name() + "/Q map " + inst2->name() + "/Q");
                _pinMappings.emplace_back(Sets->at(elem2)->getInstances()[0]->name() + "/CLK map " + inst2->name() + "/CLK");
                cell_instance++;
            }
        }

        merge_num++;
    }
}

bool sort_alg(CellLibrary *a, CellLibrary *b)
{
    return a->width() * (a->height()) < b->width() * (b->height());
}

void MBFFOptimizer::findFeasable_reg(Net *net, fstream &outfile, int net_count)
{
    vector<DisSet *> *Sets = new vector<DisSet *>;
    vector<bool> *visited = new vector<bool>(net->numPins() - 1, false);
    
    // First, collect all FFs in this net
    for (unsigned i = 1; i < net->numPins(); i++)
    {
        if (net->pPins()[i].pin->getinstance())
        {
            DisSet *set = new DisSet(net->pPins()[i].pin->getinstance(), net->pPins()[i].pin->x(), net->pPins()[i].pin->y());
            Sets->push_back(set);
        }
    }

    // Process FFs that can be merged
    Synthesize(Sets, visited, outfile, net_count);

    // Handle lonely FFs (those that weren't merged)
    for (unsigned i = 0; i < Sets->size(); i++)
    {
        if (!(*visited)[i])
        {
            // This is a lonely FF that wasn't merged
            Instance *instance;
            int x = Sets->at(i)->getInstances()[0]->x();
            int y = Sets->at(i)->getInstances()[0]->y();
            string name = "new_inst_" + to_string(_instCnt++) + "_" + to_string(net_count) + "_lonely_" + to_string(i);
            
            instance = merge1BitFF(Sets->at(i)->getInstances()[0], x, y, i, net_count);
            instance->setName(name);
            placeLegal(instance, Point2<int>(floor((y - _pPlacementRows[0]->y()) / _pPlacementRows[0]->height()),
                                                        floor((x - _pPlacementRows[0]->x()) / _pPlacementRows[0]->width())));

            _mergedInstances.push_back(instance);
            Sets->at(i)->getInstances()[0]->merged = true; // Mark the instance as merged
            _pinMappings.emplace_back(Sets->at(i)->getInstances()[0]->name() + "/D map " + instance->name() + "/D");
            _pinMappings.emplace_back(Sets->at(i)->getInstances()[0]->name() + "/Q map " + instance->name() + "/Q");
            _pinMappings.emplace_back(Sets->at(i)->getInstances()[0]->name() + "/CLK map " + instance->name() + "/CLK");
            
            cell_instance++;
        }
    }

    // Clean up
    for (auto set : *Sets)
    {
        delete set;
    }
    delete Sets;
    delete visited;
}

void MBFFOptimizer::print_weight_matrix(vector<vector<double> *> *weight_matrix)
{
    for (unsigned int i = 0; i < weight_matrix->size(); i++)
    {
        for (unsigned int j = 0; j < weight_matrix->at(i)->size(); j++)
        {
            cout << weight_matrix->at(i)->at(j) << " ";
        }
        cout << endl;
    }
}
void MBFFOptimizer::algorithm(std::string baseName)
{
    fstream outfile; // open file in write mode
    outfile.open(_outfile, ios::out);
    int net_count = 0;
    bool all_clk = true;    // we have to check all output pins are clk case

    //plot every 10% of net
    int count = 0;
    for (auto net : _pNets)
    {
        all_clk = true;
        if (net->numPins() <= 1) continue;

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
    std::cout << "Total nets to process: " << count <<", interval: " << ploting_interval<< "\n";
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
        else if (all_clk && net->numPins() > 1) // find feasable region on net [1:]
        {
            // cout << "123\n";
            net_count++;
            findFeasable_reg(net, outfile, net_count);
        }

        count++;
        if(count % 100 == 0)
        {
            std::cout << "Processing net: " << count  << "\n";
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


Instance *MBFFOptimizer::merge2BitFF(Instance *FF1, Instance *FF2, int x, int y, int merge_num, int net_count)
{
    int attempts = 0;
    Point2<int> placement_point;

    // pint points
    int placement_X; // output placement points
    int placement_Y;

    int q1_x = FF1->name2pPins()["D"]->x();
    int q1_y = FF1->name2pPins()["D"]->y();

    int q2_x = FF2->name2pPins()["D"]->x();
    int q2_y = FF2->name2pPins()["D"]->y();
    // cout << x << " " << y << endl;
    // output Flipflop
    string name = "new_inst_" + to_string(_instCnt++) + "_" + to_string(net_count) + "_" + to_string(merge_num);
    Instance *outputFF = nullptr;
    CellLibrary *bestFF;

    while (attempts < get_bit2_ff().size())
    {
        bestFF = get_bit2_ff()[attempts];

        placement_point = ((FF1->x()+FF2->x())/2, (FF1->y()+FF2->y())/2);
        // cout << "ghjk" << endl;
        placement_X = placement_point.y;
        placement_Y = placement_point.x;
        // cout << placement_X << " " << placement_Y << endl;
        //  ---------- check if still in feasable region ------------//
        if (placement_X)
        {

            //  1. get manhatten distance of displacement from flip flops
            double manhatten_distance_1 = abs(q1_x - _pPlacementRows[0]->x() - placement_X * _pPlacementRows[0]->width()) + abs(q1_y - _pPlacementRows[0]->y() - placement_Y * _pPlacementRows[0]->height());
            double manhatten_distance_2 = abs(q2_x - _pPlacementRows[0]->x() - placement_X * _pPlacementRows[0]->width()) + abs(q2_y - _pPlacementRows[0]->y() - placement_Y * _pPlacementRows[0]->height());

            //  2. calculate if slack is still positive
            double slack1 = FF1->name2pPins()["D"]->timingSlack() - manhatten_distance_1 * _displacementDelay;
            double slack2 = FF2->name2pPins()["D"]->timingSlack() - manhatten_distance_1 * _displacementDelay;

            if ((slack1 > 0) and (slack2 > 0))
            {
                // generate_name
                name = name + to_string(net_count) + "_" + to_string(merge_num);

                // create new output flip flop
                outputFF = new Instance(name, bestFF, placement_X * _pPlacementRows[0]->width() + _pPlacementRows[0]->x(), placement_Y * _pPlacementRows[0]->height() + _pPlacementRows[0]->y(), bestFF->numPins());

                // set new output pins
                // calculate output pin position => D0, D1, Q0, Q1;

                int d0_x = placement_X + bestFF->pPin("D0")->x();
                int d0_y = placement_Y + bestFF->pPin("D0")->y();

                InstancePin *D_0 = new InstancePin("", "D0_", d0_x, d0_y, 0);
                outputFF->addPin(D_0);

                int d1_x = placement_X + bestFF->pPin("D1")->x();
                int d1_y = placement_Y + bestFF->pPin("D1")->y();

                InstancePin *D_1 = new InstancePin("", "D1_", d1_x, d1_y, 0);
                outputFF->addPin(D_1);

                int o_q0_x = placement_X + bestFF->pPin("Q0")->x();
                int o_q0_y = placement_Y + bestFF->pPin("Q0")->y();

                InstancePin *Q_0 = new InstancePin("", "Q0_", o_q0_x, o_q0_y, 0);
                outputFF->addPin(Q_0);

                int o_q1_x = placement_X + bestFF->pPin("Q1")->x();
                int o_q1_y = placement_Y + bestFF->pPin("Q1")->y();

                InstancePin *Q_1 = new InstancePin("", "Q1_", o_q1_x, o_q1_y, 0);
                outputFF->addPin(Q_1);

                break;
            }
            else
            {
                attempts = attempts + 1;
            }
        }
    }
    return outputFF;
}
Point2<int> MBFFOptimizer::find_legal_position(int row, int col)
{
    int row_index = row;
    int col_index = col;
    std::queue<Point2<int>> toVisit;
    // std::set<Point2<int>> visited;
    unordered_map<Point2<int>, bool, PointHash<int>> visited;

    toVisit.push(Point2<int>(row_index, col_index));

    while (!toVisit.empty())
    {
        Point2<int> current = toVisit.front();
        toVisit.pop();

        if (visited.find(current) != visited.end())
        { // check if current is already visited
            continue;
        }
        visited.insert({current, true}); // mark current as visited

        if (!_pPlacementRows[current.x]->isoccupied(current.y))
        {
            return current;
        }

        std::vector<Point2<int>> neighbors = {
            {current.x + 1, current.y}, {current.x - 1, current.y}, {current.x, current.y + 1}, {current.x, current.y - 1}};

        for (const auto &neighbor : neighbors)
        {
            if (neighbor.x >= 0 && neighbor.x < _pPlacementRows.size() && neighbor.y >= 0 && neighbor.y < _pPlacementRows[0]->numSites() && visited.find(neighbor) == visited.end())
            {
                toVisit.push(neighbor);
            }
        }
    }
    cout << "no legal position found\n";
    return Point2<int>(-1, -1);
}