//---------------------------------------------------------------------------
// sgl.cpp
// Empty implementation of the SGL (Simple Graphics Library)
// Date:  2011/11/04
// Author: Jaroslav Krivanek, Lukas Beran CTU Prague
//---------------------------------------------------------------------------

#include "sglcontext.h"
#include <iostream>
#include <math.h>
#include <string.h>

float myabs(float number) 
{
	return (number < 0) ? -number : number;
}


int myabs(int number) 
{
	return (number < 0) ? -number : number;
}

ContextManager * ContextManager::inst = NULL;
ContextManager* ContextManager::Inst()
{
    if (inst == NULL)
    {
        inst = new ContextManager;
    }
    return inst;
}

ContextManager::ContextManager()
{
	currentContext = -1;
}

ContextManager::~ContextManager()
{
	for(unsigned int i = 0; i < contexts.size(); i++)
		if(contexts[i])
			delete contexts[i];
}

int ContextManager::CreateContext(int width, int height)
{
	contexts.push_back(new SglContext(width, height, contexts.size()));
	return contexts.size() - 1;
}

SglContext::SglContext(int n_width, int n_height, int ID)
{
	width = n_width;
	height = n_height;

	colorBufferSize = width * height;
	colorBuffer = new Color[colorBufferSize];
	depthBuffer = new float[colorBufferSize];
	activeMatrix = SGL_MODELVIEW;
	pointSize = 1.0f;
	clearColor.Set(0, 0, 0);
	areaMode = SGL_FILL;
	waitingForSglEnd = false;

	contextID = ID;
	flagbits = 0;

	vertexesMax = 40;
	vertexesSize = 0;
	vertexes = new Vector4[vertexesMax];

	sglBeginScene = false;

	mapColors = NULL;
}

void  SglContext::clearScene()
{
	sglBeginScene = true;

	for(unsigned int loop1 = 0; loop1 < primitive.size(); loop1++)
		delete primitive[loop1];
	primitive.clear();

	for(unsigned int loop1 = 0; loop1 < light.size(); loop1++)
		delete light[loop1];
	light.clear();

	for(unsigned int loop1 = 0; loop1 < material.size(); loop1++)
		delete material[loop1];
	material.clear();

	for(unsigned int loop1 = 0; loop1 < emissiveMaterial.size(); loop1++)
		delete emissiveMaterial[loop1];
	emissiveMaterial.clear();
}

SglContext::~SglContext()
{
	delete [] colorBuffer;
	delete [] depthBuffer;
	delete [] vertexes;

	while(!matrixStack.empty())
	{
		delete [] matrixStack.top();
		matrixStack.pop();
	}

	clearScene();
}

// Vytvori paprsek odpovidajici jednomu bodu na obrazovce
Ray SglContext::makeRay(float x, float y, float * matrixInvert)
{
	Vector3 point1 = Vector3(x, y, -1.0f);
	Vector3 point2 = Vector3(x, y, 1.0f);
	InversViewportVector(&point1);
	InversViewportVector(&point2);
	MultVector(&point1, matrixInvert);
	MultVector(&point2, matrixInvert);

	Ray ray;
	ray.origin = point1;
	ray.direction = point2 - point1;
	ray.direction.Normalize();

	return ray;
}

bool SglContext::lightObstacle(Ray * r, Vector3 *lightPos)
{
	float t;
	for(int loop3 = primitive.size() - 1; loop3 >= 0; loop3--)
	{
		// je bod na telese zastinen nejakym jinym telesem od svetla?
		if(primitive[loop3]->intersect(*r, &t))
		{
			// kontrola, jestli se nejedna o primitimum prilis blizke svetlu - svetlo je pak na primitivu umisteno
			Vector3 bestPoint = r->origin + r->direction * t;
			Vector3 toLight = bestPoint - *lightPos;
			float dist= DotProd(toLight, toLight);
			if(dist > 10000)
			{
				return true;
			}
		}
	}
	return false;
}

Color SglContext::traceRay(Ray ray, int depth)
{
	Color result;
	Primitive * bestPrimitive;	// ktere primitivum se protlo s paprskem ray nejblize?
	Material * bestMaterial;	// a jaky melo material
	float r, g, b;			/// barevne slozky pixelu, do kterych se nascitavaji svetla
	Vector3 L, N, R, V;		/// vektor ke svetlu, normala povrchu v bode bestPoint, vektor odrazu svetla, od bestPoint k pozorovateli
	Vector3 bestPoint;		/// prusecik paprsku a telesa
	float bestHit = 999999.0f; // prusecik musi byt blize nez tato konstanta, jinak se nezapocita
	float tHit;

	int bestID = 1000;

	// najde prusecik s nejblizsim primitivem
	for(int loop1 = 0; loop1 < primitive.size(); loop1++)
	{
		if(primitive[loop1]->intersect(ray, &tHit) && tHit < bestHit)
		{
			bestPrimitive = primitive[loop1];
			bestID = loop1;
			bestHit = tHit;					
		}
	}

	// byl-li zasah (dostatecne blizko)
	if(bestHit < 99000.0f)
	{
		Ray shadowRay;
		bool wasObstacle;

		r = g = b = 0.0f;
		bestPoint = ray.origin + ray.direction * bestHit;
		bestMaterial = bestPrimitive->material;

		// normalovy vektor dle primitiva v bode bestPoint
		N = bestPrimitive->normal(&bestPoint);
		N.Normalize();

		// vektor od pozorovatele
		V = bestPoint - ray.origin;
		V.Normalize();

		for(int loop1 = 0; loop1 < light.size(); loop1++)
		{	
			for(int loop2 = 0; loop2 < light[loop1]->sampleAmount; loop2++)
			{
				light[loop1]->generateSample(bestPoint);

				// vektor ke svetlu
				L = light[loop1]->pos - bestPoint;
				L.Normalize();
				float LN = DotProd(N, L);

				// Nepricitat barvy v pripade spatneho uhlu
				if(LN > 0.0f)
				{
					// stinovy paprsek stejny jako ke svetlu
					shadowRay.origin = bestPoint;
					shadowRay.direction = L;

					// vektor odrazeny
					R = L-N*2*LN;
					R.Normalize();

					float RV = DotProd(R, V);
					float specular = 0.0f;

					if(!lightObstacle(&shadowRay, &light[loop1]->pos))
					{
						// Spekularni odraz se pocita pouze v pripade, ze ma nejaky zasadni vliv
						if(bestMaterial->ks > 0.00001f)
							specular = pow(RV, bestMaterial->shine) * bestMaterial->ks;
						r += light[loop1]->color.r * (LN * bestMaterial->color.r * bestMaterial->kd + specular);
						g += light[loop1]->color.g * (LN * bestMaterial->color.g * bestMaterial->kd + specular);
						b += light[loop1]->color.b * (LN * bestMaterial->color.b * bestMaterial->kd + specular);
					}
				}
			}
			if(depth > 0)
			{
				float RayN = DotProd(ray.direction, N);

				// Odrazeny paprsek
				if(bestMaterial->ks > 0.00001f)
				{
					Ray rr;
					rr.origin = bestPoint;
					rr.direction = ray.direction - N * 2 * RayN;
					rr.direction.Normalize();
					Color temp = traceRay(rr, depth - 1);
					r += bestMaterial->ks * temp.r;
					g += bestMaterial->ks * temp.g;
					b += bestMaterial->ks * temp.b;
				}

				// Lomeny paprsek
				if(bestMaterial->T > 0.00001f)
				{
					float gamma, sqrterm;

					float dot = RayN;

					if (dot < 0.0f) {
						// from outside into the inside of object
						gamma = 1.0f / bestMaterial->ior;
					}
					else {
						// from the inside to outside of object
						gamma = bestMaterial->ior;
						dot = -dot;
						N = -N;
					}
					
					sqrterm = 1.0 - gamma * gamma * (1.0 - dot * dot);

					// Check for total internal reflection, do nothing if it applies.
					if (sqrterm > 0.0) {
						Ray rt;
						sqrterm = dot * gamma + sqrt(sqrterm);
						rt.direction = N * -sqrterm + ray.direction * gamma;
						rt.origin = bestPoint + rt.direction * 0.1f;
						Color temp = traceRay(rt, depth - 1);
						r += bestMaterial->T * temp.r;
						g += bestMaterial->T * temp.g;
						b += bestMaterial->T * temp.b;
					}
				}
			}
		}

		result.r = r;
		result.g = g;
		result.b = b;
	} else { // paprsek nezasahl zadne primitivum
		if(mapColors)
		{
			float d = sqrt(ray.direction.x*ray.direction.x + ray.direction.y*ray.direction.y);
			float r = d>0 ? 0.159154943*acos(ray.direction.z)/d : 0.0;
			float u = 0.5 + ray.direction.x * r;
			float v = 1 - 0.5 - ray.direction.y * r; 

			int ID = (int) (v * mapHeight) * mapWidth + (int) (u * mapWidth);
			ID *= 3;
			result.r = mapColors[ID];
			result.g = mapColors[ID + 1];
			result.b = mapColors[ID + 2];
		}
	}

	return result;
}

