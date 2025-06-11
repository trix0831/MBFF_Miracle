#ifndef MBFFOPTIMIZER_H
#define MBFFOPTIMIZER_H

#include <fstream>
#include <vector>
#include <map>
#include <sstream>
#include <unordered_map>
#include <string>
#include "Point.h"
#include <queue>
#include "CellLibrary.h"
#include "Instance.h"
#include "Pin.h"
#include "Net.h"
#include "PlacementRow.h"
#include "Point.h"
#include <iostream>
#include "Graph.h"
#include <cassert>
#include <cmath>
#include <math.h>
#include "DisSet.h"
#include <algorithm>
#include "BucketList.h"
using namespace std;

class MBFFOptimizer
{
public:
  MBFFOptimizer(fstream &inFile, string out_file)
  {
    parseInput(inFile);
    _outfile = out_file;
    cell_instance = 0;
    // best1bitff = new CellLibrary("123", "123", 1, 1000000, 1000000, 0);
  }

  ~MBFFOptimizer();
  Instance *merge2BitFF(Instance *FF1, Instance *FF2, int x, int y, int merge_num, int net_count);
  Instance *merge1BitFF(Instance *FF1, int x, int y, int merge_num, int net_count);
  unsigned           _instCnt;
  void printPlacement();
  void algorithm();
  void placement();
  void printWeight();
  void printInput();
  void PrintOutfile(fstream &outfile);
  void init_occupied();
  // void Preset();
  void set_best1bitff();
  double score(string InstanceName, int k);
  vector<Instance*> findknear(string InstanceName, int k);
  double HPWL(vector<Instance *> *pInstances); 
  // void print_ff_change();
  CellLibrary *get_best1bitff()
  {
    return best1bitff;
  }
  unsigned cell_instance;
  // void set_best1bitff(CellLibrary *pCellLibrary)
  // {
  //   best1bitff = pCellLibrary;
  // }
  // void check_disjoint_set();
  void Synthesize(vector<DisSet *> *Sets, vector<bool> *visited, fstream &outfile, int net_count = 0);
  Point2<int> find_legal_position(int row, int index);
  vector<CellLibrary *> get_bit2_ff()
  {
    return _2bitffCellLibrary;
  }
  void store_bit2_ff(CellLibrary *pCellLibrary)
  {
    _2bitffCellLibrary.push_back(pCellLibrary);
  }
  struct row_index_pair
  {
    int row;
    int index;

    // Equality operator
    bool operator==(const row_index_pair &other) const
    {
      return row == other.row && index == other.index;
    }
  };

  // Implement the hash function for row_index_pair
  struct row_index_pair_hash
  {
    std::size_t operator()(const row_index_pair &p) const
    {
      auto hash1 = std::hash<int>{}(p.row);
      auto hash2 = std::hash<int>{}(p.index);
      return hash1 ^ (hash2 << 1); // Combine the two hash values
    }
  };

    // Store all new merged instances in order
  std::vector<Instance*> _mergedInstances;

