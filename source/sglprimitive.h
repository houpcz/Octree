//---------------------------------------------------------------------------
// sglprimitive.h
// Implementation of the SGL (Simple Graphics Library)
// Date:  2011/11/04
// Author: Lukas Beran CTU Prague
//---------------------------------------------------------------------------

#ifndef _SGLPRIMITIVE_H_
#define _SGLPRIMITIVE_H_

#include <cstdlib>
#include "sglStructures.h"
#include "sgl.h"

class Ray
{
	public :
		Vector3 origin;
		Vector3 direction;
};

class Material
{
	public: // data
			Color color;
			float kd, ks, shine, T, ior;
	public: // methods
			Material(const Color &xcol, float xkd, float xks, float xshine, float xT, float xior):
					color(xcol), kd(xkd), ks(xks), shine(xshine), T(xT), ior(xior) 
			{
			};
};

class EmissiveMaterial
{
	public :
		Color col;
		Vector3 params;
		EmissiveMaterial(Color n_col, Vector3 n_params) : col(n_col), params(n_params)
		{
		}
};

class Primitive
{
	public :
		Material * material;
	public :
		Primitive(Material * n_material) : material(n_material)
		{
		}
		virtual bool intersect(const Ray & ray, float * tHit)
		{
			return false;
		}
		virtual void rasterize() = 0;
		virtual Vector3 normal(Vector3 * point) = 0;
};

class sglTriangle : public Primitive
{
	public :
		Vector3 point1, point2, point3;
		Vector3 edge1, edge2;
		Vector3 normalVector;
	public :
		sglTriangle(Vector3 n_point1, Vector3 n_point2, Vector3 n_point3, Material * n_material) : Primitive(n_material), point1(n_point1), point2(n_point2), point3(n_point3)
		{
			edge1 = point2 - point1;
			edge2 = point3 - point1;
			normalVector = CrossProd(edge1, edge2);
			normalVector.Normalize();
		}
		
		bool intersect(const Ray & ray, float * tHit)
		{
			Vector3 dir = ray.direction;
			//if(normalVector.z > 0.9999999999 && normalVector.z < 1.00000001)
			//	return false;
			if(DotProd(dir, normalVector) > 0)
				return false;
			// Compute $\VEC{s}_1$
			// Get triangle vertices in _p1_, _p2_, and _p3_
			
			Vector3 s1 = CrossProd(ray.direction, edge2);
			float divisor = DotProd(s1, edge1);
			if (divisor == 0.)
				return false;
			float invDivisor = 1.f / divisor;
			// Compute first barycentric coordinate
			Vector3 d = ray.origin - point1;
			float b1 = DotProd(d, s1) * invDivisor;
			if (b1 < 0. || b1 > 1.)
				return false;
			// Compute second barycentric coordinate
			Vector3 s2 = CrossProd(d, edge1);
			float b2 = DotProd(ray.direction, s2) * invDivisor;
			if (b2 < 0. || b1 + b2 > 1.)
				return false;
			// Compute _t_ to intersection point
			float t = DotProd(edge2, s2) * invDivisor;
			if (t < 0.001f)// || t > 1000.0f)
				return false;
			*tHit = t;
			return true;
		}

		Vector3 normal(Vector3 * point)
		{
			return normalVector;
		}

		void rasterize()
		{
			sglColor3f(material->color.r, material->color.g, material->color.b);
			sglBegin(SGL_POLYGON);
				sglVertex3f(point1.x, point1.y, point1.z);
				sglVertex3f(point2.x, point2.y, point2.z);
				sglVertex3f(point3.x, point3.y, point3.z);
			sglEnd();
		}
};

class Sphere : public Primitive
{
	private :
		Vector3 pos;
		float radius;
	public :
		Sphere(Vector3 n_pos, float n_radius, Material * n_material) : Primitive(n_material), pos(n_pos), radius(n_radius)
		{	
		}
		bool intersect(const Ray & ray, float * tHit)
		{
			const Vector3 dst = ray.origin - pos;
			const float b = DotProd(dst, ray.direction);
			const float c = DotProd(dst, dst) - radius*radius;
			const float d = b*b - c;
	
			if(d > 0) {
			  *tHit = -b - sqrtf(d);
			  if (*tHit < 0.0f)
				*tHit = -b + sqrtf(d);

			  if(*tHit < 0.01f)
				  return false;

			  return true;
			}
			return false;
		}

		Vector3 normal(Vector3 * point)
		{
			Vector3 result = *point-pos;
			return result / radius;
		}

		void rasterize()
		{
			sglPointSize(10.0f);
			sglColor3f(material->color.r, material->color.g, material->color.b);
			sglBegin(SGL_POINTS);
				sglVertex3f(pos.x, pos.y, pos.z);
			sglEnd();
		}
};

class Light
{
	public :
		Color color;
		Vector3 pos;
		int sampleAmount;
	public :
		Light(Color n_color, Vector3 n_pos, int sampleAmount) : color(n_color), pos(n_pos), sampleAmount(sampleAmount)
		{
		}
		virtual bool intersect(const Ray & ray)=0;
		virtual void generateSample(const Vector3 & a)=0;
};


class PointLight : public Light
{
	public :
		PointLight(Color n_color, Vector3 n_pos) : Light(n_color, n_pos, 1)
		{
		}

		// do nothing
		void generateSample(const Vector3 & a)
		{
		}

		bool intersect(const Ray & ray)
		{
			return false;
		}
};

class AreaLight : public Light
{
	private :
		EmissiveMaterial * material;
		sglTriangle * triangle;
		float area;
		Ray shadow;
	public :
		AreaLight(sglTriangle * n_triangle, EmissiveMaterial * n_material) : Light(Color(), Vector3(), 64), material(n_material), triangle(n_triangle)
		{
			Vector3 cross = CrossProd(triangle->edge1, triangle->edge2);
			area = cross.Length() / 2;
		}
		virtual ~AreaLight() {
			delete triangle;
		}
		void generateSample(const Vector3 & a)
		{
			float rand1 = rand() / (float) RAND_MAX;
			float rand2 = (rand() / (float) RAND_MAX) * (1- rand1);

			pos = triangle->point1 + triangle->edge1 * rand1 + triangle->edge2 * rand2;
			shadow.direction = pos - a;
			float d = shadow.direction.Length();
			shadow.direction.Normalize();
			float cosFi = DotProd(triangle->normal(NULL), -shadow.direction);
			if(cosFi > 0.0)
				color = material->col * (cosFi * (area/sampleAmount)/(material->params.x + material->params.y*d + material->params.z*d*d));
			else
			{
				color.r = 0.0f;
				color.g = 0.0f;
				color.b = 0.0f;
			}
		} 

		bool intersect(const Ray & ray)
		{
			float f;
			bool result = triangle->intersect(ray, &f);
			if(result)
				color = material->col;
			return result;
		}
};



#endif