void SglContext::raytrace()
{
	float matrixInvert[16];

	MyMath::matrixInversion(modelviewprojection, matrixInvert);

	Color rayColor[4];
	Color result;
	int primitiveSize = primitive.size();

	
	for(int loop1 = 0; loop1 < viewportHeight; loop1++)
	{
		for(int loop2 = 0; loop2 < viewportWidth; loop2++)
		{
	
	/*for(int loop1 = 400; loop1 < 490; loop1++)
	{
		for(int loop2 = 212; loop2 < 300; loop2++)
		{*/
			/*
			result = Color(0, 0, 0);
			for(int loop3 = 0; loop3 < 4; loop3++)
			{
				rayColor[loop3] = traceRay(loop2 + 0.25f * (loop3 / 2), loop1 + 0.25f * (loop3 % 2), matrixInvert);
				result = result + rayColor[loop3];
			}
			result = result / 4;
			*/
			Ray ray = makeRay(loop2 + 0.5f, loop1 + 0.5f, matrixInvert);

			int idLight = -1;
			for(int loop3 = 0; loop3 < light.size(); loop3++)
			{
				if(light[loop3]->intersect(ray))
				{
					idLight = loop3;
					break;
				}
			}

			if(idLight >= 0)
				result = light[idLight]->color;
			else
				result = traceRay(ray, 2);
			//cout << endl;

			int id = width * loop1 + loop2;
			colorBuffer[id].r = result.r;
			colorBuffer[id].g = result.g;
			colorBuffer[id].b = result.b;
		}
	}
}

void ScanLineAlg::init(Vector4 * vertexes, int vertexesSize, int viewportHeight)
{
	// upravim velikost pole TH na prihradkove trideni dle vysky aktualniho viewportu (je-li treba)
	if(viewportHeight > sizeTH)
	{
		delete [] TH;
		TH = new Edge*[viewportHeight];
		sizeTH = viewportHeight;
	}
	yMinLine = viewportHeight - 1; // vys byt nemuze prvni hranicni cara
	yMaxLine = 0;

	// Na zacatku prazdny seznam TH
	for(int loop1 = 0; loop1 < sizeTH; loop1++)
	{
		TH[loop1] = NULL;
	}

	// SAH by melo byt prazdne a nasledujici radek neni treba
	SAH.clear();

	// inicializace seznamu TH
	float x1, x2, y1, y2, z1, z2;
	Edge *tempEdge;
	for(int loop1 = 0; loop1 < vertexesSize - 1; loop1++)
	{
		if((int) (vertexes[loop1].y + 0.5f) == (int) (vertexes[loop1 + 1].y + 0.5f))
		{
			// vodorovna hrana, vynechavam ji
			continue;
		} else
			if(vertexes[loop1].y < vertexes[loop1 + 1].y)
			{
				// prvni vertex je vys nez druhy vertex
				y1 = vertexes[loop1].y;
				x1 = vertexes[loop1].x;
				z1 = vertexes[loop1].z;
				y2 = vertexes[loop1 + 1].y;
				x2 = vertexes[loop1 + 1].x;
				z2 = vertexes[loop1 + 1].z;
			} else {
				// druhy vertex je vys nez prvni
				y1 = vertexes[loop1 + 1].y;
				x1 = vertexes[loop1 + 1].x;
				z1 = vertexes[loop1 + 1].z;
				y2 = vertexes[loop1].y;
				x2 = vertexes[loop1].x;
				z2 = vertexes[loop1].z;
			}

		int iy1 = (int) (y1 + 0.5f);
		int iy2 = (int) (y2 + 0.5f) - 1; // zkraceni zespoda o 1px kvuli snizeni poctu pruseciku..

		float dx= (x2 - x1) / (y2 - y1); 
		int dy = iy2 - iy1;

		float dz = (z2 - z1) / (y2 - y1);

		if(iy1 < 0 || iy1 >=sizeTH)
			continue;
		tempEdge = new Edge(dy, x1, dx, z1, dz, TH[iy1]);
		TH[iy1] = tempEdge; // nove pridavana hrana ma jako nasledujici hranu tu, co byla puvodne v TH[iy1]

		if(iy1 < yMinLine)
			yMinLine = iy1;
		if(iy2 > yMaxLine)
			yMaxLine = iy2;
	}

	if(yMaxLine >= sizeTH)
		yMaxLine = sizeTH - 1;
	if(yMinLine < 0)
		yMinLine = 0;
}

void ScanLineAlg::fill(SglContext * cc)
{
	Edge * tempEdge;
	for(int yLine = yMinLine ; yLine <= yMaxLine; yLine++)
	{
		tempEdge = TH[yLine];
		while(tempEdge != NULL)
		{
			SAH.push_back(tempEdge);
			tempEdge = tempEdge->edge;
		}
		TH[yLine] = NULL;

		int swap;
		// bubble sort na SAH dle x
		for(unsigned int loop1 = 0; loop1 + 1 < SAH.size(); loop1++)
		{
			swap = 0;
			for(unsigned int loop2 = 0; loop2 + loop1 + 1 < SAH.size(); loop2++)
			{
				if(SAH[loop2]->x > SAH[loop2 + 1]->x)
				{
					tempEdge = SAH[loop2];
					SAH[loop2] = SAH[loop2 + 1];
					SAH[loop2 + 1] = tempEdge;
					swap = 1;
				}
			}
			if(swap == 0)	// neprohodily se zadne hrany, bubble sort muze skoncit
				break;
		}

		//if(SAH.size() > 0)
		//	cout << endl << yLine << " .... ";
		// vykresli useky mezi lichymi a sudymi hranami

		if(cc->flagbits & SGL_DEPTH_TEST)
			for(unsigned int loop1 = 0; loop1 + 1 < SAH.size(); loop1 += 2)
			{
				//cout << SAH[loop1]->x << " " << SAH[loop1 + 1]->x << " ";
				cc->DrawHorizontalLine(SAH[loop1]->x, SAH[loop1 + 1]->x, SAH[loop1]->z, SAH[loop1 + 1]->z, yLine);
			}
		else
			for(unsigned int loop1 = 0; loop1 + 1 < SAH.size(); loop1 += 2)
			{
				cc->DrawHorizontalLine(SAH[loop1]->x, SAH[loop1 + 1]->x, yLine);
			}

		// vyradi hrany dy = 0 a zbyle aktualizuje
		for(unsigned int loop1 = 0; loop1 < SAH.size(); loop1++)
		{
			// vyradi hranu
			if(SAH[loop1]->dy == 0)
			{
				delete SAH[loop1];
				SAH.erase(SAH.begin() + loop1, SAH.begin() + loop1 + 1);
				loop1--;
				continue;
			}

			SAH[loop1]->dy--;
			SAH[loop1]->x += SAH[loop1]->dx;
			SAH[loop1]->z += SAH[loop1]->dz;
		}
	}
}

