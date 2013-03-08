// ===================================================================
// $Id: mmVector3.h,v 1.1 2007/02/14 12:03:52 bittner Exp $
// This file has been written by Vlastimil Havran, October 1997

#ifndef __MMVECTOR3_H
#define __MMVECTOR3_H

#include <iostream>
using namespace std;
#include "common.h"

struct mmVector3
{
  float x, y, z;
  
  mmVector3() {}
  mmVector3(const float _x,
	  const float _y,
	  const float _z):x(_x), y(_y), z(_z) {}

  float operator[](const int index) const {
    return *(&x + index);
  }

  float Size() const {
    return sqrt(sqr(x) + sqr(y) + sqr(z));
  }
  
  void Minimize(const mmVector3 &candidate) {
    if (candidate.x < x)
      x = candidate.x;
    if (candidate.y < y)
      y = candidate.y;
    if (candidate.z < z)
      z = candidate.z;
  }

  void Maximize(const mmVector3 &candidate) {
    if (candidate.x > x)
      x = candidate.x;
    if (candidate.y > y)
      y = candidate.y;
    if (candidate.z > z)
      z = candidate.z;
  }

  int
  MajorAxis() const {
    if (x > y) {
      if (x > z)
	return 0;
      else
	return 2;
    }
    else
      if (y > z)
	return 1;
    return 2;
  }

  friend inline float DotProd(const mmVector3 &A, const mmVector3 &B);
  friend inline mmVector3 CrossProd(const mmVector3 &A, const mmVector3 &B);

  // Unary operators
  mmVector3 operator+ () const;
  mmVector3 operator- () const;

  // Assignment operators
  mmVector3& operator+= (const mmVector3 &A);
  mmVector3& operator-= (const mmVector3 &A);
  mmVector3& operator*= (const mmVector3 &A);
  mmVector3& operator*= (float A);
  mmVector3& operator/= (float A);

  friend inline mmVector3 operator+ (const mmVector3 &A, const mmVector3 &B);
  friend inline mmVector3 operator- (const mmVector3 &A, const mmVector3 &B);
  friend inline mmVector3 operator* (const mmVector3 &A, const mmVector3 &B);
  friend inline mmVector3 operator* (const mmVector3 &A, float B);
  friend inline mmVector3 operator* (float A, const mmVector3 &B);

  /// the magnitude=size of the vector
  friend inline float Magnitude(const mmVector3 &v);
  /// the squared magnitude of the vector .. for efficiency in some cases
  friend inline float SqrMagnitude(const mmVector3 &v);

  friend ostream &operator<<(ostream &s, const mmVector3 &v);

  // creates the vector of unit size corresponding to given vector
  friend inline mmVector3 Normalize(const mmVector3 &A);

};

inline ostream&
operator<< (ostream &s, const mmVector3 &A)
{
  return s << "(" << A.x << ", " << A.y << ", " << A.z << ")";
}


inline mmVector3
mmVector3::operator+() const
{
  return *this;
}

inline mmVector3
mmVector3::operator-() const
{
  return mmVector3(-x, -y, -z);
}

inline mmVector3&
mmVector3::operator+=(const mmVector3 &A)
{
  x += A.x;  y += A.y;  z += A.z;
  return *this;
}

inline mmVector3&
mmVector3::operator-=(const mmVector3 &A)
{
  x -= A.x;  y -= A.y;  z -= A.z;
  return *this;
}

inline mmVector3&
mmVector3::operator*= (float A)
{
  x *= A;  y *= A;  z *= A;
  return *this;
}

inline mmVector3&
mmVector3::operator/=(float A)
{
  float a = 1.0f/A;
  x *= a;  y *= a;  z *= a;
  return *this;
}

inline mmVector3&
mmVector3::operator*= (const mmVector3 &A)
{
  x *= A.x;  y *= A.y;  z *= A.z;
  return *this;
}


inline mmVector3
operator+ (const mmVector3 &A, const mmVector3 &B)
{
  return mmVector3(A.x + B.x, A.y + B.y, A.z + B.z);
}

inline mmVector3
operator- (const mmVector3 &A, const mmVector3 &B)
{
  return mmVector3(A.x - B.x, A.y - B.y, A.z - B.z);
}

inline mmVector3
operator* (const mmVector3 &A, const mmVector3 &B)
{
  return mmVector3(A.x * B.x, A.y * B.y, A.z * B.z);
}

inline mmVector3
operator* (const mmVector3 &A, float B)
{
  return mmVector3(A.x * B, A.y * B, A.z * B);
}

inline mmVector3
operator* (float A, const mmVector3 &B)
{
  return mmVector3(B.x * A, B.y * A, B.z * A);
}


inline float
DotProd(const mmVector3 &A, const mmVector3 &B)
{
  return A.x * B.x + A.y * B.y + A.z * B.z;
}

inline mmVector3
CrossProd (const mmVector3 &A, const mmVector3 &B)
{
  return 
    mmVector3(A.y * B.z - A.z * B.y,
	    A.z * B.x - A.x * B.z,
	    A.x * B.y - A.y * B.x);
}

inline mmVector3
Normalize(const mmVector3 &A)
{
  return A * (1.0f/Magnitude(A));
}

inline float
Magnitude(const mmVector3 &v) 
{
  return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

inline float
SqrMagnitude(const mmVector3 &v) 
{
  return v.x * v.x + v.y * v.y + v.z * v.z;
}

#endif //  __VECTOR3_H

