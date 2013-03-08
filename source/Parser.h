// ===================================================================
// $Id: Parser.h,v 1.1 2007/02/14 12:03:52 bittner Exp $
// This file has been written by Jiri Bittner, October 2006

#ifndef __PARSER_H
#define __PARSER_H

// This is the abstract class of parser of a scene consisting of
// scenes primitives such as triangles

#include <string>
using namespace std;

// Forward declaration
class Scene;

// Abstract class of a parser
class Parser
{
public:
  Parser() { }

  virtual bool ParseFile(const string filename,
						 Scene &scene) = 0;

  virtual bool
  ExportFile(const string filename,
			 Scene &scene,
			 const int offset) = 0;

};


#endif //  __PARSER_H

