#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <vector>
#include <utility>
#include <algorithm>
#include <math.h>
#include "Hex.h"

using namespace std;

//Removes ZeroWidth characters that appear for unknown reasons
void removeZeroWidth(string & line) {
  char c = '\0';
  string nul;
  nul = c;
  for (int i = 0;i < line.length();i++) {
    if (line.substr(i,1) == nul)
    line.erase(i,1);
  }
}

/*This functions makes it so that quotes from the input files can be preserved
in a way that won't disrupt the format of the final JSON*/
void fixQuotes(string & line) {
  for (int i=0;i<line.length();i++) {
    if (line.substr(i,1) == "\"") {
      line.insert(i,"\\");
      i++;
    }
  }
}

/*The original input file uses a tag-like notation to separate the notes into
different categories of information. The Info tag is used to provide description
of a place. This is an example of what an Info tag could look like
[Info]This is the burial site of a young bronze dragon[/Info] */
string getInfo(string line) {
  /*The below section finds the starting tag for Info, and returns an empty string
  if there is no such tag in the line*/
  int start = line.find("[Info]");
  if (start == string::npos)
    return "";
  start+=6; //The offset needed for the length of the starting Info tag [Info]
  /*The below section finds the closing Info tag and returns the content inbetween
  the starting and closing tags*/
  int end = line.find("[/Info]");
  string info = line.substr(start,end-start);
  fixQuotes(info);
  return info;
}

/* The original input file uses a tag-like notation to separate the notes into
different categories of information. The Feat tag is short for feature, and a
Hex can contain one of six different types of features
In a later part of the code, the feature is converted into a number from 0-6 */
string getFeat(string line) {
  /*The below section finds the starting tag for Feat, and returns an empty string
  if there is no such tag in the line*/
  int start = line.find("[Feat]");
  if (start == string::npos)
    return "";

  start+=6; //The offset needed for the length of the starting Feat tag [Feat]
  /*The above section finds the closing Feat tag and returns the content inbetween
  the starting and closing tags*/
  int end = line.find("[/Feat]");
  string feat = line.substr(start,end-start);
  return feat
}

/*The original input file uses a tag-like notation to separate the notes into
different categories of information. The Hist tag is used to describe what the
party did on what day. This is an example of what a Hist tag could look like
[Hist 10]On their 10th day of travel, the party finally reached the city[/Hist]*/
 vector <pair <int,string>> getHist(string line) {
   /*The below section finds the starting tag for Hist, and returns an empty string
   if there is no such tag in the line*/
   vector <pair <int,string>> all_hist;
  int start = line.find("[Hist "); //+6
  if (start == string::npos) {
    return all_hist;
  }
  /*a while loop is used here since there can be multiple Hist tags at a given Hex
  if the party was there for multiple days. */
  while (start != string::npos) {
  start+=6; //The offset needed for part of the starting Hist tag [Hist
   //The code below is used to figure out what day the events happened on
  int end1 = line.find("]",start);
  int day = stoi(line.substr(start,end1-start));
  /*This part of the code gets a description of what happened on that day inbetween
  the starting and closing tag. */
  start = end1+1;
  int end2 = line.find("[/Hist]",start);
  string desc = line.substr(start,end2-start);
  fixQuotes(desc);
  /*The day and description are saved in a pair and then gets ready to check for
  another Hist tag in the same line. */
  pair <int,string> hist = make_pair(day,desc);
  all_hist.push_back(hist);
  line = line.substr(end2+7,line.length()-(end2+7));
  start = line.find("[Hist ");
}
  return all_hist;
}

/*This function is used to help build a vector of vectors of coordinates that represent
the path the party has traveled so far. day represents what day it was while time
represents if it was the 1st, 2nd, 3rd, etc. Hex they traveled to that day */
void insertAt(vector <vector <pair <int,int>>> & steps, int day, int time, int hex_x, int hex_y) {
/*Continually resizes the vector to make sure it's length is the same as the number
of days that the party has traveled, so that each entry in the vector represents
one day of travel */
  if (steps.size() < day)
    steps.resize(day);
/*Continually resizes the vector of a particular day to make sure it's length is
the same as the number of hexes the party traveled that day, so that each entry
in the vector represents a hex they were on */
  if (steps[day-1].size() < time)
    steps[day-1].resize(time);
  pair <int,int> cords = make_pair(hex_x,hex_y);
  steps[day-1][time-1] = cords;
}

