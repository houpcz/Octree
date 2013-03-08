// ===================================================================
// $Id: LstParser.cpp,v 1.1 2007/02/14 12:03:52 bittner Exp $

// C++ standard headers
#include <cstdlib>
#include <iostream>
#include <list>
#include <map>

using namespace std;

// Local headers
#include "Scene.h"
#include "Triangle.h"
#include "ObjParser.h"
#include "LstParser.h"

// Parses a file and sets the scene instrance
bool
LstParser::ParseFile(const string filename,
					 Scene &scene)
{
  // parse the filelist
  FILE *f = fopen(filename.c_str(), "rt");
  if (!f) {
	cerr<<"Cannot open .lst file"<<endl;
	exit(1);
  }
  
  char s[256];
  while (fgets(s,255,f)) {
	// remove nl
	s[strlen(s)-1] = 0;
	scene.Parse(s);
  }
  fclose(f);
  return true;
}
