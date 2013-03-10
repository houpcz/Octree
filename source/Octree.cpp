#include "Octree.h"
#include <stack>

Octree::Octree(vector<Primitive *> & _primitive, bool _faster) 
{
	faster = _faster;
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
	if(faster)
	{
		set<int> b = GetCrossedTrianglesSlow(ray);
		//set<int> a = GetCrossedTrianglesFaster(ray);

		//if(a.size() != b.size())
		//{
		//	int stop = 1;
		//}
		return b;
	}
	else
	{
		return GetCrossedTrianglesSlow(ray);
	}
}

set<int> Octree::GetCrossedTrianglesFaster(Ray & r)
{
	Ray ray = r;
	AxisAlignedBox3 box = root->Box();
	crossedTriangleIDs.clear();
	float sizeX = box.Diagonal().x;
	float sizeY = box.Diagonal().y;
	float sizeZ = box.Diagonal().z;

	a = 0;
	if(ray.direction.x < 0.0)
	{
		ray.origin.x = sizeX - ray.origin.x;
		ray.direction.x = -ray.direction.x;
		a |= 4;
	}
	if(ray.direction.y < 0.0)
	{
		ray.origin.y = sizeY - ray.origin.y;
		ray.direction.y = -ray.direction.y;
		a |= 2;
	}
	if(ray.direction.z < 0.0)
	{
		ray.origin.z = sizeZ - ray.origin.z;
		ray.direction.z = -ray.direction.z;
		a |= 1;
	}

	float tx0 = (box.min.x - ray.origin.x) / ray.direction.x;
	float tx1 = (box.max.x - ray.origin.x) / ray.direction.x;
	float ty0 = (box.min.y - ray.origin.y) / ray.direction.y;
	float ty1 = (box.max.y - ray.origin.y) / ray.direction.y;
	float tz0 = (box.min.z - ray.origin.z) / ray.direction.z;
	float tz1 = (box.max.z - ray.origin.z) / ray.direction.z;

	if(Max(tx0, ty0, tz0) < Min(tx1, ty1, tz1))
	{
		float tMin, tMax;
		bool intersection = box.ComputeMinMaxT(r, tMin, tMax);
		CrossSubTree(tx0, ty0, tz0, tx1, ty1, tz1, root);
	}

	return crossedTriangleIDs;
}

struct CrossSubTreeStruct {
	float tx0, ty0, tz0;
	float tx1, ty1, tz1;
	OctNode * n;

	CrossSubTreeStruct(float _tx0, float _ty0, float _tz0, float _tx1, float _ty1, float _tz1, OctNode * _n)
	{
		tx0 = _tx0; ty0 = _ty0; tz0 = _ty0;
		tx1 = _tx1; ty1 = _ty1; tz1 = _ty1;
		n = _n;
	}
};

void Octree::CrossSubTree(float tx0, float ty0, float tz0, float tx1, float ty1, float tz1, OctNode * n)
{
	stack<CrossSubTreeStruct> stackSubTree;
	stackSubTree.push(CrossSubTreeStruct(tx0, ty0, tz0, tx1, ty1, tz1, n));

	while(!stackSubTree.empty())
	{
		CrossSubTreeStruct top = stackSubTree.top();
		stackSubTree.pop();

		if(top.tx1 < 0.0f || top.ty1 < 0.0f || top.tz1 < 0.0f)
			return;

		if(top.n->IsLeaf())
		{
			vector<int> ids = top.n->PrimitiveID();
			crossedTriangleIDs.insert(ids.begin(), ids.end());
			return;
		}

		float txm = 0.5f * (top.tx1 + top.tx0);
		float tym = 0.5f * (top.ty1 + top.ty0);
		float tzm = 0.5f * (top.tz1 + top.tz0);
	
		int currNode = FirstNode(top.tx0, top.ty0, top.tz0, txm, tym, tzm);

		do {
			switch(currNode)
			{
				case 0 :
					stackSubTree.push(CrossSubTreeStruct(top.tx0, top.ty0, top.tz0, txm, tym, tzm, top.n->Child(a)));
					currNode = NewNode(txm, 4, tym, 2, tzm, 1);
					break;
				case 1 :
					stackSubTree.push(CrossSubTreeStruct(top.tx0, top.ty0, tzm, txm, tym, top.tz1, top.n->Child(1 ^ a)));
					currNode = NewNode(txm, 5, tym, 3, top.tz1, 8);
					break;
				case 2 :
					stackSubTree.push(CrossSubTreeStruct(top.tx0, tym, top.tz0, txm, top.ty1, tzm, top.n->Child(2 ^ a)));
					currNode = NewNode(txm, 6, top.ty1, 8, tzm, 3);
					break;
				case 3 :
					stackSubTree.push(CrossSubTreeStruct(top.tx0, tym, tzm, txm, top.ty1, top.tz1, top.n->Child(3 ^ a)));
					currNode = NewNode(txm, 7, top.ty1, 8, top.tz1, 8);
					break;
				case 4 :
					stackSubTree.push(CrossSubTreeStruct(txm, top.ty0, top.tz0, top.tx1, tym, tzm, top.n->Child(4 ^ a)));
					currNode = NewNode(top.tx1, 8, tym, 6, tzm, 5);
					break;
				case 5 :
					stackSubTree.push(CrossSubTreeStruct(txm, top.ty0, tzm, top.tx1, tym, top.tz1, top.n->Child(5 ^ a)));
					currNode = NewNode(top.tx1, 8, tym, 7, top.tz1, 8);
					break;
				case 6 :
					stackSubTree.push(CrossSubTreeStruct(txm, tym, top.tz0, top.tx1, top.ty1, tzm, top.n->Child(6 ^ a)));
					currNode = NewNode(top.tx1, 8, top.ty1, 8, tzm, 7);
					break;
				case 7 :
					stackSubTree.push(CrossSubTreeStruct(txm, tym, tzm, top.tx1, top.ty1, top.tz1, top.n->Child(7 ^ a)));
					currNode = 8;
					break;
				default :
					cerr << "Error" << endl;
					break;
			}
		} while (currNode < 8);
	}
}

int Octree::FirstNode(float tx0, float ty0, float tz0, float txm, float tym, float tzm)
{
	int firstNode = 0;

	if(tx0 < ty0)
	{
		if(tx0 < tz0) // YZ plane
		{
			firstNode |= (tym < tx0) << 1;
			firstNode |= (tzm < tx0) << 2;
		} else { // XY
			firstNode |= (txm < tz0) << 0;
			firstNode |= (tym < tz0) << 1;
		}
	} else {
		if(ty0 < tz0) // XZ plane
		{
			firstNode |= (txm < ty0) << 0;
			firstNode |= (tzm < ty0) << 2;
		} else { // XY
			firstNode |= (txm < tz0) << 0;
			firstNode |= (tym < tz0) << 1;
		}
	}

	return firstNode;
}
int Octree::NewNode(float x, int xID, float y, int yID, float z, int zID)
{
	if(x < y)
	{
		if(x < z)
			return xID;
		else
			return zID;
	} else {
		if(y < z)
			return yID;
		else
			return zID;
	}
}
float Octree::Min(float a, float b, float c)
{
	if(a < b)
	{
		if(a < c)
			return a;
		else 
			return c;
	} else {
		if(b < c)
			return b;
		else
			return c;
	}
}
float Octree::Max(float a, float b, float c)
{
	if(a > b)
	{
		if(a > c)
			return a;
		else 
			return c;
	} else {
		if(b > c)
			return b;
		else
			return c;
	}
}

set<int> Octree::GetCrossedTrianglesSlow(Ray & ray)
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