/*The original input file uses a tag-like notation to separate the notes into
different categories of information. The Step tag is used to note both what day
the party visited a particular Hex and if it was the 1st, 2nd, 3rd, etc. Hex they
traveled that day. If the party visited a Hex on Day 5 and it was the 4th Hex
they visited that day, the tag would look like [Step 5 4] */
 void getSteps(string line, vector <vector <pair< int,int>>> & steps, int x, int y) {
   /*The below section finds the starting tag for Step, and returns an empty string
   if there is no such tag in the line */
 int start = line.find("[Step ");
 if (start == string::npos) {
   return;
 }
 /*a while loop is used here since there can be multiple Step tags at a given Hex
 if the party was there for multiple days or visited it multiple times in one day */
 while (start != string::npos) {
 start+=6; //The offset needed for part of the starting Hist tag [Hist
  //The code below is used to figure out what day the Hex was visited
 int end1 = line.find(" ",start);
 int day = stoi(line.substr(start,end1-start));
 //The code below is used to figure out when in the day the Hex was visited
 start = end1+1;
 int end2 = line.find("]",start);
 int time = stoi(line.substr(start,end2-start));
 /*The insertAt function is called to help build a path that shows how the party
 moved during their travels. Then, the code gets ready to look for any other
 Step tags in this line of the original input. */
 insertAt(steps,day,time,x,y);
 line = line.substr(end2+1,line.length()-(end2+1));
 start = line.find("[Step ");
}
 return;
}

/*In the original input file, notes are saved in a different part of the file than
where the map information is. Each note mentions the x and y value it's located in
as part of the note, so this function gets those coordinates and converts them into
numbers between 0 and 299 to match the width and height (in hexes) of the map.
Notes are always placed approximately near the center of a Hex */
pair <int,int> getCords(string line) {
  /*Checks for the code used to mark x coordinates. If not in the line, returns
  invalid coordinates instead to know to dismiss them. */
  int x_start = line.find("x=\"");
  if (x_start == string::npos) {
    pair <int,int> na = make_pair(-1,-1);
    return na;
  }
  /*Gets the x value of the note and uses an formula I noticed to determine its
  Hex coordinates based on its x value. */
  x_start+=3;
  int x_end = line.find('"',x_start);
  double x = stod(line.substr(x_start,x_end-x_start));
  x = round((x-150)/225.0);
  //Gets the y value of the note
  int y_start = line.find("y=\"",x_start); //+6
  y_start+=3;
  int y_end = line.find('"',y_start);
  double y = stod(line.substr(y_start,y_end-y_start));
/*Because Hex maps actually alternate, different formulas are needed to determine the
y Hex coordinate based on whether the x Hex coordinate was even or odd.*/
  if ((((int) x) % 2) == 0)
    y = round((y-150)/300.0);
  else
    y = round((y-300)/300.0);
  pair <int,int> cords = make_pair((int) x,(int) y);
  return cords;
}

//This function gets what type of terrain a Hex has(represented by a number)
int terrainIdentifier(string line) {
  int pos = line.find('\t');
  int terrain = (stoi(line.substr(0,pos)))+1; //This gets the number of the terrain
  pos++;
  pos = line.find('\t',pos); //1
  pos++;
  pos = line.find('\t',pos); //2
  pos++;
  /*This checks if the terrain has the "frozen" tag. If so, the terrain number is
  turned into a negative to represent that it is a frozen version of that terrain */
  if (line.substr(pos,1) == "1")
    terrain = terrain *-1;
  return terrain;
}

//This function converts a feature type into a number
int numericFeat(string feat) {
  if (feat == "Settlement")
    return 1;
  else if (feat == "Fortress")
    return 2;
  else if (feat == "Religious Order")
    return 3;
  else if (feat == "Ruins")
    return 4;
  else if (feat == "Monster Lair")
    return 5;
  else if (feat == "Natural Phenomenon")
    return 6;
  else
    return 0;
}

/*This function checks if a point q is on line segment pr. first is the x and
second is the y. This code was slightly altered from
https://www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/ */
int onSegment(pair <int,int> p, pair <int,int> q, pair <int,int> r) {
  if (q.first <= max(p.first, r.first) && q.first >= min(p.first, r.first) &&
        q.second <= max(p.second, r.second) && q.second >= min(p.second, r.second))
       return true;
  return false;
}