void SglContext::FillPolygon()
{
	// Pokud je polygon zadan pouze tremi vertexy, pouzije se rychlejsi algoritmus na vyplnovani
	if(vertexesSize == 4)
	{
		//FillTriangle();
		//return;
	}

	ContextManager::Inst()->scanLineAlg.init(vertexes, vertexesSize, viewportHeight);
	ContextManager::Inst()->scanLineAlg.fill(this);
}

void SglContext::DrawHorizontalLine(float fx1, float fx2, float fz1, float fz2, float fy)
{
	int x1 = (int) (fx1 + 0.5f);
	int x2 = (int) (fx2 + 0.5f);
	int temp;
	if(x1 > x2)
	{
		temp = x1;
		x1 = x2;
		x2 = temp;
	}
	int y = (int) (fy + 0.5f);
	float dz = (fz2 - fz1) / (x2 - x1);
	for(int loop1 = x1; loop1 < x2; loop1++)
	{
		DrawPixel(loop1, y, fz1);
		fz1 += dz;
	}
	DrawPixel(x2, y, fz2);
}


void SglContext::DrawHorizontalLine(float fx1, float fx2, float fy)
{
	int x1 = (int) (fx1 + 0.5f);
	int x2 = (int) (fx2 + 0.5f);
	int temp;
	if(x1 > x2)
	{
		temp = x1;
		x1 = x2;
		x2 = temp;
	}
	int y = (int) (fy + 0.5f);

	for(int loop1 = x1; loop1 <= x2; loop1++)
		DrawPixel(loop1, y);
}

void SglContext::FillTriangle()
{
	// Algoritmus dle http://www-users.mat.uni.torun.pl/~wrona/3d_tutor/tri_fillers.html
	Vector4 A, B, C, // body trojuhelnika serazene dle y-ove osy
			S, E, // startovni a koncovy bod scanline
			temp;
	for(unsigned int loop1 = 0; loop1 + 2 < vertexesSize; loop1 += 2)
	{
		// BUBBLE SORT, seradi body dle y-ove souradnice
		for(int loop2 = 0; loop2 < 2; loop2++)
		{
			for(int loop3 = 0; loop3 < 2 - loop2; loop3++)
			{
				int id1 = loop3 + loop1;
				int id2 = id1 + 1;
				if(vertexes[id1].y > vertexes[id2].y || 
				   (vertexes[id1].y == vertexes[id2].y &&
				    vertexes[id1].x < vertexes[id2].x))
				{
					temp = vertexes[id1];
					vertexes[id1] = vertexes[id2];
					vertexes[id2] = temp;
				}
			}
		}

		Vector4 A = vertexes[loop1];
		Vector4 B = vertexes[loop1+1];
		Vector4 C = vertexes[loop1+2];

		float dx1, dx2, dx3; // o kolik se budu posouvat na x-ove a y-ove souradnici u startovniho a koncoveho bodu
		if (B.y - A.y > 0.0f) 
			dx1 = (B.x - A.x) / (B.y - A.y);
		else dx1 = 0.0f;

		if (C.y - A.y > 0.0f) 
			dx2 = (C.x - A.x) / (C.y - A.y);
		else dx2 = 0.0f;

		if (C.y - B.y > 0.0f) 
			dx3 = (C.x - B.x) / (C.y - B.y); 
		else dx3 = 0.0f;
		
		S = E = A;

		if(dx1 > dx2) {
			for( ; S.y <= B.y; S.y++)
			{
				DrawHorizontalLine(S.x, E.x, S.y);
				S.x += dx2;
				E.x += dx1;
			}
			E = B;
			for( ; S.y <= C.y; S.y++)
			{
				DrawHorizontalLine(S.x, E.x, S.y);
				S.x += dx2;
				E.x += dx3;
			}
		} else {
			for( ; S.y <= B.y; S.y++)
			{
				DrawHorizontalLine(S.x, E.x, S.y);
				S.x += dx1;
				E.x += dx2;
			}
			S = B;
			for( ; S.y <= C.y; S.y++)
			{
				DrawHorizontalLine(S.x, E.x, S.y);
				S.x += dx3;
				E.x += dx2;
			}
		}
	}
}

void SglContext::DrawPixel(int x, int y, float z)
{
	int i = x + y * width;

	if(x >= 0 && x < width && 
	   y >= 0 && y < height && z > depthBuffer[i])
	{
		depthBuffer[i] = z;
		colorBuffer[i] = currentColor;	
	}
}

void SglContext::DrawPixel(int x, int y)
{
	int i = x + y * width;

	if(x >= 0 && x < width && 
	   y >= 0 && y < height)
		colorBuffer[i] = currentColor;	
}

#include "sgl.h"

/// Current error code.
static sglEErrorCode _libStatus = SGL_NO_ERROR;

static inline void setErrCode(sglEErrorCode c) 
{
  if(_libStatus==SGL_NO_ERROR)
    _libStatus = c;
}

//---------------------------------------------------------------------------
// sglGetError()
//---------------------------------------------------------------------------
sglEErrorCode sglGetError(void) 
{
  sglEErrorCode ret = _libStatus;
  _libStatus = SGL_NO_ERROR;
  return ret;
}

//---------------------------------------------------------------------------
// sglGetErrorString()
//---------------------------------------------------------------------------
const char* sglGetErrorString(sglEErrorCode error)
{
  static const char *errStrigTable[] = 
  {
      "Operation succeeded",
      "Invalid argument(s) to a call",
      "Invalid enumeration argument(s) to a call",
      "Invalid call",
      "Quota of internal resources exceeded",
      "Internal library error",
      "Matrix stack overflow",
      "Matrix stack underflow",
      "Insufficient memory to finish the requested operation"
  };

  if((int)error<(int)SGL_NO_ERROR || (int)error>(int)SGL_OUT_OF_MEMORY ) {
    return "Invalid value passed to sglGetErrorString()"; 
  }

  return errStrigTable[(int)error];
}

//---------------------------------------------------------------------------
// Initialization functions
//---------------------------------------------------------------------------

void sglInit(void) {
	_libStatus = SGL_NO_ERROR;
	ContextManager::Inst();
}

void sglFinish(void) {
	ContextManager::ClearContextManager();	
}

int sglCreateContext(int width, int height) {	
	return ContextManager::Inst()->CreateContext(width, height);
}

void sglDestroyContext(int id) {
	ContextManager::Inst()->DestroyContext(id);
}

void sglSetContext(int id) 
{
	ContextManager::Inst()->SetCurrentContext(id);
}

int sglGetContext(void) {
	return ContextManager::Inst()->GetCurrentContext();
}

float *sglGetColorBufferPointer(void) {
	return (float *) ContextManager::Inst()->CC()->colorBuffer;
}

//---------------------------------------------------------------------------
// Drawing functions
//---------------------------------------------------------------------------

void sglClearColor (float r, float g, float b, float alpha) 
{
	ContextManager::Inst()->CC()->clearColor.Set(r, g, b);
	if(ContextManager::Inst()->BetweenBeginAndEnd())
		setErrCode(SGL_INVALID_OPERATION);
}

