#ifndef HEX_H
#define HEX_H

#include <string>
#include <vector>
#include <utility>

using namespace std;

class Hex
{
public:
  int x;
  int y;
  int terrain;
  string country;
  bool visible;
  vector<int> visited;
  int feature;
  string description;
  vector <pair<int,string>> history;
  Hex(int xcor, int ycor, int terr) {
    x = xcor;
    y = ycor;
    terrain = terr;
    country = "N/A";
    visible = false;
    feature = 0;
    description = "";
  }
};

#endif