/*This function checks if points p, q, and r are clockwise, counter-clockwise, or colinear.
first is the x and second is the y. This code was slightly altered from
https://www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/ */
int orientation(pair <int,int> p, pair <int,int> q, pair <int,int> r) {
  int val = ((q.second - p.second)*(r.first-q.first)) - ((q.first-p.first)*(r.second-q.second));
  if (val == 0)
    return 0;
  else if (val > 0)
    return 1;
  else
    return 2;
}

/*This function uses the orientation to determine whether two lines segments intersect
This code was slightly altered from
https://www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/ */
bool intersection(pair <int,int> p1, pair <int,int> q1, pair <int,int> p2, pair <int,int> q2) {
//This represents the general cases for checking for an intersection
  int o1 = orientation(p1,q1,p2);
  int o2 = orientation(p1,q1,q2);
  int o3 = orientation(p2,q2,p1);
  int o4 = orientation(p2,q2,q1);
  if ((o1 != o2) && (o3 != o4))
    return true;

//These represent special cases for checking for an intersection
  if (o1 == 0 && onSegment(p1, p2, q1))
    return true;
  if (o2 == 0 && onSegment(p1, q2, q1))
    return true;
  if (o3 == 0 && onSegment(p2, p1, q2))
    return true;
  if (o4 == 0 && onSegment(p2, q1, q2))
    return true;

  return false;
}

/*This function is used to determine whether a point lies within a polygon or not by
checking how many times a line intersects the polygon using another point that
lies outside of the polygon. The polygon vector is written in a way such that
every pair of indexes represent 1 line, i.e. polygon[0],polygon[1] is one line
and polygon[2],polygon[3] is another line, even though polygon[1] = polygon[2]
since the ending point for one line is the starting point for another. This function
is inspired by and altered from
https://www.geeksforgeeks.org/how-to-check-if-a-given-point-lies-inside-a-polygon/ */
bool isInside(pair <int,int> p1, pair <int,int> q1, vector <pair<int,int>> polygon) {
  int count = 0;
  bool inside = false;
  //This checks for an intersection between p1q1 and each line segment of the polygon
  for (int i = 0; i < polygon.size(); i+=2) {
    if (intersection(p1,q1,polygon[i],polygon[i+1])) {
      //Special check to see if the point lies on a line segment of the polygon
      if (orientation(polygon[i], p1, polygon[i+1]) == 0)
          return onSegment(polygon[i], p1, polygon[i+1]);
      count++;
    }
  }
  //The point is in the polygon if there is an odd number of intersections
  if (count % 2 == 1)
    return true;
  else
    return false;
}

/*This function converts a Hex into a JSON object that can be used as part of a
larger JSON that represents the entire map*/
string hexToJSON(Hex tile) {
  string JSON = "{\n";
  JSON+="\t \"x\":";
  JSON+=to_string(tile.x);
  JSON+=",\n";

  JSON+="\t \"y\":";
  JSON+=to_string(tile.y);
  JSON+=",\n";

  JSON+="\t \"terrain\":";
  //if tile is invsible and terrain is NOT water, use 0 as a mystery terrain value
  if (!tile.visible && (tile.terrain != 1 && tile.terrain != -1))
    JSON+="0";
  else
    JSON+=to_string(tile.terrain);
  JSON+=",\n";

  JSON+="\t \"country\":\"";
  JSON+=tile.country;
  JSON+="\",\n";

  JSON+="\t \"visible\":";
  JSON+=to_string(tile.visible); //bool to string
  JSON+=",\n";

  JSON+="\t \"visited\":[ ";
  for(int i = 0;i<tile.visited.size();i++) {
    JSON+=to_string(tile.visited[i]);
    if (i+1<tile.visited.size())
      JSON+=", ";
  }
  JSON+=" ],\n";

  JSON+="\t \"feature\":";
  JSON+=to_string(tile.feature);
  JSON+=",\n";

  if (tile.description == "")
    JSON+="\t \"info\":null,\n";
  else {
  JSON+="\t \"info\":\"";
  JSON+=tile.description;
  JSON+="\",\n";
}

  JSON+="\t \"history\":{\n";
  for(int i = 0;i<tile.history.size();i++) {
    JSON+="\t\t\"";
    JSON+=to_string(tile.history[i].first);
    JSON+="\":\"";
    JSON+=tile.history[i].second;
    JSON+="\"";
    if (i+1<tile.history.size())
      JSON+=",";
    JSON+="\n";
  }
  JSON+="\t}\n";

  JSON+="}";

  return JSON;
}

