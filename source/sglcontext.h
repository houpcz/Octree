//---------------------------------------------------------------------------
// sglcontext.h
// Implementation of the SGL (Simple Graphics Library)
// Date:  2011/11/04
// Author: Lukas Beran CTU Prague
//---------------------------------------------------------------------------

#ifndef _SGLCONTEXT_H_
#define _SGLCONTEXT_H_

#include "sglprimitive.h"
#include "sgl.h"
#include "Octree.h"
#include <vector>
#include <stack>

using namespace std;

void setErrCode(sglEErrorCode c);

// Nasobeni matic, vysledek se ulozi do outputmatrix
void MultMatrix(float * outputmatrix, const float *matrix1, const float *matrix2);
// Nasobeni vektoru matici
void MultVector(Vector3 * v, float * matrix);
// Vynasobi vektor inverzi viewportu
void InversViewportVector(Vector3 *v);

// Trida pro udrzeni kontextu
class SglContext {
	public :
		Color clearColor;
		Color currentColor;
		float pointSize;
		Color * colorBuffer;
		float * depthBuffer;
		int colorBufferSize;

		sglEElementType currentElemType;
		sglEAreaMode areaMode;

		float matrix[2][16];		// projekcni a modelview matice
		float modelviewprojection[16];	// vynasobena projekcni a modelview matice
		float viewportMatrix[16];
		int activeMatrix;		// ktera z matic je aktivni
		stack<float *> matrixStack; // zasobnik na matice

		int viewportX;
		int viewportY;
		int viewportWidth;
		int viewportHeight;

		int width, height;
		int contextID;
		int flagbits;	// | a & s SGL_DEPTH_TEST

		bool waitingForSglEnd;	// uklada, jestli bylo zavolano sglBegin() a jeste nebylo zavolano sglEnd

		Vector4 * vertexes;			// Zachycuje body utvaru, ktere se budou vykreslovat
		unsigned int vertexesMax;		// Kolik se do pole vejde bodu? Pri pridani bodu, ktery se do pole nevejde, musi se pole zvetsit.
		unsigned int vertexesSize;	// Kam se prida aktualne vkladany bod. Vyjadruje i pocet bodu aktualne zadanych pomoci sglVertex
	
		//-- Raytracer --
		bool sglBeginScene;			// bylo zavolano sglBeginScene?
		vector<Material *> material;
		vector<EmissiveMaterial *> emissiveMaterial;
		vector<Primitive *> primitive;
		vector<Light *> light;
		Octree * octree;

		// Environment map
		int mapWidth;
		int mapHeight;
		float * mapColors;
	private :
		Primitive * FindNearestHittedPrimitive(Ray & ray, float & tHit);

		// Kresli usecky bez kontroly indexu do colorBufferu
		void DrawLineFaster(float fx1, float fy1, float fx2, float fy2);
		// Pomalejsi, kontroluje, jestli jednotlive body jsou uvnitr viewportu
		void DrawLineSlower(float fx1, float fy1, float fx2, float fy2);
	public :
		SglContext(int n_width, int n_height, int ID);
		~SglContext();

		// Kresleni polygonu, pixelu, usecek a kruznice
		// Zde kvuli rychlosti pristupu k colorbufferu
		// (napr. elipsa zde neni, protoze se kresli pomoci usecek)
		void FillPolygon();
		void FillTriangle();
		void FillCircle(int xs, int ys, int r);
		// Vola se, kdyz neni zapnuty depthbuffer
		inline void DrawPixel(int x, int y);
		// Vola se, kdyz je zapntuty depthBuffer
		inline void DrawPixel(int x, int y, float z);
		void DrawLine(float fx1, float fy1, float fx2, float fy2);
		// Vola se, kdyz neni zapnuty depthbuffer
		void DrawHorizontalLine(float fx1, float fx2, float fy);
		// Vola se, kdyz je zapntuty depthBuffer
		void DrawHorizontalLine(float fx1, float fx2, float fz1, float fz2, float fy);
		void DrawCircle(int xs, int ys, int r);

		// Vyuzito v sglEnd
		void DrawPoints();
		void DrawLines();
		void DrawLineStrip();
		void DrawLineLoop();

		// -- Raytracing --
		void makeOctree(bool faster);
		// Procisti scenu
		void clearScene();
		// vyrenderuje nactenou scenu
		void raytrace();
		// kontrola zastineni bodu na objektech
		inline bool lightObstacle(Ray * r, Vector3 *lightPos);
		// nageneruje paprsek, ktery prochazi bodem x, y obrazovkou
		inline Ray makeRay(float x, float y, float * matrixInvert);
		// sleduje paprsek rekurzivne az do hloubky nula (primarni se vola s hloubkou 8)
		inline Color traceRay(Ray ray, int depth);
		void addSphere(const float x,
			   const float y,
			   const float z,
			   const float radius)
		{
			primitive.push_back(new Sphere(Vector3(x, y, z), radius, material[material.size() - 1]));
		};

