#ifndef CELL_LIBRARY_H
#define CELL_LIBRARY_H

#include <string>
#include <vector>
#include <unordered_map>
#include "Pin.h"
using namespace std;

class CellLibrary
{
public:
  CellLibrary(string type = "", string name = "", unsigned numBits = -1, double width = -1, double height = -1, unsigned numPins = -1, double qPinDelay = -1, double gatePower = -1)
      : _type(type), _name(name),  _numBits(numBits), _width(width), _height(height), _numPins(numPins), _qPinDelay(qPinDelay), _gatePower(gatePower)
  {
  }

  /////////////////////////////////////////////
  // get
  /////////////////////////////////////////////
  string name() { return _name; }
  string type() { return _type; }
  unsigned numBits() { return _numBits; }
  double width() { return _width; }
  double height() { return _height; }
  unsigned numPins() { return _numPins; }
  CellLibraryPin *pPin(string pinName) { return _name2pPins[pinName]; }
  CellLibraryPin *pPin(unsigned pinId) { return _pPins[pinId]; }
  double qPinDelay() { return _qPinDelay; }
  double gatePower() { return _gatePower; }
  unordered_map<string, CellLibraryPin *> name2pPins() { return _name2pPins; }

  /////////////////////////////////////////////
  // set
  /////////////////////////////////////////////
  void setNumPins(unsigned numPins)
  {
    _numPins = numPins;
    _pPins.resize(numPins);
  }
  void addPin(CellLibraryPin *pPin)
  {
    _name2pPins[pPin->name()] = pPin;
    _pPins.push_back(pPin);
  }
  void setQPinDelay(double qPinDelay) { _qPinDelay = qPinDelay; }
  void setGatePower(double gatePower) { _gatePower = gatePower; }

private:

  string _type; // cell type ff or comb
  string _name; // cell name
  unsigned _numBits;
  double _width;
  double _height;
  unsigned _numPins;
  unordered_map<string, CellLibraryPin *> _name2pPins; // key: pin name, value: cell library pin
  vector<CellLibraryPin *> _pPins;
  double _qPinDelay;
  double _gatePower;
};

#endif // CELL_LIBRARY_H