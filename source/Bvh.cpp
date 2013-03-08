// ===================================================================
// $Id: Bvh.cpp,v 1.2 2007/03/02 12:41:24 bittner Exp $

// C++ standard headers
#include <queue>
#include <stack>
// local headers
#include "Environment.h"
#include "Scene.h"
#include "Bvh.h"
#include "Ray.h"


// Constructor
Bvh::Bvh(Scene &scene)
{
  triangles.resize(scene.triangles.size());
  for (int i=0; i < scene.triangles.size(); i++)
    triangles[i] = scene.triangles[i];
  
  BvhLeaf *leaf = new BvhLeaf;
  leaf->depth = 0;
  leaf->first = 0;
  leaf->last = scene.triangles.size()-1;
  leaf->box = scene.box;
  root = leaf;
  numberOfNodes = 1;

  Environment::GetSingleton()->GetIntValue("Bvh.maxTriangles",
					   envMaxTriangles);
  string s;
  Environment::GetSingleton()->GetStringValue("Bvh.splitType", s);

  if (strcmp(s.c_str(),"spatial_median") == 0)
    envSplitType = SPATIAL_MEDIAN;
  else
    if (strcmp(s.c_str(),"object_median") == 0)
      envSplitType = OBJECT_MEDIAN;
    else
      if (strcmp(s.c_str(),"sah") == 0)
		envSplitType = SAH;
  
  return;
}

void
Bvh::Construct()
{
  long t1 = GetTime();
  Debug << "Info: Constructing BVH"<<endl;
  cout << "Info: Constructing BVH"<<endl;
  root = SubdivideLeaf((BvhLeaf *)root, 0);
  long t2 = GetTime();
  cout << "Info: Bvh done. in " << TimeDiff(t1, t2)*1e-3
       << " seconds." << endl;
  cout << "Info: Number Of BVH Nodes = "
       << numberOfNodes << endl;
  float iSA, lSA, iVol, lVol;
  SumSurfaceAreas(iSA, lSA, iVol, lVol);
  cout<<"Info: interiorNodesSA ="<<iSA/root->box.SurfaceArea()<<endl;
  cout<<"Info: leafNodesSA ="<<lSA/root->box.SurfaceArea()<<endl;
  cout<<"Info: interiorNodesVolume ="<<iVol/root->box.Volume()<<endl;
  cout<<"Info: leafNodesVolume ="<<lVol/root->box.Volume()<<endl;

  return;
}

float
Bvh::EvaluateSahCost(BvhLeaf *leaf, const int axis, const float position)
{
  // count triangles on the left/right
  int left = 0;
  int right = 0;
  AxisAlignedBox3 leftBox, rightBox;
  leftBox.Reset();
  rightBox.Reset();

  int candidates = Max(50, leaf->Count()/20);
  float finc = leaf->Count()/(float)candidates;
  int i = leaf->first;
  float fi = leaf->first + 0.5f;
  for (; i <= leaf->last; ) {
	if ( triangles[i]->GetCenter(axis) < position ) {
	  left++;
	  // update the box
	  leftBox.Include(triangles[i]->GetBox());
	} else {
	  right++;
	  rightBox.Include(triangles[i]->GetBox());
	}
	fi+=finc;
	i = (int)fi;
  }

  float bW = 1.0f;
  float leftRatio = left/(float)leaf->Count();
  float rightRatio = right/(float)leaf->Count();

  float saLeft = 0.0f;
  float saRight = 0.0f;

  // not a valid split
  if (!left || !right)
	return MAX_FLOAT;
  
  saLeft = leftBox.SurfaceArea();
  saRight = rightBox.SurfaceArea();
	
  
  return
	saLeft*((1.0f - bW) + bW*leftRatio)
	+ saRight*((1.0f - bW) + bW*rightRatio);
}

