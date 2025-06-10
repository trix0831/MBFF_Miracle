#include "MBFFOptimizer.h"
#include <algorithm>

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

    string name = "Fout_" + to_string(net_count) + "_" + to_string(merge_num);
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
// void MBFFOptimizer::DFS(unordered_map<row_index_pair, vector<Instance *>, row_index_pair_hash> *occupied,
//                         Point2<double> point, vector<vector<bool> *> *visited,
//                         vector<vector<double> *> *weight_matrix, long long int row,
//                         long long int index, double slack, unsigned vertex_count, Graph *clique_graph, Instance *instance)
// { /// PROBLEM: May not found overlap region but found overlap point
//     // cout << row << " " << index << endl;

//     if (index >= 0 && index < _pPlacementRows[0]->numSites() && row >= 0 && row < _pPlacementRows.size())
//     {

//         if (visited->at(row)->at(index) == true)
//         {
//             return;
//         }
//         visited->at(row)->at(index) = true;
//         // point include x and y on die
//         // print_weight_matrix(weight_matrix);
//         double slack_earn = slack - (abs(point.x - (index * _pPlacementRows[row]->width() + _pPlacementRows[row]->x())) + abs(point.y - (row * _pPlacementRows[0]->height() + _pPlacementRows[0]->y())));
//         // std::cout << "slack value: " << slack_earn << endl;
//         if (slack_earn >= 0)
//         {
//             cout << instance->name() << " " << row << " " << index << endl;
//             weight_matrix->at(row)->at(index) += slack_earn;
//             struct row_index_pair temp;
//             temp.row = row;
//             temp.index = index;
//             // cout << occupied[temp].size() << endl;
//             if ((*occupied)[temp].size() == 0)
//             {
//                 // cout << occupied[temp].size() << endl;
//                 // cout << instance->name() << endl;

//                 (*occupied)[temp].push_back(instance);
//             }
//             else
//             {
//                 cout << "123\n";
//                 for (long unsigned int i = 0; i < (*occupied)[temp].size(); i++)
//                 {
//                     clique_graph->addEdge((*occupied)[temp][i], instance);
//                 }
//                 (*occupied)[temp].push_back(instance);
//             }
//         }
//         else
//         {
//             return;
//         }
//         // cout << instance->name() << endl;
//         // for (const auto &[key, value] : occupied)
//         // {
//         //     cout << key.row << " " << key.index << " ";