void sglClear(unsigned what) {
	SglContext * c = ContextManager::Inst()->CC();

	int end =  c->width * c->height;

	if(what & SGL_COLOR_BUFFER_BIT)
	{
		Color * color = c->colorBuffer;
		for(int i = 0; i < end; i++, color++)
			*color = c->clearColor;
	}
	
	if(what & SGL_DEPTH_BUFFER_BIT) {
		float * depth = c->depthBuffer;
		for(int i = 0; i < end; i++, depth++)
			*depth = -2.0f;
	} else 
		setErrCode(SGL_INVALID_VALUE);

	if(c->waitingForSglEnd)
		setErrCode(SGL_INVALID_OPERATION);
}

void sglBegin(sglEElementType mode) {
	//sglAreaMode(SGL_LINE); // ZRUSIT Pri zacatku reseni vyplnovani polygonu 


	SglContext * cc = ContextManager::Inst()->CC();
	cc->vertexesSize = 0;
	cc->currentElemType = mode;

	if(!cc->waitingForSglEnd)
	{		
		cc->waitingForSglEnd = true;	
	} else {
		setErrCode(SGL_INVALID_OPERATION);
	}
	if(mode < SGL_POINTS || mode >=SGL_LAST_ELEMENT_TYPE) {
		setErrCode(SGL_INVALID_ENUM);
		return;
	}	
}

void InversViewportVector(Vector3 *v)
{
	SglContext * cc = ContextManager::Inst()->CC();

	v->x = ((v->x - cc->viewportX) * 2.0f / cc->viewportWidth) - 1;
	v->y = ((v->y - cc->viewportY) * 2.0f / cc->viewportHeight) - 1;
}

// Vynasobi zleva vektor v matici matrix
void MultVector(Vector3 * v, float * matrix)
{
	float w = matrix[3] * v->x + matrix[7] * v->y + matrix[11] * v->z + matrix[15];
	w = 1.0 / w;
	Vector3 o = Vector3(w * (matrix[0] * v->x + matrix[4] * v->y + matrix[8] * v->z + matrix[12]),
		                w * (matrix[1] * v->x + matrix[5] * v->y + matrix[9] * v->z + matrix[13]),
						w * (matrix[2] * v->x + matrix[6] * v->y + matrix[10] * v->z + matrix[14]));
	
	*v = o;
}

// Vynasobi vektor v na vstupu modelview, projekcni matici, prevede z homogennich souradnic do svetovych a 
// vynasobi "matici" viewportu
void MultVector(Vector4 * v)
{	
	SglContext * cc = ContextManager::Inst()->CC();
	float * output = cc->modelviewprojection;
	Vector4 o = Vector4(output[0] * v->x + output[4] * v->y + output[8] * v->z + output[12] * v->w,
		                output[1] * v->x + output[5] * v->y + output[9] * v->z + output[13] * v->w,
						output[2] * v->x + output[6] * v->y + output[10] * v->z + output[14] * v->w,
						output[3] * v->x + output[7] * v->y + output[11] * v->z + output[15] * v->w);
	
	// Prevod z homogennich souradnic do svetovych
	o.x /= o.w;
	o.y /= o.w;
	o.z = o.w / o.z;
	//o.w /= o.w;
	
	v->x = (o.x + 1) * cc->viewportWidth / 2 + cc->viewportX;
	v->y = (o.y + 1) * cc->viewportHeight / 2 + cc->viewportY;
	v->z = o.z;
	//v->w = o.w;

	
}

void SglContext::DrawLineSlower(float fx1, float fy1, float fx2, float fy2)
{ 
  int x1;
  int x2;
  int y1;
  int y2;

  if(fx1 <= fx2)
  {
	  x1 = (int) (fx1 + 0.5f);
	  x2 = (int) (fx2 + 0.5f);
	  y1 = (int) (fy1 + 0.5f);
	  y2 = (int) (fy2 + 0.5f);
  } else {
	  x1 = (int) (fx2 + 0.5f);
	  x2 = (int) (fx1 + 0.5f);
	  y1 = (int) (fy2 + 0.5f);
	  y2 = (int) (fy1 + 0.5f);
  }

  int dx, dy, d, d10, d11, i;
  dx=x2-x1;   
  dy=y2-y1;  

  int minus = (y1 > y2) ? -1 : 1;
  
  if(myabs(y2 - y1) < myabs(x2 - x1))
  {
	  if(y1 <= y2)
	  {
		d=2*dy-dx;  
	    d10=2*dy; 
	    d11=2*dy-2*dx;
		for (i=0; i<=dx; i++)
		{ 
			DrawPixel(x1, y1);
			if (d>0)
				{ x1++; y1++; d+=d11; }
			else
				{ x1++; d+=d10; }
		}
	  } else
	  {
		d=-2*dy-dx;  
	    d10=-2*dy; 
	    d11=-2*dy-2*dx;
		for (i=0; i<=dx; i++)
		{ 
			DrawPixel(x1, y1);
			if (d>0)
				{ x1++; y1--; d+=d11; }
			else
				{ x1++; d+=d10; }
		}
	  }
	  
  }
  else
  {
	  if(y1 <= y2)
	  {
		  d=2*dx-dy;  
		  d10=2*dx; 
		  d11=2*dx-2*dy;
		  for (i=0; i<= dy; i++)
			{ 
			  DrawPixel(x1, y1);
			  if (d>0)
				 { y1++; x1++; d+=d11; }
			  else
				 { y1++; d+=d10; }
			}
	  } else {
		  /* ZDE je chyba, pridava to tecky navic u svislych car */
		  d=2*dx-dy;  
		  d10=2*dx; 
		  d11=2*dx+2*dy;
		   for (i=0; i <= -dy; i++)
			{ 
			  if(i!=0) // magic formula
				DrawPixel(x1, y1);
			  if (d>0)
				 { y1--; x1++; d+=d11; }
			  else
				 { y1--; d+=d10; }
			}
	  }
  }
}

// Bresenham na vykreslovani usecek
void SglContext::DrawLineFaster(float fx1, float fy1, float fx2, float fy2)
{ 
  int x1;
  int x2;
  int y1;
  int y2;

  // Zaokrouhleni souradnic krajnich bodu
  if(fx1 <= fx2)
  {
	  x1 = (int) (fx1 + 0.5f);
	  x2 = (int) (fx2 + 0.5f);
	  y1 = (int) (fy1 + 0.5f);
	  y2 = (int) (fy2 + 0.5f);
  } else {
	  x1 = (int) (fx2 + 0.5f);
	  x2 = (int) (fx1 + 0.5f);
	  y1 = (int) (fy2 + 0.5f);
	  y2 = (int) (fy1 + 0.5f);
  }

  int dx, dy, d, d10, d11, i;
  dx=x2-x1;   
  dy=y2-y1;  

  SglContext * cc = ContextManager::Inst()->CC();
  Color * colorBuffer = cc->colorBuffer;
  Color currentColor = cc->currentColor;
  int width = cc->viewportWidth;
  int index = x1 + y1 * width;

  if(myabs(y2 - y1) < myabs(x2 - x1))
  {
	  if(y1 <= y2)
	  {
		d=2*dy-dx;  
	    d10=2*dy; 
	    d11=2*dy-2*dx;
		for (i=0; i<=dx; i++)
		{ 
			colorBuffer[index] = currentColor;
			if (d>0)
				{ index++; index+=width; d+=d11; }
			else
				{ index++; d+=d10; }
		}
	  } else
	  {
		d=-2*dy-dx;  
	    d10=-2*dy; 
	    d11=-2*dy-2*dx;
		for (i=0; i<=dx; i++)
		{ 
			colorBuffer[index] = currentColor;
			if (d>0)
				{ index++; index-= width; d+=d11; }
			else
				{ index++; d+=d10; }
		}
	  }
	  
  }
  else
  {
	  if(y1 <= y2)
	  {
		  d=2*dx-dy;  
		  d10=2*dx; 
		  d11=2*dx-2*dy;
		  for (i=0; i<= dy; i++)
			{ 
			  colorBuffer[index] = currentColor;
			  if (d>0)
				 { index+=width; index++; d+=d11; }
			  else
				 { index+=width; d+=d10; }
			}
	  } else {
		  d=2*dx-dy;  
		  d10=2*dx; 
		  d11=2*dx+2*dy;
		   for (i=0; i <= -dy; i++)
			{ 
			  if(i!=0) // magic formula
				colorBuffer[index] = currentColor;
			  if (d>0)
				 { index-=width; index++; d+=d11; }
			  else
				 { index-=width; d+=d10; }
			}
	  }
  }
}