float
Bvh::SelectPlaneSah(BvhLeaf *leaf, int &axis, float &minCost)
{
  minCost = MAX_FLOAT;
  float bestPos = MAX_FLOAT;
  int bestAxis = 0;
  
  //  cout<<"Evaluating axis..."<<endl<<flush;

  const int initialPlanes = 3;
  // initiate the costs
  for (axis = 0; axis < 3; axis++) {
	float size = leaf->box.max[axis] - leaf->box.min[axis];
	for (int i=0; i < initialPlanes; i++) {
	  float pos = leaf->box.min[axis] + (i+1)*size/(initialPlanes+1);
	  float cost = EvaluateSahCost(leaf, axis, pos);
	  if (cost < minCost) {
		minCost = cost;
		bestPos = pos;
		bestAxis = axis;
	  }
	}
  }
  
  axis = bestAxis;
  //  cout<<axis<<endl<<flush;
  const float shrink = 0.5f;
  // now hierarchically refine the initial interval
  float size = shrink*
	(leaf->box.max[axis] - leaf->box.min[axis])/(initialPlanes+1);
  
  const int steps = 4;
  float cost;
  for (int i=0; i < steps; i++) {
	const float left = bestPos - size;
	const float right = bestPos + size;
	
	cost = EvaluateSahCost(leaf, axis, left);
	if (cost < minCost) {
	  minCost = cost;
	  bestPos = left;
	}
	
	cost = EvaluateSahCost(leaf, axis, right);
	if (cost < minCost) {
	  minCost = cost;
	  bestPos = right;
	}
	size = shrink*size;
  }
  
  
  
  //	const int maxPlanes = 16;
//   for (int i=0; i < maxPlanes; i++) {
// 	float pos = leaf->box.min[axis] + (i+1)*size/(maxPlanes+1);
// 	float cost = EvaluateSahCost(leaf, axis, pos);
// 	if (cost < minCost) {
// 	  minCost = cost;
// 	  bestPos = pos;
// 	}
//  cout<<i<<" count="<<leaf->Count()<<" cost="<<cost<<endl;
//   }

//  cout<<axis<<" "<<bestPos<<" "<<minCost<<endl<<flush;
  return bestPos;
}

void
Bvh::SumSurfaceAreas(float &interiorSA,
					 float &leafSA,
					 float &interiorVol,
					 float &leafVol
					 )
{
  stack<BvhNode *> nodeStack;

  interiorSA = 0.0f;
  leafSA = 0.0f;
  interiorVol = 0.0f;
  leafVol = 0.0f;
  
  nodeStack.push(root);

  while (!nodeStack.empty()) {
	BvhNode *node = nodeStack.top();
	nodeStack.pop();

	if (node->IsLeaf()) {
	  leafSA += node->box.SurfaceArea();
	  leafVol += node->box.Volume();
	} else {
	  interiorSA += node->box.SurfaceArea();
	  interiorVol += node->box.Volume();
	  BvhInterior *interior = (BvhInterior *)node;
	  nodeStack.push(interior->back);
	  nodeStack.push(interior->front);
	}
  }
}

int
Bvh::SortTriangles(BvhLeaf *leaf, const int axis, const float position)
{
  int i = leaf->first;
  int j = leaf->last;
  
  while(1) {
    while( triangles[i]->GetCenter(axis) < position ) 
      i++;
	
    while( position < triangles[j]->GetCenter(axis) )
      j--;
    
    if (i < j) {
      swap(triangles[i], triangles[j]);
      i++;
      j--;
    }
    else
      break;
  }
  
  return j;
}

int
Bvh::SortTrianglesSpatialMedian(BvhLeaf *leaf, const int axis)
{
  // spatial median
  float x = leaf->box.Center(axis);
  return SortTriangles(leaf, axis, x);
}


int
Bvh::SortTrianglesObjectMedian(BvhLeaf *leaf, const int axis)
{
  // Now distribute the objects into the subnodes
  // Use QuickMedian sort
  int l = leaf->first;
  int r = leaf->last;
  int k = (l + r)/2;

  
  float min, max;
  
  while (l < r) {
    int i = l;
    int j = r;
	
    // get some estimation of the pivot
    float x = triangles[(l + r)/2]->GetCenter(axis);
    
    while(1) {
      while( triangles[i]->GetCenter(axis) < x ) 
		i++;
      
      while( x < triangles[j]->GetCenter(axis) )
		j--;
	  
      if (i <= j) {
		swap(triangles[i], triangles[j]);
		i++;
		j--;
      }
      else
		break;
    }
	
    // now check the extends
    
    if (i >= k )
      r = j;
    else
      l = i;
  }
  
  return k;
}