/*This function takes the vector of vectors of Hex coordinates that represent the
path the party has taken and converts into a JSON array as part of a larger JSON */
string pathToJSON(vector< vector< pair<int,int>>> path) {
  string JSON = "[\n";
  for (int i = 0; i < path.size(); i++) {
    JSON+= "\t[\n";
    for (int j = 0; j < path[i].size(); j++) {
      JSON+= "\t\t{ \"x\":";
      JSON+=to_string(path[i][j].first);
      JSON+=", \"y\":";
      JSON+=to_string(path[i][j].second);
      JSON+=" }";
      if (j < path[i].size() - 1)
        JSON+=",";
      JSON+="\n";
    }
    JSON+="]";
    if (i < path.size() - 1)
      JSON+=",";
    JSON+="\n";
  }
  JSON+="]";
  return JSON;
}

int main(int argc, char** argv) {
  ifstream myfile;
  ofstream newfile;
  myfile.open("Compass-Definitive.txt");
  newfile.open("map.json");
  int t = 0;
  string line;
  vector <vector <Hex>> hexmap;
  int ycor = 0;
  int xcor = -1; //set to -1 since it will be increased to 0 before being used
  /*The three below booleans are used to determine what part of the file is being read
  and what to do based on the part of the file being read*/
  bool terrain_check = false;
  bool note_check = false;
  bool scan = false;

//This is the point used to help check if points lie within a certain polygon
  pair <int,int> ray = make_pair(-10000,-9999);

/*This section of code is used to create the polygons representing the borders
of countries on the maps. Each pair is added to the vector twice since the end
point of one line segment is the starting point of another line segment such
that every two elements in the vector represent 1 line segment of the polygon*/

  //OZANAO
  vector <pair<int,int>> Ozanao;
  pair <int,int> o1 = make_pair(0,299);
  Ozanao.push_back(o1);
  pair <int,int> o2 = make_pair(0,158);
  Ozanao.push_back(o2);
  Ozanao.push_back(o2);
  pair <int,int> o3 = make_pair(43,158);
  Ozanao.push_back(o3);
  Ozanao.push_back(o3);
  pair <int,int> o4 = make_pair(50,179);
  Ozanao.push_back(o4);
  Ozanao.push_back(o4);
  pair <int,int> o5 = make_pair(131,196);
  Ozanao.push_back(o5);
  Ozanao.push_back(o5);
  pair <int,int> o6 = make_pair(129,268);
  Ozanao.push_back(o6);
  Ozanao.push_back(o6);
  pair <int,int> o7 = make_pair(21,299);
  Ozanao.push_back(o7);
  Ozanao.push_back(o7);
  Ozanao.push_back(o1);

  //ILIRA
  vector <pair<int,int>> Ilira;
  pair <int,int> i1 = make_pair(0,0);
  Ilira.push_back(i1);
  pair <int,int> i2 = make_pair(76,0);
  Ilira.push_back(i2);
  Ilira.push_back(i2);
  pair <int,int> i3 = make_pair(99,48);
  Ilira.push_back(i3);
  Ilira.push_back(i3);
  pair <int,int> i4 = make_pair(77,71);
  Ilira.push_back(i4);
  Ilira.push_back(i4);
  pair <int,int> i5 = make_pair(37,98);
  Ilira.push_back(i5);
  Ilira.push_back(i5);
  pair <int,int> i6 = make_pair(0,98);
  Ilira.push_back(i6);
  Ilira.push_back(i6);
  Ilira.push_back(i1);

  //AKHISIR
  vector <pair<int,int>> Akhisir;
  pair <int,int> a1 = make_pair(110,40);
  Akhisir.push_back(a1);
  pair <int,int> a2 = make_pair(189,35);
  Akhisir.push_back(a2);
  Akhisir.push_back(a2);
  pair <int,int> a3 = make_pair(226,71);
  Akhisir.push_back(a3);
  Akhisir.push_back(a3);
  pair <int,int> a4 = make_pair(227,91);
  Akhisir.push_back(a4);
  Akhisir.push_back(a4);
  pair <int,int> a5 = make_pair(209,100);
  Akhisir.push_back(a5);
  Akhisir.push_back(a5);
  pair <int,int> a6 = make_pair(182,125);
  Akhisir.push_back(a6);
  Akhisir.push_back(a6);
  pair <int,int> a7 = make_pair(174,168);
  Akhisir.push_back(a7);
  Akhisir.push_back(a7);
  pair <int,int> a8 = make_pair(131,190);
  Akhisir.push_back(a8);
  Akhisir.push_back(a8);
  pair <int,int> a9 = make_pair(80,184);
  Akhisir.push_back(a9);
  Akhisir.push_back(a9);
  pair <int,int> a10 = make_pair(54,177);
  Akhisir.push_back(a10);
  Akhisir.push_back(a10);
  pair <int,int> a11 = make_pair(43,156);
  Akhisir.push_back(a11);
  Akhisir.push_back(a11);
  pair <int,int> a12 = make_pair(39,126);
  Akhisir.push_back(a12);
  Akhisir.push_back(a12);
  pair <int,int> a13 = make_pair(62,89);
  Akhisir.push_back(a13);
  Akhisir.push_back(a13);
  Akhisir.push_back(a1);

  //XACAMAL
  vector <pair<int,int>> Xacamal;
  pair <int,int> x1 = make_pair(234,74);
  Xacamal.push_back(x1);
  pair <int,int> x2 = make_pair(299,74);
  Xacamal.push_back(x2);
  Xacamal.push_back(x2);
  pair <int,int> x3 = make_pair(299,190);
  Xacamal.push_back(x3);
  Xacamal.push_back(x3);
  pair <int,int> x4 = make_pair(255,173);
  Xacamal.push_back(x4);
  Xacamal.push_back(x4);
  pair <int,int> x5 = make_pair(236,183);
  Xacamal.push_back(x5);
  Xacamal.push_back(x5);
  pair <int,int> x6 = make_pair(216,167);
  Xacamal.push_back(x6);
  Xacamal.push_back(x6);
  pair <int,int> x7 = make_pair(195,166);
  Xacamal.push_back(x7);
  Xacamal.push_back(x7);
  pair <int,int> x8 = make_pair(197,136);
  Xacamal.push_back(x8);
  Xacamal.push_back(x8);
  pair <int,int> x9 = make_pair(186,123);
  Xacamal.push_back(x9);
  Xacamal.push_back(x9);
  pair <int,int> x10 = make_pair(205,106);
  Xacamal.push_back(x10);
  Xacamal.push_back(x10);
  pair <int,int> x11 = make_pair(229,99);
  Xacamal.push_back(x11);
  Xacamal.push_back(x11);
  Xacamal.push_back(x1);

  //Wichahawk
  vector <pair<int,int>> Wichahawk;
  pair <int,int> w1 = make_pair(299,191);
  Wichahawk.push_back(w1);
  pair <int,int> w2 = make_pair(299,276);
  Wichahawk.push_back(w2);
  Wichahawk.push_back(w2);
  pair <int,int> w3 = make_pair(183,276);
  Wichahawk.push_back(w3);
  Wichahawk.push_back(w3);
  pair <int,int> w4 = make_pair(183,199);
  Wichahawk.push_back(w4);
  Wichahawk.push_back(w4);
  pair <int,int> w5 = make_pair(209,165);
  Wichahawk.push_back(w5);
  Wichahawk.push_back(w5);
  pair <int,int> w6 = make_pair(224,173);
  Wichahawk.push_back(w6);
  Wichahawk.push_back(w6);
  pair <int,int> w7 = make_pair(229,181);
  Wichahawk.push_back(w7);
  Wichahawk.push_back(w7);
  pair <int,int> w8 = make_pair(241,180);
  Wichahawk.push_back(w8);
  Wichahawk.push_back(w8);
  pair <int,int> w9 = make_pair(257,173);
  Wichahawk.push_back(w9);
  Wichahawk.push_back(w9);
  Wichahawk.push_back(w1);

  //Creating Path Vector
  vector <vector <pair <int,int>>> path;

//Once this part of the file is reached, all the relavent information has alread been retrieved
  while (line != "</map>") {
    getline(myfile, line);
    removeZeroWidth(line);

/*When this line is read, that means that there is no terrain information on this
line or the following lines*/
    if (line == "</tilerow>") {
      terrain_check = false;
    }

/*This part of the code creates new Hexes for the hexmap, determines their terrain
and determines what country the Hex is in, if any*/
    if (terrain_check) {
      int terrain = terrainIdentifier(line); //gets the terrain
      hexmap[xcor].push_back(Hex(xcor,ycor,terrain)); //adds new Hex to hexmap

      pair<int,int> cord = make_pair(xcor,ycor);

      bool insideO = isInside(cord,ray,Ozanao);
      bool insideI = isInside(cord,ray,Ilira);
      bool insideA = isInside(cord,ray,Akhisir);
      bool insideX = isInside(cord,ray,Xacamal);
      bool insideW = isInside(cord,ray,Wichahawk);

      //special check to see if point is on border-zone of two countries
      if (insideX && insideW)
        hexmap[xcor][ycor].country = "Xacamal-Wichahawk Border";
      else if (insideO)
        hexmap[xcor][ycor].country = "Ozanao";
      else if (insideI)
        hexmap[xcor][ycor].country = "Ilira";
      else if (insideA)
        hexmap[xcor][ycor].country = "Akhisir";
      else if (insideX)
        hexmap[xcor][ycor].country = "Xacamal";
      else if (insideW)
        hexmap[xcor][ycor].country = "Wichahawk";
      //another special check to see if point is on border-zone of two countries
      else if (xcor > 211 && ycor > 165 && ycor < 192)
        hexmap[xcor][ycor].country = "Xacamal-Wichahawk Border";
      //Only Ozanao and a specific Hex are visible to the public
      if ((xcor == 56 && ycor == 1) || insideO)
        hexmap[xcor][ycor].visible = true;


      ycor++;
    }

//When this line is read, it means it's time to scan another row of the map
    if (line == "<tilerow>") {
      hexmap.push_back(vector<Hex>());
      xcor++;
      ycor = 0;
      terrain_check = true;
    }

/*When this line is read it means it's time to check the notes of the input file
This will happen after the entire 300x300 hexmap vector is created and all terrain checked*/
    if (note_check) {
      if (scan) {
/*This part of the code gets information about the notes of a particular Hex and
and adds that information to the appropriate Hex in the hexmap*/
    getSteps(line,path,xcor,ycor);

    string info = getInfo(line);
    hexmap[xcor][ycor].description = info;
    string feat = getFeat(line);
    hexmap[xcor][ycor].feature = numericFeat(feat);
    vector <pair<int,string>> hist = getHist(line);
    hexmap[xcor][ycor].history = hist;
    vector <int> days;
    for (int i=0;i<hist.size();i++)
      days.push_back(hist[i].first);
    hexmap[xcor][ycor].visited = days;
    scan = false; //scan is set to false until a new note for a new Hex is read
  }


    pair<int,int> hex_cords = getCords(line);
    //This is a check to make sure that the coordinates are valid
    if (hex_cords.first != -1 && hex_cords.second != -1) {
      //Notes only need to be scanned if they correspond to a Hex that is visible
      if (hexmap[hex_cords.first][hex_cords.second].visible) {
          scan = true;
          xcor = hex_cords.first;
          ycor = hex_cords.second;
        }
    }
  }
    if (line == "<notes>")
      note_check = true;
  }

//After reading all info from the file, start creating the JSON file
//This section creates an array of arrays of hexes
  newfile<<"{\n\t\"world_info\":[\n";
  for (int i=0;i<hexmap.size();i++) {
    newfile<<"\t[\n";
    for (int j=0;j<hexmap[i].size();j++) {
      newfile<<hexToJSON(hexmap[i][j]);
      if (j+1<hexmap[i].size())
        newfile<<",";
      newfile<<"\n";
    }
    newfile<<"\t]";
    if (i+1<hexmap.size())
      newfile<<",";
    newfile<<"\n";
    }
  newfile<<"],";

//This section gets how many days have passed so far
  newfile<<"\n\t\"max_days\":";
  newfile<<to_string(path.size());
  newfile<<",";

//This section creates an array of array of coordinates that the party traveled
  newfile<<"\n\t\"path\":";
  newfile<<pathToJSON(path);
  newfile<<"\n}";

  myfile.close();
  newfile.close();
}