void SglContext::DrawLine(float fx1, float fy1, float fx2, float fy2)
{
	int w = width;
	int h = height;

	// Pokud se usecka vejde cela na obrazovku, vykresluje se rychleji -
	// nemusi se kontrolovat kazdy bod, jestli je uvnitr colorBufferu
	if(fx1 >= 0.0f && (int) (fx2 + 0.5f) < w && (int) (fx1 + 0.5f) < w && fx2 >= 0 &&
	   fy1 >= 0.0f && (int) (fy2 + 0.5f) < h && (int) (fy1 + 0.5f) < h && fy2 >= 0)
	   DrawLineFaster(fx1, fy1, fx2, fy2);
	else
	   DrawLineSlower(fx1, fy1, fx2, fy2);
}

// Pouziva pouze DrawLineDDA
float sign(float a)
{
	return (a > 0) ? 1.0f : ((a == 0.0f) ? 0.0f : -1.0f);
}
// Algoritmus DDA na vykresleni usecky
// NEPOUZIVA SE
void DrawLineDDA(float fx1, float fy1, float fx2, float fy2)
{ 
  int PocetKroku,i;
  int x1 = (int) fx1;
  int x2 = (int) fx2;
  int y1 = (int) fy1;
  int y2 = (int) fy2;

  float x,y,dx,dy;
  PocetKroku=(int) max(myabs((x2-x1)),myabs((y2-y1)));
  ContextManager::Inst()->CC()->DrawPixel(x1, y1);
  if (PocetKroku != 0)
    { 
	  dx=(x2-x1) / (float) PocetKroku; 
	  dy=(y2-y1) / (float) PocetKroku;
      x=x1+0.5f*sign(dx); 
	  y=y1+0.5f*sign(dy);
      for (i=0; i<PocetKroku; i++)
        { 
		  ContextManager::Inst()->CC()->DrawPixel((int) x, (int) y);
          x+=dx; y+=dy;
        }
    }
}

void SglContext::DrawPoints()
{
	// Vykresluje body s velikosti po zaokrouhleni 1
	if(pointSize < 1.49f)
		for(unsigned int i = 0; i < vertexesSize; i++)
		{
			DrawPixel(vertexes[i].x, vertexes[i].y, vertexes[i].z);
		}
	else
	{ // Vykresluje body silnejsi nez jeden pixel jako ctverce
		for(unsigned int i = 0; i < vertexesSize; i++)
		{
			float sx = vertexes[i].x - pointSize / 2;
			float sy = vertexes[i].y - pointSize / 2;
			// Vykresli jeden bod jako ctverec
			for(int j = 0; j < (int) (pointSize + 0.5f); j++)
				for(int k = 0; k < (int) (pointSize + 0.5f); k++)
					DrawPixel(sx + j, sy + k, vertexes[i].z);
		}
	}
}

void SglContext::DrawLines()
{
	// Vykresli serii usecek. Sude body jsou zacatky usecek, liche jsou konce usecek.
	for(unsigned int i = 0; i < vertexesSize / 2; i++)
		DrawLine(vertexes[i * 2].x, vertexes[i * 2].y, vertexes[i * 2 + 1].x, vertexes[i * 2 + 1].y);
}

void SglContext::DrawLineStrip()
{
	// Vykresli linii usecek. Prvni usecka zadana dvema body. Kazdy dalsi bod definuje dalsi usecku.
	for(unsigned int i = 0; i < vertexesSize - 1; i++)
		DrawLine(vertexes[i].x, vertexes[i].y, vertexes[i + 1].x, vertexes[i + 1].y);
}

void SglContext::DrawLineLoop()
{
	// Vykresli smycku usecek, prvni usecka zadana dvema body, kazdy dalsi bod definuje novou usecku
	for(unsigned int i = 0; i < vertexesSize - 1; i++)
		DrawLine(vertexes[i].x, vertexes[i].y, vertexes[i + 1].x, vertexes[i + 1].y);
			
	// Spoji posledni a prvni bod
	DrawLine(vertexes[0].x, vertexes[0].y, vertexes[vertexesSize - 1].x, vertexes[vertexesSize - 1].y);
}

void sglEnd(void) {
	SglContext * cc = ContextManager::Inst()->CC();

	// Osetri mozne chyby
	if(!cc->waitingForSglEnd) { setErrCode(SGL_INVALID_OPERATION); return; }

	// Co se bude vykreslovat?
	sglEElementType type = cc->currentElemType;

	// V pripade vykreslovani polygonu se dle areaMode rozhodne vykreslit ho plny, nebo pouze linie, ci krajni body
	if(type == SGL_POLYGON || type == SGL_TRIANGLES)
	{
		switch(cc->areaMode)
		{
			case SGL_POINT :
				type = SGL_POINTS;
				break;
			case SGL_LINE :
				type = SGL_LINE_LOOP;
				break;
		}
	}

	switch(type)
	{
		case SGL_POINTS :
			cc->DrawPoints();
			break;
		case SGL_LINES :
			cc->DrawLines();
			break;
		case SGL_LINE_STRIP :
			cc->DrawLineStrip();
			break;
		case SGL_TRIANGLES :
			cc->FillTriangle();
			break;
		case SGL_POLYGON :
			if(cc->sglBeginScene)
			{
				cc->addTriangles();
			} else
			{
				cc->vertexes[cc->vertexesSize++] = cc->vertexes[0];
				cc->FillPolygon();
			}
			break;
		case SGL_LINE_LOOP :
			cc->DrawLineLoop();
			break;
		default :
			// Nemelo by nastat, chybu zavola sglBegin
			break;
	}

	ContextManager::Inst()->CC()->waitingForSglEnd = false;
}

void sglVertex4f(float x, float y, float z, float w) 
{
	SglContext * cc = ContextManager::Inst()->CC();

	// V pripade maleho mista pro vrcholy se zvetsi dynamicke pole vrcholu
	if(cc->vertexesSize >= cc->vertexesMax - 2)
	{
		cc->vertexesMax *= 2;
		Vector4 * tempPoints = new Vector4[cc->vertexesMax];
		for(unsigned int i = 0; i < cc->vertexesSize; i++)
			tempPoints[i] = cc->vertexes[i];

		delete [] cc->vertexes;
		cc->vertexes = tempPoints;
	}
	// konec zvetsovani pole

	cc->vertexes[cc->vertexesSize].x = x;
	cc->vertexes[cc->vertexesSize].y = y;
	cc->vertexes[cc->vertexesSize].z = z;
	cc->vertexes[cc->vertexesSize].w = w;

	if(!cc->sglBeginScene)
		MultVector(&cc->vertexes[cc->vertexesSize]);

	cc->vertexesSize++;
}

void sglVertex3f(float x, float y, float z) 
{
	sglVertex4f(x, y, z, 1.0f);
}

void sglVertex2f(float x, float y) 
{
	sglVertex4f(x, y, 1.0f, 1.0f);
}