		void addTriangles()
		{
			if(emissiveMaterial.size() == 0)
			{
				switch(vertexesSize)
				{
					case 4 :
						primitive.push_back(new sglTriangle(vertexes[2], vertexes[3], vertexes[0], material[material.size() - 1]));
						// neni break schvalne
					case 3 :
						primitive.push_back(new sglTriangle(vertexes[0], vertexes[1], vertexes[2], material[material.size() - 1]));
						break;
				}
			} else {
				light.push_back(new AreaLight(new sglTriangle(vertexes[0], vertexes[1], vertexes[2], material[material.size() - 1]), emissiveMaterial[emissiveMaterial.size() - 1]));
			}
		}

		void addMaterial(const float r,
				 const float g,
				 const float b,
				 const float kd,
				 const float ks,
				 const float shine,
				 const float T,
				 const float ior) 
		{
			material.push_back(new Material(Color(r, g, b) , kd, ks, shine, T, ior));
		};

		void addEmissiveMaterial(const float r,
								 const float g,
								 const float b,
								 const float c0,
								 const float c1,
								 const float c2
								 )
		{
			emissiveMaterial.push_back(new EmissiveMaterial(Color(r, g, b) , Vector3(c0, c1, c2)));
		};

		void addPointLight(const float x,
				   const float y,
				   const float z,
				   const float r,
				   const float g,
				   const float b) 
		{
			light.push_back(new PointLight(Color(r, g, b), Vector3(x, y, z)));
		};

		void addEnvironmentMap(const int width,
					   const int height,
					   float *texels)
		{
			mapWidth = width;
			mapHeight = height;
			mapColors = texels;
		}

		void rasterize()
		{
			sglEnable(SGL_DEPTH_TEST);
			for(int loop1 = 0; loop1 < 1 && loop1 < primitive.size(); loop1++)
			{
				primitive[loop1]->rasterize();
			}
			sglDisable(SGL_DEPTH_TEST);
		}

		void rasterizeGL()
		{
			glEnable(GL_LIGHTING);
			glEnable(GL_LIGHT0);
			glShadeModel( GL_SMOOTH );

			glBegin(GL_TRIANGLES);
			for(int loop1 = 0; loop1 < primitive.size(); loop1++)
			{
				primitive[loop1]->rasterizeGL();
			}
			glEnd();
		}
};

// FILLING POLYGONS STRUCTURES
struct Edge
{
	int dy;
	float x;
	float dx;
	float z;
	float dz;
	Edge * edge;

	Edge(int n_dy, float n_x, float n_dx, float n_z, float n_dz, Edge * n_edge) : dy(n_dy), x(n_x), dx(n_dx), z(n_z), dz(n_dz), edge(n_edge)
	{
	}
};

class ScanLineAlg
{
	private :
		int sizeTH;
		Edge ** TH;
		vector<Edge *> SAH;
		int yMinLine, yMaxLine;
	public :
		ScanLineAlg()
		{
			TH = NULL;
			sizeTH = 0;
		}
		~ScanLineAlg()
		{
			if(TH != NULL)
				delete [] TH;
			
			for(unsigned int loop1 = 0; loop1 < SAH.size(); loop1++)
			{
				delete SAH[loop1];
			}
			SAH.clear();
		}

		void init(Vector4 * vertexes, int vertexesSize, int viewportHeight);
		void fill(SglContext * cc);
};


// <- FILLING POLYGONS STRUCTURES

class ContextManager {
	private :
		ContextManager();
		~ContextManager();
		static ContextManager *inst;

		vector<SglContext *> contexts;
		int currentContext;
	public :
		ScanLineAlg scanLineAlg;

		static ContextManager * Inst();
		static void ClearContextManager() { delete inst; inst = NULL; }
		int CreateContext(int width, int height);

		void DestroyContext(unsigned int context) {
			if(context >= 0 && context < contexts.size())
			{
				if(contexts[context])
				{
					delete contexts[context];
					contexts[context] = NULL;
				}
			} else setErrCode(SGL_INVALID_VALUE);
		}
		
		SglContext * CC() { 
			if(currentContext < 0)
			{
				setErrCode(SGL_INVALID_OPERATION);
				return NULL;
			}
			return contexts[currentContext]; 
		} // current context

		bool BetweenBeginAndEnd()
		{
			return CC()->waitingForSglEnd;
		}

		bool BetweenBeginAndEndScene()
		{
			return CC()->sglBeginScene;
		}

		void SetCurrentContext(int context) { 
			if(context >= 0 && context < (signed) contexts.size())
			{
				if(contexts[context])
				{
					currentContext = context; 
				} else setErrCode(SGL_INVALID_VALUE);  
			}
		}
		int GetCurrentContext() {
			if(currentContext < 0)
				setErrCode(SGL_INVALID_OPERATION);

			return currentContext; 
		}
};

#endif