  // Store pin mappings: each line like "C1/D map C3/D0"
  std::vector<std::string> _pinMappings;




private:
  // Weight factors
  void DFS(unordered_map<row_index_pair, vector<Instance *>, row_index_pair_hash> *occupied,
           Point2<double> point, vector<vector<bool> *> *visited,
           vector<vector<double> *> *weight_matrix, long long int row,
           long long int index, double slack, unsigned vertex_count, Graph *clique_graph, Instance *instance);
  // point is the pin x,y where slack is pos.
  double _alpha;
  double _beta;
  double _gamma;
  double _lambda;
  double c1 = 1;
  double c2 = 1;
  string _outfile;
  // Die size
  Point2<double> _dieLeftBottom;
  Point2<double> _dieRightTop;
  Point2<double> _placementRowLeftBottom;
  Point2<double> _placementRowRightTop;
  // Inputs and outputs
  unsigned _numInputPins;
  unordered_map<string, InstancePin *> _name2pInputPins;
  unsigned _numOutputPins;
  unordered_map<string, InstancePin *> _name2pOutputPins;
  // Cell Library
  unordered_map<string, CellLibrary *> _name2pFFLibrary;
  unordered_map<string, CellLibrary *> _name2pGateLibrary;
  // Instances
  unsigned _numInstances;
  vector<Instance *> _pflipflops;
  unordered_map<string, Instance *> _name2pInstances_ff;
  unordered_map<string, Instance *> _name2pInstances_gate;
  BucketList _bucket;
  // Nets
  unsigned _numNets;
  vector<Net *> _pNets;
  // Max placement utilization ratio
  double _binWidth;
  double _binHeight;
  double _binMaxUtil;
  // Placement rows
  vector<PlacementRow *> _pPlacementRows;
  // Displacement delay
  double _displacementDelay;
  CellLibrary *best1bitff;
  // vector<DisSet *> _disjointSets;
  vector<CellLibrary *> _2bitffCellLibrary;
  void findFeasable_reg(Net *net, fstream &outfile, int net_count = 0);
  void print_weight_matrix(vector<vector<double> *> *weight_matrix);
  void printCliqueGraph(Graph *clique_graph);
  void recordMergedInstance(Instance* instance,
                                         const std::vector<std::pair<std::string, std::string>>& pinMap)
{
    _mergedInstances.push_back(instance);
    for (const auto& [src, dst] : pinMap)
    {
        _pinMappings.emplace_back(src + " map " + dst);
    }
}