//         //     for (int i = 0; i < value.size(); i++)
//         //     {
//         //         cout << value[i]->name() << " " << endl;
//         //     }
//         // }
//         // printCliqueGraph(clique_graph);
//         // print_weight_matrix(weight_matrix);
//         // cout << "123\n";
//         visited->at(row)->at(index) = true;
//         DFS(occupied, point, visited, weight_matrix, row + 1, index, slack, vertex_count, clique_graph, instance);
//         DFS(occupied, point, visited, weight_matrix, row - 1, index, slack, vertex_count, clique_graph, instance);
//         DFS(occupied, point, visited, weight_matrix, row, index + 1, slack, vertex_count, clique_graph, instance);
//         DFS(occupied, point, visited, weight_matrix, row, index - 1, slack, vertex_count, clique_graph, instance);
//     }
//     else
//     {
//     }
// }
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
void MBFFOptimizer::Synthesize(vector<DisSet *> *Sets, vector<bool> *visited, fstream &outfile, int net_count)
{
    unsigned count = 0;
    unsigned elem1 = 0;
    unsigned elem2 = 0;
    int merge_num = 0;
    // outfile << net_count << " ";
    // outfile << visited->size() << " " << Sets->size() << endl;
    // cout << "set size = " << Sets->size() << "\n"
    //      << endl;
    // cout << "visted size = " << visited->size() << "\n"
    //      << endl;
    // for (const auto &set : *Sets)
    // {
    //     cout << set->getInstances()[0]->name() << " " << set->getInstances()[0]->x() << " " << set->getInstances()[0]->y() << " " << set->getInstances()[0]->name2pPins()["D"]->timingSlack() << endl;
    //     cout << set->getInstances()[0]->name() << " x_coor: " << set->getPoint().x << " y_coor : " << set->getPoint().y << "Timing slack " << set->getInstances()[0]->name2pPins()["D"]->timingSlack() << endl;
    // }
    while (count < visited->size())
    {
        bool flag = false;
        bool found = false;
        // find first unvisted element
        for (unsigned i = 0; i < Sets->size(); i++)
        {

            if ((*visited)[i] == false)
            {
                count++;
                elem1 = i;
                found = true;
                (*visited)[i] = true;
                // cout << "ran";
                break;
            }
        }

        if (!found)
        {
            break;
        }
        if (Sets->at(elem1)->getInstances()[0]->name2pPins()["D"]->timingSlack() > 0)
        {

            for (unsigned j = elem1 + 1; j < Sets->size(); j++)
            {
                if ((j < Sets->size()) && (Sets->at(j)->getInstances()[0]->name2pPins()["D"]->timingSlack() > 0) && ((*visited)[j] == false) && ((abs(Sets->at(elem1)->getPoint().x - Sets->at(j)->getPoint().x) + abs(Sets->at(elem1)->getPoint().y - Sets->at(j)->getPoint().y)) < ((Sets->at(elem1)->getInstances()[0]->name2pPins()["D"]->timingSlack() + Sets->at(j)->getInstances()[0]->name2pPins()["D"]->timingSlack()) / _displacementDelay)))
                {
                    count++;
                    flag = true;
                    elem2 = j;
                    (*visited)[j] = true;

                    // cout << "ran unreadable code" << endl;
                    break;
                }
            }
        }

        if (!flag)
        {
            Instance *instance;
            int x = Sets->at(elem1)->getInstances()[0]->x();
            int y = Sets->at(elem1)->getInstances()[0]->y();
            // cout << "abababababa\n";
            instance = merge1BitFF(Sets->at(elem1)->getInstances()[0], x, y, merge_num, net_count);
            // cout << "123123213213\n";
            // outfile << net_count << endl;
            // output_map[instance->name()] = instance;
            cell_instance++;
            outfile << "CellInst " << cell_instance << endl;
            outfile << "Inst " << instance->name() << " " << instance->pCellLibrary()->name() << " " << instance->x() << " " << instance->y() << endl;
            outfile << Sets->at(elem1)->getInstances()[0]->name() << "/" << "D" << " map " << instance->name() << "/D" << endl;
            outfile << Sets->at(elem1)->getInstances()[0]->name() << "/" << "Q" << " map " << instance->name() << "/Q" << endl;
        }
        else
        {
            // cout << "merge!!!!" << endl;
            Instance *instance;
            // cout << Sets->at(elem1)->getInstances()[0]->name() << " " << Sets->at(elem2)->getInstances()[0]->name() << endl;
            // cout << Sets->at(elem1)->getPoint().x;
            double x = (Sets->at(elem1)->getInstances()[0]->name2pPins()["D"]->x() * Sets->at(elem2)->getInstances()[0]->name2pPins()["D"]->timingSlack() + Sets->at(elem2)->getInstances()[0]->name2pPins()["D"]->x() * Sets->at(elem1)->getInstances()[0]->name2pPins()["D"]->timingSlack()) / (Sets->at(elem1)->getInstances()[0]->name2pPins()["D"]->timingSlack() + Sets->at(elem2)->getInstances()[0]->name2pPins()["D"]->timingSlack());
            double y = (Sets->at(elem1)->getInstances()[0]->name2pPins()["D"]->y() * Sets->at(elem2)->getInstances()[0]->name2pPins()["D"]->timingSlack() + Sets->at(elem2)->getInstances()[0]->name2pPins()["D"]->y() * Sets->at(elem1)->getInstances()[0]->name2pPins()["D"]->timingSlack()) / (Sets->at(elem1)->getInstances()[0]->name2pPins()["D"]->timingSlack() + Sets->at(elem2)->getInstances()[0]->name2pPins()["D"]->timingSlack());
            // cout << x << " " << y << endl;
            int row = floor((y - _pPlacementRows[0]->y()) / (_pPlacementRows[0]->height()));
            int index = floor((x - _pPlacementRows[0]->x()) / (_pPlacementRows[0]->width()));
            // cout << "row: " << row << "index: " << index;
            // cout << "abc\n";
            // cout << net_count << endl;
            instance = merge2BitFF(Sets->at(elem1)->getInstances()[0], Sets->at(elem2)->getInstances()[0], row, index, merge_num, net_count);
            // cout << "abcdefghijklmnop\n";
            if (instance)
            {
                cell_instance++;
                outfile << "CellInst " << cell_instance << endl;
                outfile << "Inst " << instance->name() << " " << instance->pCellLibrary()->name() << " " << instance->x() << " " << instance->y() << endl;
                outfile << Sets->at(elem1)->getInstances()[0]->name() << "/" << "D" << " map " << instance->name() << "/D0" << endl;
                outfile << Sets->at(elem1)->getInstances()[0]->name() << "/" << "Q" << " map " << instance->name() << "/Q0" << endl;
                outfile << Sets->at(elem2)->getInstances()[0]->name() << "/" << "D" << " map " << instance->name() << "/D1" << endl;
                outfile << Sets->at(elem2)->getInstances()[0]->name() << "/" << "Q" << " map " << instance->name() << "/Q1" << endl;
                // outfile << Sets->at(elem1)->getInstances()[0]->name() << " map to " << instance->name() << " " << instance->x() << " " << instance->y() << endl;
                // outfile << Sets->at(elem2)->getInstances()[0]->name() << " map to " << instance->name() << " " << instance->x() << " " << instance->y() << endl;
            }
            else
            {
                Instance *instance;
                cell_instance++;
                int x = Sets->at(elem1)->getInstances()[0]->x();
                int y = Sets->at(elem1)->getInstances()[0]->y();
                // cout << "abababababa\n";
                instance = merge1BitFF(Sets->at(elem1)->getInstances()[0], x, y, merge_num, net_count);
                // cout << "123123213213\n";
                // output_map[instance->name()] = instance;
                outfile << "CellInst " << cell_instance << endl;
                outfile << "Inst " << instance->name() << " " << instance->pCellLibrary()->name() << " " << instance->x() << " " << instance->y() << endl;
                outfile << Sets->at(elem1)->getInstances()[0]->name() << "/" << "D" << " map " << instance->name() << "/D" << endl;
                outfile << Sets->at(elem1)->getInstances()[0]->name() << "/" << "Q" << " map " << instance->name() << "/Q" << endl;
                merge_num++;
                cell_instance++;
                x = Sets->at(elem2)->getInstances()[0]->x();
                y = Sets->at(elem2)->getInstances()[0]->y();
                // cout << "abababababa\n";
                instance = merge1BitFF(Sets->at(elem2)->getInstances()[0], x, y, merge_num, net_count);
                // cout << "123123213213\n";
                // output_map[instance->name()] = instance;
                outfile << "CellInst " << cell_instance << endl;
                outfile << "Inst " << instance->name() << " " << instance->pCellLibrary()->name() << " " << instance->x() << " " << instance->y() << endl;
                outfile << Sets->at(elem2)->getInstances()[0]->name() << "/" << "D" << " map " << instance->name() << "/D" << endl;
                outfile << Sets->at(elem2)->getInstances()[0]->name() << "/" << "Q" << " map " << instance->name() << "/Q" << endl;
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
    // outfile << net->numPins() << " " << net->name() << endl;
    vector<DisSet *> *Sets = new vector<DisSet *>;
    vector<bool> *visited = new vector<bool>(net->numPins() - 1, false);
    for (unsigned i = 1; i < net->numPins(); i++)
    {
        // cout << "123";
        // cout << net->pin(i).pin->getinstance() << endl;
        if (net->pPins()[i].pin->getinstance())
        {
            // cout << net->pPins()[i].pin->getinstance() << endl;
            // cout << net->pPins()[i].inst_name << " " << net->pPins()[i].pin->getinstance()->name2pPins()["D"]->timingSlack();
            DisSet *set = new DisSet(net->pPins()[i].pin->getinstance(), net->pPins()[i].pin->x(), net->pPins()[i].pin->y());
            // cout << set->getInstances()[0]->name() << endl;
            Sets->push_back(set);
        }
    }
    Synthesize(Sets, visited, outfile, net_count);
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
void MBFFOptimizer::algorithm()
{
    fstream outfile; // open file in write mode
    outfile.open(_outfile, ios::out);
    int net_count = 0;
    bool all_clk = true;    // we have to check all output pins are clk case
    for (auto net : _pNets) // at each net, we have to redo weight_matrix and at the end of
    {
        // cout << "123" << endl;
        all_clk = true; // each net, we have to reset all placementRow
        // cout << "123\n";
        // vector<vector<double> *> *weight_matrix = new vector<vector<double> *>;
        // for (unsigned i = 0; i < _pPlacementRows.size(); i++)
        // {
        //     // cout << i << endl;
        //     vector<double> *weight_row = new vector<double>;
        //     for (unsigned int j = 0; j < _pPlacementRows[i]->numSites(); j++)
        //     {
        //         // cout << _pPlacementRows[i]->weight(j) << endl;
        //         weight_row->push_back(_pPlacementRows[i]->weight(j));
        //         // cout << _pPlacementRows[i]->weight(j) << " ";
        //         // cout << weight_row->at(j) << " ";
        //         // weight_matrix[i][j] = _pPlacementRows[i]->weight(j);
        //     }
        //     weight_matrix->push_back(weight_row);

        // findFeasable_reg(net, weight_matrix);
        // cout << weight_matrix[0][0] << endl;
        // print_weight_matrix(weight_matrix);
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
    }
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
    string name = "Fout_";
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
