// ===================================================================
// $Id: Color.h,v 1.2 2007/02/28 16:45:42 bittner Exp $
// This file has been written by Jiri Bittner, October 2006

#ifndef __COLOR_H
#define __COLOR_H

#include <vector>
using namespace std;

#include "common.h"

// Color with alpha-channel for OpenGL rendering

struct mmColor
{
  mmColor() {}
  void Random() {
    r = ::RandomValue(0,255);
    g = ::RandomValue(0,255);
    b = ::RandomValue(0,255);
    a = 0;
  }
  unsigned char r, g, b, a;
};


inline mmColor
RainbowColorMapping(const float _value)
{
  mmColor color;

  float value = 4.0f*(1.0f - _value);

  if (value < 0.0f)
	value = 0.0f;
  else
	if (value > 4.0f)
	  value = 4.0f;

#define MAX_COLOR_VALUE 255
#define MIN_COLOR_VALUE 0

  int band = (int)(value);
  value -= band;
  
  switch (band) {
  case 0:
	color.r = MAX_COLOR_VALUE;
	color.g = value*MAX_COLOR_VALUE;
	color.b = MIN_COLOR_VALUE;
	break;
  case 1:
	color.r = (1.0f - value)*MAX_COLOR_VALUE;;
	color.g = MAX_COLOR_VALUE;
	color.b = MIN_COLOR_VALUE;
	break;
  case 2:
	color.r = MIN_COLOR_VALUE;
	color.g = MAX_COLOR_VALUE;
	color.b = value*MAX_COLOR_VALUE;
	break;
  case 3:
	color.r = MIN_COLOR_VALUE;
	color.g = (1.0f - value)*MAX_COLOR_VALUE;;
	color.b = MAX_COLOR_VALUE;
	break;
  default:
	color.r = value*MAX_COLOR_VALUE;
	color.g = MIN_COLOR_VALUE;
	color.b = MAX_COLOR_VALUE;
	break;
  }

  return color;
}


#endif // __COLOR_H

