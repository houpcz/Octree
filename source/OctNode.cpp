#include "OctNode.h"
#include "Octree.h"


OctNode::OctNode(vector<int> parentPrimitive, AxisAlignedBox3 _box, Octree * _tree, OctNode * _parent)
{
	tree = _tree;
	box = _box;
	parent = _parent;

	for(int loop1 = 0; loop1 < parentPrimitive.size(); ++loop1)
	{
		int id = parentPrimitive[loop1];
		if(box.CollisionTriangle(tree->GetTriangle(id)))
			primitiveID.push_back(id);
	}

	if(primitiveID.size() > tree->MaxPrimitive() && box.Volume() > tree->MinVolume())
	{
		children = new OctNode*[CHILD_NUMBER];
		for(int loop1 = 0; loop1 < CHILD_NUMBER; loop1++)
		{
			children[loop1] = new OctNode(primitiveID, box.GetOctant(loop1), tree, this);
		}
		primitiveID.clear();
	} else {
		children = NULL;
		tree->AddLeaf(this);
	}
}

set<int> OctNode::GetCrossedTriangles(Ray & ray)
{
	float tMin, tMax;
	set<int> result;
	if(box.ComputeMinMaxT(ray, tMin, tMax))
	{
		if(children != NULL)
		{
			set<int> temp;
			for(int loop1 = 0; loop1 < CHILD_NUMBER; loop1++)
			{
				temp = children[loop1]->GetCrossedTriangles(ray);
				result.insert(temp.begin(), temp.end());
			}
		} else {
			result.insert(primitiveID.begin(), primitiveID.end());
		}
	}
	return result;
}

OctNode::~OctNode(void)
{
	if(children != NULL)
	{
		for(int loop1 = 0; loop1 < CHILD_NUMBER; loop1++)
		{
			delete children[loop1];
		}
		delete [] children;
	}
}
