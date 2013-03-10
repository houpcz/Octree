
#ifndef _OCTNODE_H_
#define _OCTNODE_H_

#include <vector>
#include <set>
#include "AxisAlignedBox3.h"

#ifndef NULL
#define NULL 0
#endif

using namespace std;
class Octree;

class OctNode
{
private :
	Octree * tree;
	AxisAlignedBox3 box;

	static const int CHILD_NUMBER = 8;
	OctNode * parent;
	OctNode ** children;

	vector<int> primitiveID;
public:
	OctNode(vector<int> parentPrimitive, AxisAlignedBox3 _box, Octree * _tree, OctNode * _parent);
	set<int> GetCrossedTriangles(Ray & ray);
	vector<int> PrimitiveID() { return primitiveID; };
	AxisAlignedBox3 Box() { return box; };
	OctNode * Child(int childNumber) { 
		return children[childNumber]; 
	};
	bool IsLeaf() { return children == NULL; };
	~OctNode(void);
};

#endif

