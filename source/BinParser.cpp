#include <iostream>
using namespace std;
// This is the parser of BIN format containing triangles

#include "BinParser.h"
#include "Scene.h"

#define MAGIC 0x827923

// this have to be changed 
#define VERSION 1.1

bool
BinParser::ExportFile(const string filename,
					  Scene &scene,
					  const int offset)
{
  FILE *f = fopen(filename.c_str(), "wb");
  if (!f) {
    cerr << "Error: cannot open file for writing "
		 << filename.c_str() << endl;
    exit(1);
  }
  
  int buff[8];

  buff[0] = MAGIC;
  buff[1] = (int)VERSION*1000;
  buff[2] = scene.triangles.size() - offset;

  fwrite(buff, sizeof(int), 3, f);
  
  int i = offset;
  for (; i < scene.triangles.size(); i++) {
    fwrite(scene.triangles[i], sizeof(Triangle), 1, f);
  }

  fclose(f);
  return true;
}


bool
BinParser::ParseFile(const string filename,
					 Scene &scene)
{
  FILE *f = fopen(filename.c_str(), "rb");
  if (!f) {
    Debug << "Error: cannot open file for reading "
	  << filename.c_str() << endl;
    return false;
  }

  int buff[8];
  fread(buff, sizeof(int), 3, f);

  if (buff[0] != MAGIC) {
    Debug << "Error: wrong file type "
	  << filename.c_str() << endl;
    return false;
  }

  if (buff[1] != (int)VERSION*1000) {
    Debug << "Error: file version does match "
	  << filename.c_str() << endl;
    return false;
  }

  int number = buff[2];
  int offset = scene.triangles.size();
  scene.triangles.resize( offset + number);

#if 0
  int chunkSize = 1000;

  for (int i=0; i < number;) {
    int toRead = Min(chunkSize, number - i);
    fread(&(scene.triangles[i]), sizeof(Triangle), toRead, f);
    i += toRead;
  }
#else
  for (int i=0; i < number; i++) {
    scene.triangles[offset + i] = new Triangle;
    fread(scene.triangles[offset + i], sizeof(Triangle), 1, f);
  }
#endif
  fclose(f);
  return true;
}

// End of file BinParser.cpp