void SglContext::FillCircle(int xs, int ys, int r)
{
	int x=r; 
	int y=0; 
	int d=0;
	int d1, d2;

	while (x>=y)
	{ 
		// Vykresli do jednotlivych osmi oktantu dle vypocitaneho jednoho
		DrawHorizontalLine(-x + xs, x + xs, y + ys);
		DrawHorizontalLine(-x + xs, x + xs, -y + ys);
		DrawHorizontalLine(-y + xs, y + xs, x + ys);
		DrawHorizontalLine(-y + xs, y + xs, -x + ys);

		// Bresenhamuv algoritmus na kruznici
		d1=d+2*y+1; d2=d-2*x+2*y+2;
		if ((d1+d2)>0)
			{ x--; y++; d=d2; }
		else               
			{ y++; d=d1; }
	}
}

void SglContext::DrawCircle(int xs, int ys, int r)
{
	int x=r; 
	int y=0; 
	int d=0;
	int d1, d2;

	while (x >= y)
	  { 
		// Vykresli do jednotlivych osmi oktantu dle vypocitaneho jednoho
		DrawPixel(x + xs, y + ys);
		DrawPixel(x + xs, -y + ys);
		DrawPixel(-x + xs, y + ys);
		DrawPixel(-x + xs, -y + ys);

		DrawPixel(y + xs, x + ys);
		DrawPixel(y + xs, -x + ys);
		DrawPixel(-y + xs, x + ys);
		DrawPixel(-y + xs, -x + ys);

		// Bresenhamuv algoritmus na kruznici
		d1=d+2*y+1; d2=d-2*x+2*y+2;
		if ((d1+d2)>0)
			{ x--; y++; d=d2; }
		else               
			{ y++; d=d1; }
	  }
}

void sglCircle(float x, float y, float z, float radius) 
{
	SglContext * cc = ContextManager::Inst()->CC();

	// Kontrola moznych chyb
	if(radius < 0) {			setErrCode(SGL_INVALID_VALUE); return; }
	if(cc->waitingForSglEnd) {	setErrCode(SGL_INVALID_OPERATION); return; }

	// Transformuje stred kruhu dle aktualnich transformacnich matic
	Vector4 v = Vector4(x, y, z, 1);
	MultVector(&v);

	// Vypocet noveho upraveneho polomeru dle specifikace v sgl.h
	float output[16];
	memcpy(output, cc->modelviewprojection, 64);
	output[0] *= cc->viewportWidth / 2;
	output[5] *= cc->viewportHeight /2;
	float scale = sqrt(output[0] * output[5] - output[1] * output[4]);

	// Vykresli kruznici s tranformovanym stredem a upravenym polomerem
	switch(cc->areaMode)
	{
		case SGL_POINT :
			sglBegin(SGL_POINTS);
				sglVertex2f(x, y);
			sglEnd();
			break;
		case SGL_FILL :
			cc->FillCircle((int) v.x, (int) v.y, (int) (radius * scale));
			break;
		case SGL_LINE :
			cc->DrawCircle((int) v.x, (int) v.y, (int) (radius * scale));
			break;
	}
}

#define M_PI 3.14159265358979323846
void Ellipse(float x, float y, float a, float b)
{
	SglContext * cc = ContextManager::Inst()->CC();

	sglEElementType type;

	bool onlyCenter = false;
	switch(cc->areaMode)
	{
		case SGL_FILL :
			type = SGL_POLYGON;
			break;
		case SGL_POINT :
			onlyCenter = true;
			break;
		case SGL_LINE :
			type = SGL_LINE_LOOP;
			break;
	}

	if(onlyCenter)
	{
		sglBegin(SGL_POINTS);
			sglVertex2f(x, y);
		sglEnd();
		return;
	}

	// Vykresli elipsu pomoci usecek, nebo jako polygon
	sglBegin(type);
	float xx, yy;
	float angle = 0.0f;
	float angleStep = (float) (M_PI / 20.0);
	for(int i = 0; i < 40; i++)
	{
		xx = a * cos(angle);
		yy = b * sin(angle);
		angle += angleStep;
		sglVertex2f(xx + x, yy + y);
	}
	sglEnd();
}

void sglEllipse(float x, float y, float z, float a, float b) 
{
	ContextManager::Inst()->GetCurrentContext();

	// Kontrola moznych chyb
	if(a < 0 || b < 0) { setErrCode(SGL_INVALID_VALUE); return; }

	Ellipse(x, y, a, b);
}

void Arc(float x, float y, float z, float radius, float from, float to) 
{
	SglContext * cc = ContextManager::Inst()->CC();

	// Kontrola moznych chyb
	if(radius < 0) { setErrCode(SGL_INVALID_VALUE); return; }

	if(cc->areaMode == SGL_POINT)
	{
		sglBegin(SGL_POINTS);
			sglVertex2f(x, y);
		sglEnd();
		return;
	}

	// Kruhova vysec vykreslena pomoci serie usecek
	if(cc->areaMode == SGL_LINE)
		sglBegin(SGL_LINE_STRIP);
	else
		sglBegin(SGL_POLYGON);

		float xx, yy;
		float angle = from;
		float angleStep = (float) (M_PI / 20.0);
		int usecek = (int) ((0.01f + 40.0f*(myabs(from-to))/(2 * M_PI)) + 0.5f);

		for(int i = 0; i < usecek; i++)
		{
			xx = radius * cos(angle);
			yy = radius * sin(angle);
			angle += angleStep;
			sglVertex2f(xx + x, yy + y);
		}

		xx = radius * cos(to);
		yy = radius * sin(to);
		sglVertex2f(xx + x, yy + y);

		if(cc->areaMode == SGL_FILL)
			sglVertex2f(x, y);
	sglEnd();
}

void sglArc(float x, float y, float z, float radius, float from, float to) 
{
	Arc(x, y, z, radius, from, to );
}

//---------------------------------------------------------------------------
// Transform functions
//---------------------------------------------------------------------------

void sglMatrixMode( sglEMatrixMode mode ) 
{
	// Kontrola moznych chyb
	if(mode != SGL_MODELVIEW && mode != SGL_PROJECTION) { setErrCode(SGL_INVALID_ENUM); return; }
	if(ContextManager::Inst()->BetweenBeginAndEnd()) {    setErrCode(SGL_INVALID_OPERATION); return; }

	// Nastavi, ktera matice (MODELVIEW nebo PROJECTION) bude aktualni
	ContextManager::Inst()->CC()->activeMatrix = (int) mode;
}

void sglPushMatrix(void) 
{
	SglContext * cc = ContextManager::Inst()->CC();

	// Kontrola moznych chyb
	if(cc->waitingForSglEnd) { setErrCode(SGL_INVALID_OPERATION); return; }

	// Udela kopii aktualni matice a kopii vlozi na zasobnik
	float * matrix = new float[16];
	memcpy(matrix, cc->matrix[cc->activeMatrix], 64);
	cc->matrixStack.push(matrix);
}

void sglPopMatrix(void) {
	SglContext * cc = ContextManager::Inst()->CC();

	// Kontrola moznych chyb
	if(cc->waitingForSglEnd) {		  setErrCode(SGL_INVALID_OPERATION); return; }
	if(cc->matrixStack.size() == 0) { setErrCode(SGL_STACK_UNDERFLOW); return;}

	// Vynda matici ze zasobniku a prekopiruje jeji obsah do aktualni matice
	float * matrix = cc->matrixStack.top();
	memcpy(cc->matrix[cc->activeMatrix], matrix, 64);
	cc->matrixStack.pop();
	delete [] matrix;

	// Udržuje aktuální modelview * projection matici
	MultMatrix(cc->modelviewprojection, cc->matrix[SGL_MODELVIEW], cc->matrix[SGL_PROJECTION]);
}

