// ===================================================================
// $Id: ObjParser.cpp,v 1.1 2007/02/14 12:03:52 bittner Exp $

// C++ standard headers
#include <cstdlib>
#include <iostream>
#include <list>
#include <map>

using namespace std;
#include "gzstream.h"

// Local headers
#include "Scene.h"
#include "ObjParser.h"
#include "Triangle.h"

typedef vector<mmVector3> VertexContainer;
typedef vector<int> VertexIndexContainer;

bool
LoadTriangles(char *str,
	      const VertexContainer &vertices, 
	      TriangleContainer &triangles)
{
  char *pch = strtok(str + 1, " ");
  
  VertexIndexContainer indices;
  
  while (pch != NULL) {
    const int index = (int)strtol(pch, NULL, 10) - 1;

    //Debug << index << " x ";
    // store vertex in hash table
    //	hashTable[index] = vertices[index];
    if (index >= 0 && index < vertices.size())
      indices.push_back(index);
    else
      Debug<<"Error: index out of range "<<index<<endl<<flush;

	// ignore normals...
    pch = strtok(NULL, " ");
  }
  
  if (indices.size() < 3) {
    Debug<<"Error: small number of triangle indices "<<endl<<flush;
  }
  else {
    triangles.push_back(new Triangle(vertices[indices[0]],
									 vertices[indices[1]],
									 vertices[indices[2]]));
    
    if (indices.size() > 3) {
      // assume a convex face and triangulate it by fan
      for (int i=0; i < indices.size() - 3; i++) {
		triangles.push_back(new Triangle(vertices[indices[0]],
										 vertices[indices[i+2]],
										 vertices[indices[i+3]]));
      } // for
    }
  }
  
  return true;
}


// Parses a file and sets the scene instrance
bool
ObjParser::ParseFile(const string filename,
					 Scene &scene)
{
  FILE *file;

  igzstream inStream(filename.c_str());
  
  // some error had occured
  if (inStream.fail()) {
    cerr << "Error: Can't open file " << filename.c_str() << " for reading (err. "
         << inStream.rdstate() << ").\n";
    return false;
  }
  
  VertexContainer vertices; // table for vertices
  
  char str[2048];

  // tmeporal fix for PowerPlant
  //scene.triangles.reserve(15000000);
  //  vertices.reserve(30000000);
  
  while(1) {
    inStream.getline(str, 2048);
    
    if (!inStream)
      break;

    switch (str[0]) {
	case 'v': // vertex or normal
	  {
		switch (str[1]) {
		case 'n' :
		  // normal do nothing
		  break;
		case 't' :
		  // texture do nothing
		  break;
		default:
		  float x, y, z; //cout << "v";	
		  sscanf(str + 1, "%f %f %f", &x, &y, &z);
		  vertices.push_back(mmVector3(x,y,z));
		  //cout << "vertex: " << vertices.back() << endl;
		}
		break;
	  }
	case 'f': 
      {
		LoadTriangles(str, vertices, scene.triangles);
		break;
      } 
	default:
	  break;
	}
  }
  
return true;
}


bool
ObjParser::ExportFile(const string filename,
					  Scene &scene,
					  const int offset)
{
  ofstream f(filename.c_str());
  int index = 1;
  cout<<"traingles for export:"<<scene.triangles.size()<<endl;

  for (int i=offset; i < scene.triangles.size(); i++) {
	Triangle *t = scene.triangles[i];
	for (int j=0; j < 3; j++)
	  f<<"v "<<t->vertices[j].x<<" "<<t->vertices[j].y<<" "<<t->vertices[j].z<<endl;
	f<<"f "<<index<<" "<<index+1<<" "<<index+2<<endl;
	index+=3;
  }

  f<<"#finished export"<<endl<<flush;
  return true;
}

// End of file ObjParser.cpp



