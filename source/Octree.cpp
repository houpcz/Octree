#include "Octree.h"


Octree::Octree(vector<Primitive *> & _primitive)
{
	vector<int> allIDs;
	AxisAlignedBox3 box;
	box.Reset();
	sglTriangle * temp;
	for(int loop1 = 0; loop1 < _primitive.size(); ++loop1)
	{
		temp = dynamic_cast<sglTriangle *>(_primitive[loop1]);
		primitive.push_back(temp);
		box.Include(temp->point1);
		box.Include(temp->point2);
		box.Include(temp->point3);
		allIDs.push_back(loop1);
	}

	root = new OctNode(allIDs, box, this, NULL);

	cout << "Oct leafs " << leaf.size() << endl;

	CheckOctree();
}

void Octree::AddLeaf(OctNode * node)
{
	leaf.push_back(node);
}

bool Octree::CheckOctree()
{
	set<int> triangle;
	vector<int> ids;
	for(int loop1 = 0; loop1 < leaf.size(); loop1++)
	{
		ids = leaf[loop1]->PrimitiveID();
		triangle.insert(ids.begin(), ids.end());
	}

	if(primitive.size() != triangle.size())
	{
		cout << primitive.size() << " " << triangle.size() << endl;
	}

	return primitive.size() == triangle.size();
}

set<int> Octree::GetCrossedTriangles(Ray & ray)
{
	return root->GetCrossedTriangles(ray);
}

Octree::~Octree(void)
{
	delete root;
}

sglTriangle * Octree::GetTriangle(int triangleID)
{
	return primitive[triangleID];
}