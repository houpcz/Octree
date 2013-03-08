// ===================================================================
// $Id: AppEnvironment.h,v 1.1 2007/02/14 12:03:52 bittner Exp $
// This file has been written by Jiri Bittner, October 2006

#ifndef __APP_ENVIRONMENT_H__
#define __APP_ENVIRONMENT_H__

#include "Environment.h"

class AppEnvironment : public Environment
{
public:
  AppEnvironment():Environment() {}

  virtual void RegisterOptions();
  
};

#endif // __APP_ENVIRONMENT_H__