void sglLoadIdentity(void) {
	SglContext * cc = ContextManager::Inst()->CC();
	if(cc->waitingForSglEnd)
	{
		setErrCode(SGL_INVALID_OPERATION);
		return;
	}

	// nahradni aktualní matici maticí jednotkovou
	float * matrix = cc->matrix[cc->activeMatrix];
	matrix[0] = 1; matrix[4] = 0; matrix[8] = 0;  matrix[12] = 0;
	matrix[1] = 0; matrix[5] = 1; matrix[9] = 0;  matrix[13] = 0;
	matrix[2] = 0; matrix[6] = 0; matrix[10] = 1; matrix[14] = 0;
	matrix[3] = 0; matrix[7] = 0; matrix[11] = 0; matrix[15] = 1;

	// Udržuje aktuální modelview * projection matici
	MultMatrix(cc->modelviewprojection, cc->matrix[SGL_MODELVIEW], cc->matrix[SGL_PROJECTION]);
}

void sglLoadMatrix(const float *matrix) {
	SglContext * cc = ContextManager::Inst()->CC();
	if(cc->waitingForSglEnd)
	{
		setErrCode(SGL_INVALID_OPERATION);
		return;
	}

	// prekopiruje obsah matice argumentu do aktualni matice
	memcpy(cc->matrix[cc->activeMatrix], matrix, 64);

	// Udržuje aktuální modelview * projection matici
	MultMatrix(cc->modelviewprojection, cc->matrix[SGL_MODELVIEW], cc->matrix[SGL_PROJECTION]);
}

// Nasobeni matic
// outputmatrix = matrix1 * matrix2
void MultMatrix(float * outputmatrix, const float *matrix1, const float *matrix2)
{
	outputmatrix[0]=matrix1[0]*matrix2[0]+matrix1[1]*matrix2[4]+matrix1[2]*matrix2[8]+matrix1[3]*matrix2[12];
	outputmatrix[1]=matrix1[0]*matrix2[1]+matrix1[1]*matrix2[5]+matrix1[2]*matrix2[9]+matrix1[3]*matrix2[13];
	outputmatrix[2]=matrix1[0]*matrix2[2]+matrix1[1]*matrix2[6]+matrix1[2]*matrix2[10]+matrix1[3]*matrix2[14];
	outputmatrix[3]=matrix1[0]*matrix2[3]+matrix1[1]*matrix2[7]+matrix1[2]*matrix2[11]+matrix1[3]*matrix2[15];

	outputmatrix[4]=matrix1[4]*matrix2[0]+matrix1[5]*matrix2[4]+matrix1[6]*matrix2[8]+matrix1[7]*matrix2[12];
	outputmatrix[5]=matrix1[4]*matrix2[1]+matrix1[5]*matrix2[5]+matrix1[6]*matrix2[9]+matrix1[7]*matrix2[13];
	outputmatrix[6]=matrix1[4]*matrix2[2]+matrix1[5]*matrix2[6]+matrix1[6]*matrix2[10]+matrix1[7]*matrix2[14];
	outputmatrix[7]=matrix1[4]*matrix2[3]+matrix1[5]*matrix2[7]+matrix1[6]*matrix2[11]+matrix1[7]*matrix2[15];

	outputmatrix[8]=matrix1[8]*matrix2[0]+matrix1[9]*matrix2[4]+matrix1[10]*matrix2[8]+matrix1[11]*matrix2[12];
	outputmatrix[9]=matrix1[8]*matrix2[1]+matrix1[9]*matrix2[5]+matrix1[10]*matrix2[9]+matrix1[11]*matrix2[13];
	outputmatrix[10]=matrix1[8]*matrix2[2]+matrix1[9]*matrix2[6]+matrix1[10]*matrix2[10]+matrix1[11]*matrix2[14];
	outputmatrix[11]=matrix1[8]*matrix2[3]+matrix1[9]*matrix2[7]+matrix1[10]*matrix2[11]+matrix1[11]*matrix2[15];

	outputmatrix[12]=matrix1[12]*matrix2[0]+matrix1[13]*matrix2[4]+matrix1[14]*matrix2[8]+matrix1[15]*matrix2[12];
	outputmatrix[13]=matrix1[12]*matrix2[1]+matrix1[13]*matrix2[5]+matrix1[14]*matrix2[9]+matrix1[15]*matrix2[13];
	outputmatrix[14]=matrix1[12]*matrix2[2]+matrix1[13]*matrix2[6]+matrix1[14]*matrix2[10]+matrix1[15]*matrix2[14];
	outputmatrix[15]=matrix1[12]*matrix2[3]+matrix1[13]*matrix2[7]+matrix1[14]*matrix2[11]+matrix1[15]*matrix2[15];
}

void sglMultMatrix(const float *matrix) 
{
	SglContext * cc = ContextManager::Inst()->CC();
	if(cc->waitingForSglEnd)
	{
		setErrCode(SGL_INVALID_OPERATION);
		return;
	}

	float outputmatrix[16];
	const float * matrix1 = matrix;
	const float * matrix2 = cc->matrix[cc->activeMatrix];
	MultMatrix(outputmatrix, matrix1, matrix2);

	// Prekopiruje vyslednou matici do aktualni
	memcpy(cc->matrix[cc->activeMatrix], outputmatrix, 64);

	// Udržuje aktuální modelview * projection matici
	MultMatrix(cc->modelviewprojection, cc->matrix[SGL_MODELVIEW], cc->matrix[SGL_PROJECTION]);
}

void sglTranslate(float x, float y, float z) 
{
	float matrix[16];
	matrix[0] = 1; matrix[4] = 0; matrix[8] = 0;  matrix[12] = x;
	matrix[1] = 0; matrix[5] = 1; matrix[9] = 0;  matrix[13] = y;
	matrix[2] = 0; matrix[6] = 0; matrix[10] = 1; matrix[14] = z;
	matrix[3] = 0; matrix[7] = 0; matrix[11] = 0; matrix[15] = 1;

	// PRYC
	/*
	float mout[16];
	float out2[16];
	printf("\nOk inversion - %d\n", MyMath::matrixInversion(matrix, mout));
	MultMatrix(out2,matrix, mout);
	*/
	// PRYC

	sglMultMatrix(matrix);
}

void sglScale(float scalex, float scaley, float scalez) 
{
	float matrix[16];
	matrix[0] = scalex; matrix[4] = 0;		matrix[8] = 0;		 matrix[12] = 0;
	matrix[1] = 0;		matrix[5] = scaley; matrix[9] = 0;		 matrix[13] = 0;
	matrix[2] = 0;		matrix[6] = 0;		matrix[10] = scalez; matrix[14] = 0;
	matrix[3] = 0;		matrix[7] = 0;		matrix[11] = 0;		 matrix[15] = 1;

	/*
	float m[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
	float inv[16];
	MyMath::matrixInversion(m, inv);
	*/

	// PRYC
	/*
	float mout[16];
	float out2[16];
	printf("\nOk inversion - %d\n", MyMath::matrixInversion(matrix, mout));
	MultMatrix(out2,matrix, mout);
	*/
	// PRYC

	sglMultMatrix(matrix);

	// PRYC
	/*
	printf("\nOk inversion - %d\n", MyMath::matrixInversion(ContextManager::Inst()->CC()->matrix[1], mout));
	MultMatrix(out2,ContextManager::Inst()->CC()->matrix[1], mout);
	*/
	// PRYC
}

void sglRotateZ(float angle);
void sglRotate2D(float angle, float centerx, float centery) 
{	
	// Rotace kolem bodu se stredem centerx, centery
	sglTranslate(centerx, centery, 0);
	sglRotateZ(angle);
	sglTranslate(-centerx, -centery, 0);
}

void sglRotateZ(float angle) {
	float matrix[16];
	angle = - angle;
	matrix[0] = cos(angle);  matrix[4] = sin(angle); matrix[8] = 0;  matrix[12] = 0;
	matrix[1] = -sin(angle); matrix[5] = cos(angle); matrix[9] = 0;  matrix[13] = 0;
	matrix[2] = 0;			 matrix[6] = 0;			 matrix[10] = 1; matrix[14] = 0;
	matrix[3] = 0;			 matrix[7] = 0;			 matrix[11] = 0; matrix[15] = 1;

	sglMultMatrix(matrix);
}