  void parseInput(fstream &inFile)
  {
    if (!inFile.is_open())
    {
      cout << "Failed to open the file." << endl;
      return;
    }
    string line;
    while (getline(inFile, line))
    {
      stringstream ss(line);
      string item;
      vector<string> input_string;
      while (getline(ss, item, ' '))
      {
        item.erase(remove(item.begin(), item.end(), '\n'), item.end()); // Remove newline character from 'item'
        item.erase(remove(item.begin(), item.end(), '\r'), item.end());
        input_string.push_back(item);
      }
      if (input_string[0] == "Alpha")
      {
        _alpha = stod(input_string[1]);
        // cout << "input alpha" << endl;
      }
      else if (input_string[0] == "Beta")
      {
        _beta = stod(input_string[1]);
        // cout << "input beta" << endl;
      }
      else if (input_string[0] == "Gamma")
      {
        _gamma = stod(input_string[1]);
        // cout << "input Gamma" << endl;
      }
      else if (input_string[0] == "Lambda") // Not used in 2025 CAD B
      {
        _lambda = stod(input_string[1]);
        // cout << "input lambda" << endl;
      }
      else if (input_string[0] == "DieSize")
      {
        _dieLeftBottom = Point2<double>(stod(input_string[1]), stod(input_string[2]));
        _dieRightTop = Point2<double>(stod(input_string[3]), stod(input_string[4]));
      }
      else if (input_string[0] == "NumInput")
      {
        _numInputPins = stoul(input_string[1]);
      }
      else if (input_string[0] == "Input")
      {
        _name2pInputPins[input_string[1]] = new InstancePin(input_string[0], input_string[1], stod(input_string[2]), stod(input_string[3]));
        // cout << "read Input " << _name2pInputPins[input_string[1]]->name() << endl;
      }
      else if (input_string[0] == "NumOutput")
      {
        _numOutputPins = stoul(input_string[1]);
      }
      else if (input_string[0] == "Output")
      {

        _name2pOutputPins[input_string[1]] = new InstancePin(input_string[0], input_string[1], stod(input_string[2]), stod(input_string[3]));
        // cout << "read Output" << endl;
      }

      else if (input_string[0] == "FlipFlop")
      {
        _name2pFFLibrary[input_string[2]] = new CellLibrary(input_string[0], input_string[2], stoul(input_string[1]), stod(input_string[3]), stod(input_string[4]), stoul(input_string[5]));
        string index1;
        for (int i = 0; i < stoi(input_string[5]); i++)
        { // loop to read pin and add to current library
          getline(inFile, line);
          stringstream s1(line);
          vector<string> pin_string1;
          while (getline(s1, index1, ' '))
          {
            index1.erase(remove(index1.begin(), index1.end(), '\n'), index1.end()); // Remove newline character from 'index'
            index1.erase(remove(index1.begin(), index1.end(), '\r'), index1.end());
            pin_string1.push_back(index1);
          }

          _name2pFFLibrary[input_string[2]]->addPin(new CellLibraryPin(pin_string1[1], stod(pin_string1[2]), stod(pin_string1[3]), i));
        }
        if (input_string[1] == "2")
        {
          store_bit2_ff(_name2pFFLibrary[input_string[2]]);
        }
      }
      else if (input_string[0] == "Gate")
      {
        _name2pGateLibrary[input_string[1]] = new CellLibrary(input_string[0], input_string[1], 0, stod(input_string[2]), stod(input_string[3]), stoul(input_string[4]));
        // cout << _name2pGateLibrary[input_string[1]]->name() << endl;
        string index2;
        for (int i = 0; i < stoi(input_string[4]); i++)
        { // loop to read pin and add to current library
          getline(inFile, line);
          stringstream s2(line);
          vector<string> pin_string2;
          while (getline(s2, index2, ' '))
          {
            index2.erase(remove(index2.begin(), index2.end(), '\n'), index2.end()); // Remove newline character from 'index'
            index2.erase(remove(index2.begin(), index2.end(), '\r'), index2.end());
            pin_string2.push_back(index2);
          }
          _name2pGateLibrary[input_string[1]]->addPin(new CellLibraryPin(pin_string2[1], stod(pin_string2[2]), stod(pin_string2[3]), i));
        }
      }
      else if (input_string[0] == "NumInstances")
      {
        _numInstances = stoul(input_string[1]);
      }
      else if (input_string[0] == "Inst")
      {
        if (_name2pGateLibrary.find(input_string[2]) != _name2pGateLibrary.end())
        {
          _name2pInstances_gate[input_string[1]] = new Instance(input_string[1], _name2pGateLibrary[input_string[2]], stod(input_string[3]), stod(input_string[4]), _name2pGateLibrary[input_string[2]]->numPins());

          for (unsigned int i = 0; i < _name2pGateLibrary[input_string[2]]->numPins(); i++)
          {
            InstancePin *p = new InstancePin("", _name2pGateLibrary[input_string[2]]->pPin(i)->name(), _name2pInstances_gate[input_string[1]]->x() + _name2pGateLibrary[input_string[2]]->pPin(i)->x(), _name2pInstances_gate[input_string[1]]->y() + _name2pGateLibrary[input_string[2]]->pPin(i)->y(), i);
            p->setInstance(_name2pInstances_gate[input_string[1]]);
            _name2pInstances_gate[input_string[1]]->addPin(p);
            // _name2pInstances_gate[input_string[1]]->addPin(new InstancePin("", _name2pGateLibrary[input_string[2]]->pPin(i)->name(), _name2pInstances_gate[input_string[1]]->x() + _name2pGateLibrary[input_string[2]]->pPin(i)->x(), _name2pInstances_gate[input_string[1]]->y() + _name2pGateLibrary[input_string[2]]->pPin(i)->y(), i));
          }
        }
        else if (_name2pFFLibrary.find(input_string[2]) != _name2pFFLibrary.end())
        {
          _name2pInstances_ff[input_string[1]] = new Instance(input_string[1], _name2pFFLibrary[input_string[2]], stod(input_string[3]), stod(input_string[4]), _name2pFFLibrary[input_string[2]]->numPins());

          for (unsigned int i = 0; i < _name2pFFLibrary[input_string[2]]->numPins(); i++)
          {
            InstancePin *p = new InstancePin("", _name2pFFLibrary[input_string[2]]->pPin(i)->name(), _name2pInstances_ff[input_string[1]]->x() + _name2pFFLibrary[input_string[2]]->pPin(i)->x(), _name2pInstances_ff[input_string[1]]->y() + _name2pFFLibrary[input_string[2]]->pPin(i)->y(), i);
            p->setInstance(_name2pInstances_ff[input_string[1]]);
            // cout << p->getinstance()->name() << endl;
            _name2pInstances_ff[input_string[1]]->addPin(p);
            // _name2pInstances_ff[input_string[1]]->addPin(new InstancePin("", _name2pFFLibrary[input_string[2]]->pPin(i)->name(), _name2pInstances_ff[input_string[1]]->x() + _name2pFFLibrary[input_string[2]]->pPin(i)->x(), _name2pInstances_ff[input_string[1]]->y() + _name2pFFLibrary[input_string[2]]->pPin(i)->y(), i));
          }
        }
        else
        {
          cout << "bbb " << "Library not found " << input_string[2] << " bbb" << endl;
        }
      }
      else if (input_string[0] == "NumNets")
      {
        _numNets = stoul(input_string[1]);
        // cout << "read NumNets" << endl;
      }
      else if (input_string[0] == "Net")
      {
        Net *net = new Net(input_string[1], 0, stoul(input_string[2]));
        string index;
        for (unsigned int i = 0; i < stoul(input_string[2]); i++)
        { // loop to read pin and add to current library
          getline(inFile, line);
          stringstream ss(line);
          getline(ss, index, ' ');
          vector<string> pin_string;
          while (getline(ss, index, '/'))
          {
            index.erase(remove(index.begin(), index.end(), '\n'), index.end()); // Remove newline character from 'index'
            index.erase(remove(index.begin(), index.end(), '\r'), index.end()); // Remove carriage return characters
            pin_string.push_back(index);                                        // Adds the string 'index' to the end of 'pin_string'
          }
          // cout << pin_string[0] << " " << pin_string[1] << endl;
          // cout << pin_string[0];
          // cout << pin_string[0];

          // cout << pin_string[0] << endl;
          // for (int i = 0; i < pin_string.size(); i++)
          // {
          //   cout << pin_string[i] << " ";
          // }
          // cout << endl;
          // cout << pin_string[0] << endl;
          // for (const auto &[key, value] : _name2pOutputPins)
          // {
          //   cout << key << " has name " << value->name() << endl;
          // }
          // cout << pin_string[0] << endl;
          // cout << _name2pInputPins[pin_string[0]] << endl;
          // cout << _name2pInputPins["CLK"] << endl;
          // cout << "aa" << pin_string[0] << "aa" << endl;
          // // cout << _name2pInputPins["in"] << endl;
          // cout << pin_string.size() << " find CLK size" << endl;
          // cout << (_name2pInputPins.find(pin_string[0]) != _name2pInputPins.end()) << endl;
          // cout << ((pin_string.size() == 1) & (_name2pInputPins.find(pin_string[0]) != _name2pInputPins.end()));
          if (pin_string.size() == 1 && _name2pInputPins.find(pin_string[0]) != _name2pInputPins.end())
          {
            // cout << _name2pInputPins[pin_string[0]]->x() << endl;
            // cout << _name2pInputPins[pin_string[0]].name() << endl;
            net->addPin("DIE", _name2pInputPins[pin_string[0]]);
            _name2pInputPins[pin_string[0]]->setNet(net);
            // cout << net->pin(0).pin->name() << endl;
            // _name2pInputPins[pin_string[0]]->setNet(net);
          }
          else if (pin_string.size() == 1 && _name2pOutputPins.find(pin_string[0]) != _name2pOutputPins.end())
          {
            // cout << pin_string[0] << endl;
            net->addPin("DIE", _name2pOutputPins[pin_string[0]]);
            _name2pOutputPins[pin_string[0]]->setNet(net);
            // _name2pOutputPins[pin_string[0]]->setNet(net);
            // cout << net->pin(0).pin->name() << endl;
          }
          else
          {
            if (_name2pInstances_ff.find(pin_string[0]) != _name2pInstances_ff.end())
            {
              net->addPin(_name2pInstances_ff[pin_string[0]]->name(), (_name2pInstances_ff[pin_string[0]]->pPin(pin_string[1])));
              // _name2pInstances_ff[pin_string[0]]->pPin(pin_string[1])->setNet(net);
              // cout << ((net->pin(0)->pin->name())) << endl;
              _name2pInstances_ff[pin_string[0]]->pPin(pin_string[1])->setNet(net);
            }
            else if (_name2pInstances_gate.find(pin_string[0]) != _name2pInstances_gate.end())
            {
              net->addPin(_name2pInstances_gate[pin_string[0]]->name(), (_name2pInstances_gate[pin_string[0]]->pPin(pin_string[1])));
              // _name2pInstances_gate[pin_string[0]]->pPin(pin_string[1])->setNet(net);
              // cout << ((net->pin(0)->pin->name())) << endl;
              _name2pInstances_gate[pin_string[0]]->pPin(pin_string[1])->setNet(net);
            }

            else
            {
              cout << "Instance not found " << pin_string[0] << endl;
            }
          }
        }
        // cout << net->pin(0)->pin->name() << endl; // net -> pinpair vector -> pinpair -> pin(in pinpiar)->pin_name
        _pNets.push_back(net);
        // cout << "read Nets" << endl;
      }
      else if (input_string[0] == "BinWidth")
      {
        _binWidth = stod(input_string[1]);
      }
      else if (input_string[0] == "BinHeight")
      {
        _binHeight = stod(input_string[1]);
      }
      else if (input_string[0] == "BinMaxUtil")
      {
        _binMaxUtil = stod(input_string[1]);
      }
      else if (input_string[0] == "PlacementRows")
      {
        PlacementRow *placementRow = new PlacementRow(stod(input_string[1]), stod(input_string[2]), stod(input_string[3]), stod(input_string[4]), stoul(input_string[5]));
        _pPlacementRows.push_back(placementRow);
        // cout << "read Placement Rows" << endl;
      }
      else if (input_string[0] == "DisplacementDelay")
      {
        _displacementDelay = stod(input_string[1]);
      }
      else if (input_string[0] == "QpinDelay")
      {
        _name2pFFLibrary[input_string[1]]->setQPinDelay(stod(input_string[2]));
      }
      else if (input_string[0] == "TimingSlack")
      {
        if (_name2pInstances_ff.find(input_string[1]) != _name2pInstances_ff.end())
        {
          _name2pInstances_ff[input_string[1]]->setSlack(input_string[2], stod(input_string[3]));
          Net *net = _name2pInstances_ff[input_string[1]]->pPin(input_string[2])->pNet();
          for (const auto &pin : net->pPins())
          {
            if (pin.pin->name() == input_string[2])
            {
              pin.pin->setTimingSlack(stod(input_string[3]));
            }
          }
        }
        // cout << "read TimingSlack" << endl;
      }
      else if (input_string[0] == "GatePower")
      {
        if (_name2pGateLibrary.find(input_string[1]) != _name2pGateLibrary.end())
        {
          _name2pGateLibrary[input_string[1]]->setGatePower(stod(input_string[2]));
        }
        else if (_name2pFFLibrary.find(input_string[1]) != _name2pFFLibrary.end())
        {
          _name2pFFLibrary[input_string[1]]->setGatePower(stod(input_string[2]));
        }
        // cout << "read GatePower" << endl;
      }
      else
      {
        cout << "Invalid input" << input_string[0] << endl;
      }
      // cout << _pPlacementRows[0];

      // _placementRowRightTop = Point2<double>(_pPlacementRows.back()->x() + _pPlacementRows.back()->width() * _pPlacementRows.back()->numSites(), _pPlacementRows.back()->y() + _pPlacementRows.back()->height());
    }
    // cout << _pPlacementRows[0];
    if (!_pPlacementRows.empty())
    {
      _placementRowRightTop = Point2<double>(_pPlacementRows.back()->x() + _pPlacementRows.back()->width() * _pPlacementRows.back()->numSites(), _pPlacementRows.back()->y() + _pPlacementRows.back()->height());
      _placementRowLeftBottom = Point2<double>(_pPlacementRows[0]->x(), _pPlacementRows[0]->y());
    }
    // cout << _placementRowLeftBottom.x << " " << _placementRowLeftBottom.y << endl;
    // cout << _placementRowRightTop.x << " " << _placementRowRightTop.y << endl;
    cout << "Parsing input done" << endl;
    set_best1bitff();

    // cout << "123" << endl;
    // cout << _pNets[0]->pin(0)->name() << endl;
    // cout << "123" << endl;
  }
  //       //"FLIPFLOP 1 FF1 5 10 2"

  //       // Process0.3656366666 each line as needed
  //       // cout << line << endl; // For demonstration, print each line

  //   }
  //   }
};

#endif // MBFFOPTIMIZER_H