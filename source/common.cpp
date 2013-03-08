// ===================================================================
// $Id: common.cpp,v 1.1 2007/02/14 12:03:52 bittner Exp $

// C++ standard headers
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include "common.h"

// local headers
#ifndef _MSC_VER
// this is for UNIX/Linux etc.
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/resource.h>
#else
// This is for MS Windows
#include <windows.h>
#include <sys/types.h>
#include <sys/timeb.h>
#endif

// global file streams
ofstream Debug;

// Puts the specified number of spaces out to cout.  Used for
// indentation during CObject3D::Describe().
void
indent(ostream &app, int ind)
{
  int i;

  for (i = 0; i < ind; i++)
    app << ' ';
}

// Global variables in the whole program
float Limits::Threshold = 1e-6f;
float Limits::Small = 0.01f;
float Limits::Infinity = 1e20f;

// Generates a random value as floating point
float
Random(float max)
{
#ifdef __UNIX__
  return (random()*max/0x7FFFFFFF);
#else
  return (((float)rand())/RAND_MAX)*max;
#endif
  
}

// 
int
Random(int max)
{
  return rand()%max;
}

// Randomizes the RND generator using a time event
void
Randomize()
{
  time_t t;
  srand((unsigned) time(&t));
}

// Randomizes the RND generator using a specified seed
void
Randomize(const unsigned int seed)
{
  srand(seed);
}

#ifdef  _MSC_VER
static bool hasHRTimer = false;
static LARGE_INTEGER hrFreq;
#endif

// Inits timing
void
InitTiming()
{
#ifdef  _MSC_VER
  hasHRTimer = (bool) QueryPerformanceFrequency(&hrFreq);
#endif
}

// Returns the local time from the start of the program
// The user time consumed by the program is reported
long
GetTime()
{
#ifndef  _MSC_VER
  static struct rusage r;
  getrusage(RUSAGE_SELF,&r);
  return r.ru_utime.tv_usec+1000000*r.ru_utime.tv_sec;
#else
  if (hasHRTimer) {
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    // return in usec
    return (long) (1000000*counter.QuadPart/(hrFreq.QuadPart));
  }
  else {
    static struct _timeb mtime;
    _ftime(&mtime);
    
    return 1000*(1000*mtime.time + mtime.millitm);
  }
#endif
}

// Computes the real time (if more processes on the computer,
// then it is different to user time)
long
GetRealTime()
{
#ifndef  _MSC_VER
  static struct timeval _tstart;
  static struct timezone tz;
  gettimeofday(&_tstart,&tz);
  return (long)(1000000*_tstart.tv_sec + _tstart.tv_usec);
#else
  if (hasHRTimer) {
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    // return in usec
    return (long) (1000000*counter.QuadPart/(hrFreq.QuadPart));
  }
  else {
    static struct _timeb mtime;
    _ftime(&mtime);
    return 1000*(1000*mtime.time + mtime.millitm);
  }
#endif
}

// return time diff. in ms
float
TimeDiff(long time1,long time2) // in ms
{
  const float clk=1.0e-3f; // ticks per second
  long t=time2-time1;
  
  return ((t<0)?-t:t)*clk;
}

char *
TimeString()
{
  time_t t;
  time(&t);
  return ctime(&t);
}

char *
GetAbsPath(char *name)
{
  //  char *url = new char[strlen(name)+1];
  //  strcpy(url,name);

  // get the absolute path
  int c = strlen(name);
  int i = c-1;
  bool wasDot = false;
  // search for double dot
  for (;i>=0;i--) 
    if (name[i]=='.') {
      if (wasDot)
	break;
      wasDot = true;
    }
    else
      wasDot = false;

  if (i > 0)
    i += 3;
  if (i < 0)
    i = 0;
  
  char *url = new char[c-i+1];
  int j = 0;
  for (;i < c;i++,j++)
    url[j] = name[i];
  url[j] = 0;
  
  return url;
}

char *
GetPath(const char *s)
{
  int i=strlen(s);
  for (; i>0; i--) {
    if (s[i]=='/' || s[i]=='\\')
      break;
  }
  
  char *path = new char[i+1];
  int j = 0;
  for (; j<i; j++)
    path[j] = s[j];
  path[j] = 0;
  return path;
}

char *
strdup(char *a)
{
  if (a) {
    char *s = new char[strlen(a)+1];
    strcpy(s,a);
    return s;
  } else
    return NULL;
}

bool
FileExists(char *filename)
{
  FILE *f;

  f = fopen(filename,"r");
  if (f) {
    fclose(f);
    return true;
  }
  return false;
}

// End of file common.cpp

