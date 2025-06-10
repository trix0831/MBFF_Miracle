#ifndef PLACEMENTROW_H
#define PLACEMENTROW_H
#include <string>
#include <vector>
#include <unordered_map>
#include "Pin.h"
using namespace std;

class PlacementRow
{
public:
  PlacementRow(double x = -1, double y = -1, double width = -1, double height = -1, unsigned numSites = 0) : _x(x), _y(y), _width(width), _height(height), _numSites(numSites)
  {
    setPlacement();
  }

  /////////////////////////////////////////////
  // get (for pins of this net)
  /////////////////////////////////////////////
  double x() { return _x; }
  double y() { return _y; }
  double width() { return _width; }
  double height() { return _height; }
  unsigned numSites() { return _numSites; }
  vector<bool> placement() { return _placement; }
  bool isPlaced(unsigned putSites)
  {
    return _placement[putSites];
  }
  double weight(unsigned putSites)
  {
    return _weight[putSites];
  }
  /////////////////////////////////////////////
  // set (for pins of this net)
  /////////////////////////////////////////////
  void setX(double x) { _x = x; }
  void setY(double y) { _y = y; }
  void setWidth(double width) { _width = width; }
  void setHeight(double height) { _height = height; }
  void setNumSites(unsigned numSites) { _numSites = numSites; }
  void setPlacement()
  {
    _placement = vector<bool>(_numSites, false);
    _weight = vector<double>(_numSites, 0);
  }
  void Place(unsigned putSites)
  {
    _placement[putSites] = true;
  }
  void resetPlace(unsigned putSites)
  {
    _placement[putSites] = false;
  }
  void setWeight(unsigned putSites, double weight)
  {
    _weight[putSites] = weight;
  }
  void initoccupied()
  {
    _occupied = vector<bool>(_numSites, false);
  }
  void setoccupied(unsigned putSites , bool occupied)
  {
    _occupied[putSites] = occupied;
  }
  bool isoccupied(unsigned putSites)
  {
    return _occupied[putSites];
  }

  /*
  1  0 0-9
  2  0 10-19
  3  0 20-29
  4  0 30-
  5  0 40
  */
private:
  double _x;
  double _y;
  double _width;
  double _height;
  unsigned _numSites;
  vector<bool> _placement;
  vector<bool> _occupied;
  vector<double> _weight;
};
#endif