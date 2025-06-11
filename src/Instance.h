#ifndef INSTANCE_H
#define INSTANCE_H

#include <string>
#include <vector>
#include <unordered_map>
#include "CellLibrary.h"
#include "Pin.h"
using namespace std;

class Instance
{
public:
  Instance(string name = "", CellLibrary *pCellLibrary = nullptr, double x = -1, double y = -1, unsigned numPins = -1)
      : _name(name), _pCellLibrary(pCellLibrary), _fixed(false), _x(x), _y(y), _numPins(numPins)
  {
    for (unsigned i = 0; i < numPins; i++)
    {
      _TimingSlack[_pCellLibrary->pPin(i)->name()] = 0;
    }
  }

  /////////////////////////////////////////////
  // get
  /////////////////////////////////////////////
  string name() { return _name; }
  CellLibrary *pCellLibrary() { return _pCellLibrary; }
  bool fixed() { return _fixed; }
  double x() { return _x; }
  double y() { return _y; }
  unsigned numPins() { return _numPins; }
  InstancePin *pPin(string pinName) { return _name2pPins[pinName]; }
  unordered_map<string, InstancePin *> name2pPins() { return _name2pPins; }
  double TimingSlack(string pinName) { return _TimingSlack[pinName]; }
  unordered_map<string, double> TimingSlack() { return _TimingSlack; }
  bool merged = 0;
  // Net *pinNet(InstancePin *pPin) { return _pinNet[pPin]; } // check different pin nets
  /////////////////////////////////////////////
  // set
  /////////////////////////////////////////////
  void setName(string name) { _name = name; }
  void setCellLibrary(CellLibrary *pCellLibrary) { _pCellLibrary = pCellLibrary; }
  void setFixed(bool fixed) { _fixed = fixed; }
  void setX(double x) { _x = x; }
  void setY(double y) { _y = y; }
  // void setPinNet(InstancePin *pPin, Net *pNet) { _pinNet[pPin] = pNet; }
  void setNumPins(unsigned numPins)
  {
    _numPins = numPins;
    _pPins.resize(numPins);
  }
  void addPin(InstancePin *pPin)
  {
    pPin->setInstance(this);
    _name2pPins[pPin->name()] = pPin;
    _pPins.push_back(pPin);
  }
  void setSlack(string pinname, double Slack)
  {
    _TimingSlack[pinname] = Slack;
  }

private:
  string _name;
  CellLibrary *_pCellLibrary;
  bool _fixed;
  double _x;
  double _y;
  unsigned _numPins;
  unordered_map<string, InstancePin *> _name2pPins; // key: pin name, value: instance pin
  vector<InstancePin *> _pPins;
  unordered_map<string, double> _TimingSlack;
  // unordered_map<InstancePin *, Net *> _pinNet;
};

#endif // INSTANCE_H