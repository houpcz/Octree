// ===================================================================
// $Id: LstParser.h,v 1.1 2007/02/14 12:03:52 bittner Exp $
// This file has been written by Jiri Bittner, October 2006

#ifndef __LSTPARSER_H
#define __LSTPARSER_H

#include "Parser.h"

// This is a parser for .lst files
// Each line of the file represents a file to parse

class LstParser :
  public Parser
{
 public:
  
  LstParser(): Parser() {}
  
  virtual bool ParseFile(const string filename,
						 Scene &scene);
  
  virtual bool
  ExportFile(const string filename,
			 Scene &scene,
			 const int offset
			 )
  {
	return false;
  }
  
};


#endif  // __LSTPARSER_H
