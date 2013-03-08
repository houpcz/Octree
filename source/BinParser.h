// ===================================================================
// $Id: BinParser.h,v 1.1 2007/02/14 12:03:52 bittner Exp $
// This file has been written by Jiri Bittner, October 2006

#ifndef __BINPARSER_H
#define __BINPARSER_H

#include "Parser.h"

class BinParser :
  public Parser
{
public:
  
  BinParser(): Parser() {}
  
  virtual bool ParseFile(const string filename,
			 Scene &scene);
  
  virtual bool ExportFile(const string filename,
						  Scene &scene,
						  const int offset);
  
};

#endif // __BINPARSER_H
