#include "MBFFOptimizer.h"
#include <algorithm>
#include "visualizer.h"

// Assuming 2bifffLibrary is a vector of BiFFFLibrary objects

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

            _mergedInstances.push_back(instance);
            Sets->at(elem1)->getInstances()[0]->merged = true; // Mark the instance as merged
            _pinMappings.emplace_back(Sets->at(elem1)->getInstances()[0]->name() + "/D map " + instance->name() + "/D");
            _pinMappings.emplace_back(Sets->at(elem1)->getInstances()[0]->name() + "/Q map " + instance->name() + "/Q");
            _pinMappings.emplace_back(Sets->at(elem1)->getInstances()[0]->name() + "/CLK map " + instance->name() + "/CLK");

            cell_instance++;
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
            else
            {
                // Fallback to 2 × 1-bit FFs
                int x1 = Sets->at(elem1)->getInstances()[0]->x();
                int y1 = Sets->at(elem1)->getInstances()[0]->y();
                string name1 = "new_inst_" + to_string(_instCnt++) + "_" + to_string(net_count) + "_" + to_string(merge_num);
                Instance *inst1 = merge1BitFF(Sets->at(elem1)->getInstances()[0], x1, y1, merge_num, net_count);
                inst1->setName(name1);  // Set the new name for the instance
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
// void MBFFOptimizer::Preset()
// {
//     // find best 1bit ff
//     for (auto &[name, cell] : _name2pFFLibrary)
//     {
//         if (cell->numBits() == 1)
//         {
//             double area = cell->width() * cell->height();
//             if (area <= get_best1bitff()->width() * get_best1bitff()->height())
//             {
//                 set_best1bitff(cell);
//             }
//         }
//     }
//     sort(_2bitffCellLibrary.begin(), _2bitffCellLibrary.end(), sort_alg);
//     // sort 2bit ff from lowest area to largest
// }
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

        if (count % ploting_interval == 0)
        {
            std::cout << "Plotting progress: " << count/ploting_interval*10 << "%\n";
            plotMerge(dieWidth(), dieHeight(), _name2pInstances_ff, _name2pInstances_gate, _mergedInstances, baseName, count /ploting_interval);
        }
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
    // cout << "-------------------------Pin Correspond to Net-------------------" << endl;
    // for (const auto &[key, instance] : _name2pInstances_ff)
    // {
    //     cout << instance->name() << endl;
    //     for (const auto &[key, pin] : instance->name2pPins())
    //     {
    //         cout << pin->pNet()->name() << " " << pin->name() << " " << pin->timingSlack() << endl;
    //     }
    // }
};

// void MBFFOptimizer::check_disjoint_set()
// {
//     _pflipflops.clear();
//     _pflipflops.clear();
//     int instance_count = 0;
//     cout << "constructing disjoint set\n";
//     for (const auto &ins : _name2pInstances_ff)
//     {
//         _pflipflops.push_back(ins.second);
//         DisjointSet *disjointset = new DisjointSet(instance_count);
//         disjointset->makeSet();
//         disjointset->setRank(0);
//         disjointset->setroot(instance_count);
//         _disjointSets.push_back(disjointset);
//         instance_count++;
//     }
//     assert(_disjointSets.size() == _pflipflops.size());
//     assert(_pflipflops.size() == _name2pInstances_ff.size());
// }

