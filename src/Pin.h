#ifndef PIN_H
#define PIN_H

#include <string>
// #include "Net.h"
// #include "Instance.h"
using namespace std;
class Instance;
class Net;
class BasePin
{
public:
  BasePin(string type, string name = "", double x = -1, double y = -1)
      : _type(type), _name(name), _x(x), _y(y)
  {
  }

  /////////////////////////////////////////////
  // get
  ////////////////////////////////////////////
  string type() { return _type; }
  string name() { return _name; }
  double x() { return _x; }
  double y() { return _y; }

  /////////////////////////////////////////////
  // set
  /////////////////////////////////////////////
  void setPosition(double x, double y)
  {
    _x = x;
    _y = y;
  }

private:
  string _type; // input, output, flipflop, gate...
  string _name;
  double _x, _y; // relative position for cell library pin, absolute position for instance pin
};

class CellLibraryPin : public BasePin
{
public:
  CellLibraryPin(string name = "", double x = -1, double y = -1, unsigned cellLibraryId = -1)
      : BasePin("cellLibrary", name, x, y), _cellLibraryId(cellLibraryId)
  {
  }

  /////////////////////////////////////////////
  // get
  /////////////////////////////////////////////
  unsigned cellLibraryId() { return _cellLibraryId; }

  /////////////////////////////////////////////
  // set
  /////////////////////////////////////////////
  void setCellLibraryId(unsigned cellLibraryId) { _cellLibraryId = cellLibraryId; }

private:
  unsigned _cellLibraryId;
};

class InstancePin : public BasePin
{
public:
  InstancePin(string type = "", string name = "", double x = -1, double y = -1, int instanceId = -1, double timingSlack = 0)
      : BasePin(type, name, x, y), _instanceId(instanceId), _timingSlack(timingSlack)
  {
  }
  void setInstance(Instance *instance) { _instance = instance; };
  /////////////////////////////////////////////
  // get
  /////////////////////////////////////////////
  int instanceId() { return _instanceId; }
  Instance *getinstance() { return _instance; };
  double timingSlack() { return _timingSlack; }
  Net *pNet() { return _pNet; }
  /////////////////////////////////////////////
  // set
  /////////////////////////////////////////////
  void setInstanceId(int instanceId) { _instanceId = instanceId; }
  void setTimingSlack(double timingSlack) { _timingSlack = timingSlack; }
  void setNet(Net *pNet) { _pNet = pNet; }

private:
  int _instanceId; // input and output = -1, flipflop and gate = instance id
  double _timingSlack;
  Instance *_instance = nullptr;
  Net *_pNet;
};

#endif // PIN_H