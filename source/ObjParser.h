// ===================================================================
// $Id: ObjParser.h,v 1.1 2007/02/14 12:03:52 bittner Exp $
// This file has been written by Jiri Bittner, October 2006

#ifndef __OBJPARSER_H
#define __OBJPARSER_H

#include "Parser.h"

// This is the parser of Wavefront OBJ format restricted to triangles
// Some documentation is at http://www.eg-models.de/formats/Format_Obj.html
// and http://www.fileformat.info/format/wavefrontobj/

class ObjParser :
  public Parser
{
 public:
  
  ObjParser(): Parser() {}
  
  virtual bool ParseFile(const string filename,
						 Scene &scene);

  virtual bool
  ExportFile(const string filename,
			 Scene &scene,
			 const int offset
			 );
	
  
};


#endif  // __OBJPARSER_H
