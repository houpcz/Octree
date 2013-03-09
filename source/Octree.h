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
public:
	Octree(vector<Primitive *> & _primitive);
	~Octree(void);

	set<int> GetCrossedTriangles(Ray & ray);

	sglTriangle * GetTriangle(int triangleID);
	int MaxPrimitive() { return 150; };
	float MinVolume() { return 0.01f; };
	void AddLeaf(OctNode * node);
	bool CheckOctree();
};

#endif

