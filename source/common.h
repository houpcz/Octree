// ================================================================
// $Id: common.h,v 1.2 2007/03/02 12:41:24 bittner Exp $
// ****************************************************************
// 
/** \file common.h
Common defines for the ERS system.

This file contains various macros, templates and constants for the ERS system.

@author Jiri Bittner
*/

#ifndef __COMMON_H
#define __COMMON_H


#include <cmath>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <climits>
#include <string>

using namespace std;


// This constant should be used for the length of the array char for filenames
// etc., for example: char filename[MaxStringLength]
const int MaxStringLength = 2048;

#if defined(_MSC_VER)
#pragma warning(disable:4018)
#pragma warning(disable:4800)
//#pragma warning(disable:4244)

// Note matt: comment this out because conflicts with definition in qt library!!
#if 0
typedef unsigned int uint;
#endif

typedef unsigned short ushort;
typedef unsigned char uchar;
typedef unsigned long ulong;
#endif

#if defined(__GNUC__) || defined(_MSC_VER)
#define DIRCAT '.'
#endif

#if !defined(__WATCOMC__) && !defined(__CYGWIN32__) && !defined(_MSC_VER)
#include <values.h>
#else // __WATCOMC__
#define M_PI        3.14159265358979323846
#define MAXFLOAT    3.40282347e+37F
#endif // __WATCOMC__

// some compilers do not define the bool yet, but it was declared by ANSI
#if !defined(__WATCOMC__) && !defined(_MSC_VER)
#if defined (__GNUC__) || (_BOOL)
//#error "HAS BOOL defined"
#define HAS_BOOL
#endif
#else // __WATCOMC__
#if  (__WATCOMC__ > 1060)
//#error "Watcom HAS BOOL defined"
#define HAS_BOOL
#endif
#endif // __WATCOMC__


#if defined(__WATCOMC__) || defined(_MSC_VER)
#define strcasecmp stricmp
#define strncasecmp strnicmp
#endif // __WATCOMC__

// matt
#define USE_GZLIB 1

#if USE_GZLIB

#define OUT_BIN_MODE ios::out
#define IN_BIN_MODE ios::in

#else

#ifdef sgi
#define OUT_BIN_MODE ios::out
#define IN_BIN_MODE ios::in
#else // sgi
#if defined(__WATCOMC__) || defined(_MSC_VER)
#define OUT_BIN_MODE ios::out | ios::binary
#define IN_BIN_MODE ios::in | ios::binary
#else
#define OUT_BIN_MODE ios::out | ios::bin
#define IN_BIN_MODE ios::in | ios::bin
#endif // __WATCOMC_
#endif // sgi

#endif

typedef unsigned long dword;

#ifndef NULL
#define NULL (void *)0
#endif // NULL

typedef unsigned char byte;

#ifndef __GNUG__
// typedef int bool;
#endif

#ifndef getch
#define getch() getchar()
#endif

#define THRESHOLD 1.0e-5

#ifndef PI
#define PI    3.14159265358979323846f
#endif

#define MIN_FLOAT -1e25f
#define MAX_FLOAT  1e25f

#ifndef DEL_PTR
#define DEL_PTR(ptr) do {if (ptr) {        \
                           delete (ptr);   \
			   (ptr) = 0;}}    \
                         while (0)
#endif // DEL_PTR


// Clears a container (i.e., a vector of pointers) and deletes the pointers
#if 0
#ifndef CLEAR_CONTAINER
#define CLEAR_CONTAINER(co) do { while (!(co).empty()) {   \
                                 DEL_PTR((co).back()); \
                                 (co).pop_back();} \
                               } while (0)
#endif

#else

#ifndef CLEAR_CONTAINER
#define CLEAR_CONTAINER(co) do { for (int _i = 0; _i < (int)(co).size(); ++ _i) { \
                                 DEL_PTR((co)[_i]);} \
                                 (co).clear(); } \
                                 while (0)
#endif

#endif


inline
int Signum(const float a, const float thresh = THRESHOLD)
{
  if (a>thresh)
    return 1; 
  else
    if (a<-thresh)
      return -1; 
  return 0;
}

inline double Absd(const double a)
{
  return (a >= 0.0) ? a : -a;
}

inline float Abs(const float a)
{
  return (a >= 0.0f) ? a : -a;
}

// =======================================================
// Comparing things
//struct Limits {
//  const float thresh=THRESHOLD;
//  const float small=0.1;
//};

template <class T>
bool
ClipValue(T &v, const T m, const T M)
{
  if (v<m) {
    v = m;
    return true;
  }
  if (v>M) {
    v = M;
    return true;
  }

  return false;
}


inline int
eq(float a, float b, float t=THRESHOLD)
{
  return Abs(a-b)<t;
}

inline int
leq(float a, float b, float t=THRESHOLD)
{
  return a - b < t;
}

inline int
geq(float a, float b, float t=THRESHOLD)
{
  return t > b - a;
}

inline int
le(float a, float b, float t=THRESHOLD)
{
  return !geq(a, b, t);
}

inline int
ge(float a, float b, float t=THRESHOLD)
{
  return !leq(a, b, t);
}

// ========================================================

// -------------------------------------------------------------------
// Indents to a given stream by the number of spaces specified.
// This routine is located in main.cpp, for lack of a better place.
// -------------------------------------------------------------------
void indent(ostream &app, int ind);

