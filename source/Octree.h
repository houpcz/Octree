#ifndef _OCTREE_H_
#define _OCTREE_H_

#include <vector>
#include <set>
#include "OctNode.h"
#include "sglprimitive.h"

using namespace std;

class Octree
{
private :
	OctNode * root;
	vector<sglTriangle *> primitive;
	vector<OctNode *> leaf;

	set<int> crossedTriangleIDs;
	int a;
	bool faster;
public:
	Octree(vector<Primitive *> & _primitive, bool _faster);
	~Octree(void);

	set<int> GetCrossedTriangles(Ray & ray);
	set<int> GetCrossedTrianglesSlow(Ray & ray);
	set<int> GetCrossedTrianglesFaster(Ray & r);
	void CrossSubTree(float tx0, float ty0, float tz0, float tx1, float ty1, float tz1, OctNode * n);
	int FirstNode(float tx0, float ty0, float tz0, float txm, float tym, float tzm);
	int NewNode(float x, int xID, float y, int yID, float z, int zID);
	float Min(float a, float b, float c);
	float Max(float a, float b, float c);

	sglTriangle * GetTriangle(int triangleID);
	int MaxPrimitive() { return 100; };
	float MinVolume() { return 0.0001f; };
	void AddLeaf(OctNode * node);
	bool CheckOctree();
	OctNode * GetLeaf(int leafID) { return leaf[leafID]; };
};

#endif