void MBFFOptimizer::init_occupied()
{
    cout << "init occupied\n";
    for (unsigned i = 0; i < _pPlacementRows.size(); i++)
    {
        _pPlacementRows[i]->initoccupied();
    }
    for (const auto &gate : _name2pInstances_gate)
    { // second is the gate
        int gate_row_bottom = (gate.second->y() - _pPlacementRows[0]->y()) / _pPlacementRows[0]->height();
        int gate_row_top = (gate.second->y() + gate.second->pCellLibrary()->height() - _pPlacementRows[0]->y()) / _pPlacementRows[0]->height();
        int gate_index_left = (gate.second->x() - _pPlacementRows[0]->x()) / _pPlacementRows[0]->width();
        int gate_index_right = (gate.second->x() + gate.second->pCellLibrary()->width() - _pPlacementRows[0]->x()) / _pPlacementRows[0]->width();
        // cout<<"gate row bottom: "<<gate_row_bottom<<" gate row top: "<<gate_row_top<<" gate index left: "<<gate_index_left<<" gate index right: "<<gate_index_right<<endl;
        for (int i = gate_row_bottom; i <= gate_row_top; i++)
        {
            for (int j = gate_index_left; j <= gate_index_right; j++)
            {
                _pPlacementRows[i]->setoccupied(j, true);
                // cout<<(gate_row_top - gate_row_bottom+1) * (gate_index_right-gate_index_left+1)<<"   ";
            }
        }
        // cout<<endl;
    }

    cout << "finish init occupied\n";
    int count = 0;
    for (unsigned int i = 0; i < _pPlacementRows.size(); i++)
    {
        for (unsigned int j = 0; j < _pPlacementRows[i]->numSites(); j++)
        {
            // cout<<_pPlacementRows[i]->isoccupied(j)<<" ";
            if (_pPlacementRows[i]->isoccupied(j))
            {
                count++;
            }
        }
    }
    cout << count << endl;
}
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

        placement_point = find_legal_position(x, y);
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
// void MBFFOptimizer::print_ff_change()
// {
//     cout << "\nPrint Output\n";
//     for (const auto &[str, instance] : output_map)
//     {
//         cout << str << " " << instance->name() << endl;
//     }
// }


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
    vector<Instance*> L;
    vector<Instance*> target2k, targetk;
    target2k = findknear(InstanceName, k * 2);
    for(int i = 0; i < k; i++){
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

vector<Instance*> MBFFOptimizer::findknear(string InstanceName, int k)
{
    vector<Instance*> L;
    vector<Instance*> target;
    double x = _name2pInstances_ff[InstanceName]->x();
    double y = _name2pInstances_ff[InstanceName]->y();
    L.push_back(_name2pInstances_ff[InstanceName]);
    int left = 0;
    int right = _bucket.bucket_query(x, 0).size() - 1;
    int index;
    // cout << "left0: " << left << " right0: " << right << endl;
    while(left <= right){
        int mid = (left + right) / 2;
        if(_bucket.bucket_query(x, 0)[mid]->y() < y){
            left = mid + 1;
        }else if(_bucket.bucket_query(x, 0)[mid]->y() > y){
            right = mid - 1;
        }else{
            index = mid;
            break;
        }
        index = mid;
    }
    if(index + 1 < _bucket.bucket_query(x, 0).size()){
        L.push_back(_bucket.bucket_query(x, 0)[index + 1]);
    }
    if(index - 1 >= 0){
        L.push_back(_bucket.bucket_query(x, 0)[index - 1]);
    }
    
    int find_right = 1;
    left = 0;
    right = _bucket.bucket_query(x, find_right).size() - 1;
    while(right == -1 && find_right < 5){
        find_right++;
        right = _bucket.bucket_query(x, find_right).size() - 1;
    }
    // cout << "left1: " << left << " right1: " << right << endl;
    if(right != -1){
        while(left <= right){
            int mid = (left + right) / 2;
            // cout << "mid: " << mid << endl;
            // cout << _bucket.bucket_query(x, find_right).size() <<endl;
            if(_bucket.bucket_query(x, find_right)[mid]->y() < y){
                left = mid + 1;
            }else if(_bucket.bucket_query(x, find_right)[mid]->y() > y){
                right = mid - 1;
            }else{
                index = mid;
                break;
            }
            index = mid;
        }
        // cout <<"hey"<<endl;
        L.push_back(_bucket.bucket_query(x, find_right)[index]);
        if(index + 1 < _bucket.bucket_query(x, find_right).size()){
            L.push_back(_bucket.bucket_query(x, find_right)[index + 1]);
        }
        if(index - 1 >= 0){
            L.push_back(_bucket.bucket_query(x, find_right)[index - 1]);
        }
    }

    int find_left = -1;
    left = 0;
    right = _bucket.bucket_query(x, -1).size() - 1;
    while(right == -1 && find_left > -5){
        find_left--;
        right = _bucket.bucket_query(x, find_left).size() - 1;
    }
    // cout << "left-1: " << left << " right-1: " << right << endl;
    if(right != -1){
        while(left <= right){
            int mid = (left + right) / 2;
            if(_bucket.bucket_query(x, find_left)[mid]->y() < y){
                left = mid + 1;
            }else if(_bucket.bucket_query(x, find_left)[mid]->y() > y){
                right = mid - 1;
            }else{
                index = mid;
                break;
            }
            index = mid;
        }
        L.push_back(_bucket.bucket_query(x, find_left)[index]);
        if(index + 1 < _bucket.bucket_query(x, find_left).size()){
            L.push_back(_bucket.bucket_query(x, find_left)[index + 1]);
        }
        if(index - 1 >= 0){
            L.push_back(_bucket.bucket_query(x, find_left)[index - 1]);
        }
    }

    if(L.size() < k)
    {
        cout << "not enough elements in bucket\n";
        return L;
    }

    // Sort the vector based on the distance to the target point
    sort(L.begin(), L.end(), [x, y](Instance* a, Instance* b) {
        double dist_a = (a->x() - x) * (a->x() - x) + (a->y() - y) * (a->y() - y);
        double dist_b = (b->x() - x) * (b->x() - x) + (b->y() - y) * (b->y() - y);
        return dist_a < dist_b;
    });

    // cout <<"k: "<<k<<endl;
    for(int i = 0; i < L.size(); i ++){
        if(i >= k){
            break;
        }
        target.push_back(L[i]);
    }

    return target;
}