// ---------------------------------------------------------
// RandomValue
//	Returns a random floating-point value between the two 
//	values.  Range is inclusive; the function should 
//	occasionally return exactly a or b.
// ---------------------------------------------------------
inline float
RandomValue(float a, float b)
{
  float range = (float) Abs(a - b);
  return ((float)rand() / RAND_MAX) * range + ((a < b) ? a : b);
}


inline int
RandomIntValue(int a, int b)
{
  int range = abs(a - b);
  return (rand() * range) / RAND_MAX + ((a < b) ? a : b);
}

inline float sqr(float a)
{
  return a*a;
}


template <class T>
void Swap(T &a,T &b)
{
  T c;                 
  c = b;
  b = a;
  a = c;
}

template <class T>
int eq(T &a, T &b, T &c, T &d) {
  return a == b && c==d && b==c;
}

template <class T>
T Min(T a,T b)
{
  return a<b ? a : b;
}

template <class T>
T Max(T a,T b)
{
  return a>b ? a : b;
}


// Generate a floating point random value between 0 and fmax
float Random(float fmax);

// Generate an integer random value between 0 and fmax
int  Random(int max);
// Randomize RND generator
void Randomize();
//  Randomize RND generator using a seed 
void
Randomize(const unsigned int seed);


void GetKey(char *s=NULL);

inline float Deg2Rad(const float a)
{
  return a*(PI/180.0f);
}

inline float Rad2Deg(const float a) {
  return a*(180.0f/PI);
}

// Some functions to measure a time
void InitTiming();
long GetTime();
long GetRealTime();
float TimeDiff(long t1,long t2);
char *TimeString();

// manipulator
inline ostream &DEBUGINFO( ostream &s ) {
  return s<<"FILE "<<__FILE__<<",LINE "<<__LINE__<<endl;
}

#define DINFO __FILE__<<":"<<__LINE__


// -------------------------------------------------------------------
// Limits.
//  This class encapsulates all the concessions to floating-point
//  error made by ray tracers.
// -------------------------------------------------------------------

class Limits
{
public:
  // This is the number used to reject too-close intersections.
  // The default value is 1.0.
  static float Threshold;

  // This is a "small" number.  Less than this number is assumed to
  // be 0, when such things matter.
  // The default value is 0.1.
  static float Small;

  // This is an impractically "large" number, used for intersection
  // parameters out to infinity (e.g. the span resulting from an
  // FindAllIntersections operation on a plane).
  // The default value is 100000.
  static float Infinity;
};

// ---------------------------------------------------------
// EpsilonEqual(x,y)
//   Returns if two values are equal or not (by epsilon)
// ---------------------------------------------------------
inline int
EpsilonEqual(const float &x, const float &y)
{
  return fabs(x-y) < Limits::Small;
}

// ---------------------------------------------------------
// EpsilonEqual(x)
//   Returns if a value is zero (+/- epsilon)
// ---------------------------------------------------------
inline int
EpsilonEqual(const float &x)
{
  return fabs(x) < Limits::Small;
}

// ---------------------------------------------------------
// EpsilonEqual(x,y,epsilon)
//   Returns if two values are equal or not by a given epsilon
// ---------------------------------------------------------
inline int
EpsilonEqual(const float &x, const float &y, const float &epsilon)
{
  return fabs(x-y) < epsilon;
}

// ---------------------------------------------------------
// InRange
//	Returns nonzero if min <= candidate <= max.
// ---------------------------------------------------------
template<class T>
inline int 
InRange(T min, T max, T candidate)
{
  return (candidate >= min) && (candidate <= max);
}

// --------------------------------------------------------------
// string function with new operator

inline char*
StrDup(char *src) {
  char *p;
  for (p = src; *p++; );
  char *dest = new char[p-src];
  for ( p = dest;(*p++ = *src++) != 0; );
  return dest;
}


inline char *
StrToLower(char *src) {
  char *p;
  for (p = src; *p; p++)
    *p = tolower(*p);
  return src;
}

// return l = log2(a) if a is a power of two else -ceillog2(a)
inline int
GetLog2(int a)
{
  int i, x;
  i = 0;
  x = 1;

  while (x < a) {
    i++;
    x <<= 1;
  }

  return (x==a) ? i: -i;
}


// return ceil(log2(a)) even if a is not a power of two
// Example:
// GetCeilLog2(15) = 4
// GetCeilLog2(16) = 4
// GetCeilLog2(17) = 5
inline int
GetCeilLog2(int a)
{
  int i, x;
  if (a < 0)
    return -1;

  i = 0;
  x = 1;

  while (x < a) {
    i++;
    x <<= 1;
  }

  return i;
}

// Function to work with paths to files
char *
GetAbsPath(char *path);

char *
strdup(char *a);

bool
FileExists(char *filename);

#define DEBUG_LEVEL 5
//#define DEBUG_LEVEL 1000
//#define DEBUG_LEVEL 50000
  
// debug stream
extern ofstream Debug;


bool
CreateDir(char *dir);

char *
GetPath(const char *s);



inline string ReplaceSuffix(string filename, string a, string b)
{
  string result = filename;
  
  int pos = (int)filename.rfind(a, (int)filename.size() - 1);
  if (pos == filename.size() - a.size()) {
	result.replace(pos, a.size(), b);
  }
  return result;
}

#if USE_GZLIB
// type of out put and input streams
#define OUT_STREAM ogzstream
#define IN_STREAM igzstream
#else
#define OUT_STREAM ofstream
#define IN_STREAM ifstream
#endif

#endif //  __COMMON_H











