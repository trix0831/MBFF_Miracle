#ifndef NET_H
#define NET_H

#include <vector>
#include <string>
#include "Pin.h"
#include "Instance.h"
using namespace std;

class Net
{
public:
  struct pin_pair
  {
    string inst_name;
    InstancePin *pin;
  };
  Net(string name = "", unsigned netId = -1, unsigned numPins = 0) : _name(name), _netId(netId), _numPins(numPins)
  {
  }

  /////////////////////////////////////////////
  // get (for pins of this net)
  /////////////////////////////////////////////
  string name() { return _name; }
  unsigned netId() { return _netId; }
  unsigned numPins() { return _numPins; }
  pin_pair pin(unsigned index) { return _pPins[index]; } // index: 0 ~ (numPins-1), not Pin id
  vector<pin_pair> pPins() { return _pPins; }

  /////////////////////////////////////////////
  // set (for pins of this net)
  /////////////////////////////////////////////
  void setNumPins(unsigned numPins)
  {
    _numPins = numPins;
  }
  void addPin(string instance_name, InstancePin *pPin)
  {
    pin_pair p;
    p.inst_name = instance_name;
    p.pin = pPin;
    _pPins.push_back(p);
  }
  void clearPins() { _pPins.clear(); }

private:
  string _name;
  unsigned _netId;
  unsigned _numPins;
  vector<pin_pair> _pPins; // The first always be the input pin
  vector<string> _pinNames;
};

#endif // NET_H