BvhNode *
Bvh::SubdivideLeaf(BvhLeaf *leaf, const int dummyAxis)
{
  if (leaf->Count() < envMaxTriangles)
    return leaf;

  int axis = leaf->box.Diagonal().MajorAxis();
  
  //int axis = dummyAxis; //leaf->box.Diagonal().MajorAxis();

  // position of the split in the partailly sorted array of triangles
  // corresponding to this leaf
  int split;
  
  //  bool objectMedian = false;
  //  bool spatialMedian = true;

  // Spatial median subdivision
  switch (envSplitType) {

  case SPATIAL_MEDIAN:
    split = SortTrianglesSpatialMedian(leaf, axis);
	if ( 
		((split - leaf->first) < leaf->Count()/20) ||
		((leaf->last - split) < leaf->Count()/20) ) {
	  split = SortTrianglesObjectMedian(leaf, axis);
	}
	break;
	
  // Object median subdivision: approximately the same number
  // of objects on the left of the the splitting point.
	//	if ( (envSplitType == OBJECT_MEDIAN) ||
	//       ((split - leaf->first) < leaf->Count()/10) ||
	//       ((leaf->last - split) < leaf->Count()/10) ) {
  case OBJECT_MEDIAN:
	split = SortTrianglesObjectMedian(leaf, axis);
	break;

  case SAH: {
	//    cerr << "Error: SAH Bvh split not yet implemented!\n";
	float cost;
	float pos = SelectPlaneSah(leaf, axis, cost);
	if (pos != MAX_FLOAT)
	  split = SortTriangles(leaf, axis, pos);
	else
	  split = SortTrianglesObjectMedian(leaf, axis);
	break;
  }
  }
	

  numberOfNodes+=2;
  BvhLeaf *front = new BvhLeaf;
  BvhInterior *parent = new BvhInterior;

  parent->axis = axis;
  parent->box = leaf->box;
  parent->depth = leaf->depth;

  parent->back = leaf;
  parent->front = front;
  
  // now assign the triangles to the subnodes
  front->first = split + 1;
  front->last = leaf->last;
  front->depth = leaf->depth + 1;
  leaf->last = split;
  leaf->depth = front->depth;

  //  cout<<"bc="<<((BvhLeaf *)parent->back)->Count()<<endl;
  //  cout<<"fc="<<((BvhLeaf *)parent->front)->Count()<<endl;

  UpdateLeafBox((BvhLeaf *) parent->back);
  UpdateLeafBox((BvhLeaf *) parent->front);
  parent->back = SubdivideLeaf( (BvhLeaf *) parent->back, (axis + 1) % 3 );
  parent->front = SubdivideLeaf( (BvhLeaf *) parent->front, (axis + 1) % 3 );
  
  return parent;
}


void
Bvh::UpdateLeafBox(BvhLeaf *leaf)
{
  leaf->box.Reset();
  for (int i=leaf->first; i <= leaf->last; i++) {
#if 0
    leaf->box.min.Minimize(triangles[i]->GetMin());
    leaf->box.max.Maximize(triangles[i]->GetMax());
#else
    leaf->box.Include(triangles[i]->vertices[0]);
    leaf->box.Include(triangles[i]->vertices[1]);
    leaf->box.Include(triangles[i]->vertices[2]);
#endif
  } // for

  return;
}


void
Bvh::UpdateBoxes(BvhNode *node)
{
  if (node->IsLeaf()) {
    UpdateLeafBox((BvhLeaf *)node);
  }
  else {
    BvhInterior *interior = (BvhInterior *)node;
    UpdateBoxes(interior->back);
    UpdateBoxes(interior->front);
    node->box = Union(interior->back->box, interior->front->box);
  }

  return;
}

// Cast a ray through BVH hierarchy and find a nearest intersection
// This is a naive implementation
bool
Bvh::CastRay(Ray &ray)
{
  ray.Reset();

  priority_queue<RayTraversalInfo> nodeStack;

  nodeStack.push(RayTraversalInfo(0, root));

  //  cout<<"START!!!"<<endl;
  while (!nodeStack.empty()) {
    RayTraversalInfo info = nodeStack.top();
    nodeStack.pop();
    if (info.t >= ray.t)
      break;

    if (info.node->IsLeaf()) {
      // cout<<"node->tmin"<<info.t<<endl;
      BvhLeaf *leaf = (BvhLeaf *)info.node;
      for (int i = leaf->first; i <= leaf->last; i++) {
		// compute intersection with triangle
		// perform t-based depth culling already the routine
		triangles[i]->CastRay(ray);
		
      } // for i	  
    }
    else {
      BvhInterior *interior = (BvhInterior *)info.node;
      float tmin, tmax;
	  
      if (interior->back->box.ComputeMinMaxT(ray, tmin, tmax))
		if (tmax > 0 && tmin < ray.t) {
		  nodeStack.push(RayTraversalInfo(tmin, interior->back));
		}
      
      if (interior->front->box.ComputeMinMaxT(ray, tmin, tmax))
		if (tmax > 0 && tmin < ray.t) {
		  nodeStack.push(RayTraversalInfo(tmin, interior->front));
		}
    }
  } // while
  
  return ray.triangle != NULL;
}

// End of file Bvh.cpp