void sglRotateY(float angle) {
	float matrix[16];
	angle = -angle;
	matrix[0] = cos(angle);  matrix[4] = 0;			 matrix[8] = sin(angle);  matrix[12] = 0;
	matrix[1] = 0;			 matrix[5] = 1;			 matrix[9] = 0;			  matrix[13] = 0;
	matrix[2] = -sin(angle); matrix[6] = 0;			 matrix[10] = cos(angle); matrix[14] = 0;
	matrix[3] = 0;			 matrix[7] = 0;			 matrix[11] = 0;		  matrix[15] = 1;

	sglMultMatrix(matrix);
}

void sglOrtho(float left, float right, float bottom, float top, float near, float far) 
{
	float matrix[16];
	matrix[0] = 2 / (right - left); matrix[4] = 0;					matrix[8] = 0;					matrix[12] = -(right + left) / (right - left);
	matrix[1] = 0;					matrix[5] = 2 / (top - bottom); matrix[9] = 0;					matrix[13] = -(top + bottom) / (top - bottom);
	matrix[2] = 0;					matrix[6] = 0;					matrix[10] = -2 / (far - near); matrix[14] = -(far + near) / (far - near);
	matrix[3] = 0;					matrix[7] = 0;					matrix[11] = 0;					matrix[15] = 1;
	
	sglMultMatrix(matrix);
}

void sglFrustum(float left, float right, float bottom, float top, float near, float far) 
{
	float A = (right + left) / (right - left);
	float B = (top + bottom) / (top - bottom);
	float C = -(far + near) / (far - near);
	float D = -(2 * far * near) / (far - near); 
	float matrix[16];
	matrix[0] = 2 * near / (right - left); matrix[4] = 0;						  matrix[8] = A;   matrix[12] = 0;
	matrix[1] = 0;						   matrix[5] = 2 * near / (top - bottom); matrix[9] = B;   matrix[13] = 0;
	matrix[2] = 0;						   matrix[6] = 0;						  matrix[10] = C;  matrix[14] = D;
	matrix[3] = 0;						   matrix[7] = 0;						  matrix[11] = -1; matrix[15] = 0;
		
	sglMultMatrix(matrix);
}

void sglViewport(int x, int y, int width, int height) 
{
	SglContext * cc = ContextManager::Inst()->CC();
	
	// Kontrola moznych chyb
	if(width <= 0 || height <= 0) { setErrCode(SGL_INVALID_VALUE ); return; }
	if(cc->waitingForSglEnd) {		setErrCode(SGL_INVALID_OPERATION); return; }

	// Nastaveni viewportu
	cc->viewportHeight = height;
	cc->viewportWidth = width;
	cc->viewportX = x;
	cc->viewportY = y;
}

//---------------------------------------------------------------------------
// Attribute functions
//---------------------------------------------------------------------------

void sglColor3f(float r, float g, float b) 
{
	// Zmena aktualni barvy
	ContextManager::Inst()->CC()->currentColor.Set(r, g, b);
}

void sglAreaMode(sglEAreaMode mode) {
	// Kontrola moznych chyb
	if(mode != SGL_POINT && mode != SGL_FILL && mode != SGL_LINE) { setErrCode(SGL_INVALID_ENUM); return; }
	if(ContextManager::Inst()->BetweenBeginAndEnd()) {				setErrCode(SGL_INVALID_OPERATION); return; }

	// Nastaveni vyplnovaciho modu pro polygony
	ContextManager::Inst()->CC()->areaMode = mode;
}

void sglPointSize(float size) {
	// Kontrola moznych chyb
	if(size <= 0.0f) {									setErrCode(SGL_INVALID_VALUE); return; }
	if(ContextManager::Inst()->BetweenBeginAndEnd()) {  setErrCode(SGL_INVALID_OPERATION); return; }

	// Nastaveni sirky bodu
	ContextManager::Inst()->CC()->pointSize = size;
}

void sglEnable(sglEEnableFlags cap) {
	//Kontrola moznych chyb
	if(cap != SGL_DEPTH_TEST) {						   setErrCode(SGL_INVALID_ENUM); return; }
	if(ContextManager::Inst()->BetweenBeginAndEnd()) { setErrCode(SGL_INVALID_OPERATION); return; }

	// Vypnuti hloubkoveho testu
	ContextManager::Inst()->CC()->flagbits |= cap;
}

void sglDisable(sglEEnableFlags cap) {
	//Kontrola moznych chyb
	if(cap != SGL_DEPTH_TEST) {						   setErrCode(SGL_INVALID_ENUM); return; }
	if(ContextManager::Inst()->BetweenBeginAndEnd()) { setErrCode(SGL_INVALID_OPERATION); return; }

	// Vypnuti hloubkoveho testu
	ContextManager::Inst()->CC()->flagbits &= ~cap;
}


//---------------------------------------------------------------------------
// RayTracing oriented functions
//---------------------------------------------------------------------------

void sglBeginScene() 
{
	//sglClear(SGL_COLOR_BUFFER_BIT);
	if(ContextManager::Inst()->BetweenBeginAndEnd()) {  setErrCode(SGL_INVALID_OPERATION); return; }

	if(ContextManager::Inst()->CC()->sglBeginScene == true)
	{
		// error
	} else
		ContextManager::Inst()->CC()->clearScene();
}

void sglEndScene() 
{
	if(ContextManager::Inst()->BetweenBeginAndEnd()) {  setErrCode(SGL_INVALID_OPERATION); return; }

	ContextManager::Inst()->CC()->sglBeginScene = false;
}

void sglSphere(const float x,
			   const float y,
			   const float z,
			   const float radius) {
	if(!ContextManager::Inst()->BetweenBeginAndEndScene() ||
		ContextManager::Inst()->BetweenBeginAndEnd()) {  setErrCode(SGL_INVALID_OPERATION); return; }

	ContextManager::Inst()->CC()->addSphere(x, y, z, radius);
}

void sglMaterial(const float r,
				 const float g,
				 const float b,
				 const float kd,
				 const float ks,
				 const float shine,
				 const float T,
				 const float ior) 
{
	if(!ContextManager::Inst()->BetweenBeginAndEndScene()) {  setErrCode(SGL_INVALID_OPERATION); return; }

	ContextManager::Inst()->CC()->addMaterial(r, g, b, kd, ks, shine, T, ior);
}

void sglEmissiveMaterial(
						 const float r,
						 const float g,
						 const float b,
						 const float c0,
						 const float c1,
						 const float c2
						 )
{
	if(!ContextManager::Inst()->BetweenBeginAndEndScene()) {  setErrCode(SGL_INVALID_OPERATION); return; }

	ContextManager::Inst()->CC()->addEmissiveMaterial(r, g, b, c0, c1, c2);
}

void sglPointLight(const float x,
				   const float y,
				   const float z,
				   const float r,
				   const float g,
				   const float b) 
{
	if(!ContextManager::Inst()->BetweenBeginAndEndScene()) {  setErrCode(SGL_INVALID_OPERATION); return; }

	ContextManager::Inst()->CC()->addPointLight(x, y, z, r, g, b);
}

void sglRayTraceScene() 
{

	ContextManager::Inst()->CC()->raytrace();
	//sglRasterizeScene();
}

void sglRasterizeScene() {
	ContextManager::Inst()->CC()->rasterize();
}

void sglEnvironmentMap(const int width,
					   const int height,
					   float *texels)
{
	
	ContextManager::Inst()->CC()->addEnvironmentMap(width, height, texels);
}
