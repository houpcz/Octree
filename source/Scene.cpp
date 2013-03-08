// ===================================================================
// $Id: Scene.cpp,v 1.1 2007/02/14 12:03:52 bittner Exp $

// local headers
#include "BinParser.h"
#include "ObjParser.h"
#include "LstParser.h"
#include "Scene.h"

static int
SplitFilenames(const string str,
			   vector<string> &filenames)
{
  int pos = 0;
  
  while(1) {
	int npos = (int)str.find(';', pos);
	
	if (npos < 0 || npos - pos < 1)
	  break;
	filenames.push_back(string(str, pos, npos - pos));
	pos = npos + 1;
  }
  
  filenames.push_back(string(str, pos, str.size() - pos));
  return (int)filenames.size();
}


void
Scene::Parse(const string filename)
{
  long t1 = GetTime();

  vector<string> files;
  SplitFilenames(filename, files);

  for (int i=0; i < files.size(); i++)
	ParseFile(files[i]);
  
  // Measure and report time
  long t2 = GetTime();
  float parseTime = TimeDiff(t1, t2)*1e-3;

  cout << "Info: scene parsed in "
       << parseTime << " seconds" << endl;
  
  Debug << "Info: scene parsed in " << parseTime << " seconds" << endl
	<< "Info: #triangles = " << triangles.size() << endl;

  
  
  return;
}

void
Scene::ParseFile(const string filename)
{
  if (strstr(filename.c_str(), ".lst")) {
	LstParser *lstParser = new LstParser;
	lstParser->ParseFile(filename, *this);
	delete lstParser;
  } else {
	// first try to get cached version of the file in banary format
	BinParser *binParser = new BinParser;
	
	bool binaryParsed = binParser->ParseFile(filename + ".bin", *this);
	
	if (!binaryParsed) {
	  // Allocate parser for the detected scene format
	  Parser *parser;
	  // Currently only .obj is supported
	  parser = new ObjParser;
	  
	  int offset = triangles.size();
	  // Parse the data
	  parser->ParseFile(filename, *this);
	  
	  // export the scene for faster parsing next time
	  binParser->ExportFile(filename + ".bin", *this, offset);
	}
  }
}

void
Scene::Preprocess()
{
  TriangleContainer::const_iterator ti = triangles.begin();
  
  for (; ti != triangles.end(); ti++)
    (*ti)->color.Random();
  
  ComputeBoundingBox();
  // Normalize();
  
  bvh = new Bvh(*this);
  bvh->Construct();

  return;
}

void
Scene::ComputeBoundingBox()
{
  // Compute bounding box of the scene
  box.Reset();
  TriangleContainer::const_iterator ti = triangles.begin();
  
  for (; ti != triangles.end(); ti++) {
    box.Include((*ti)->vertices[0]);
    box.Include((*ti)->vertices[1]);
    box.Include((*ti)->vertices[2]);
  } // for
  
  Debug << "Info: Scene bounding box " << box << endl;

  return;
}

void
Scene::Normalize()
{
  TriangleContainer::iterator ti = triangles.begin();
  // translates all triangles so that the bbox is symetric
  mmVector3 translation = -box.Center();

  for (; ti != triangles.end(); ti++) {
    (*ti)->vertices[0] += translation;
    (*ti)->vertices[1] += translation;
    (*ti)->vertices[2] += translation;
  } // for

  float scale = 1.0f/box.Diagonal().Size();
  for (ti = triangles.begin(); ti != triangles.end(); ti++) {
    (*ti)->vertices[0] *= scale;
    (*ti)->vertices[1] *= scale;
    (*ti)->vertices[2] *= scale;
  }

  // recompute the bounding box
  ComputeBoundingBox();

  return;
}

// End of file Scene.cpp

