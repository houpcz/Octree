#ifndef _SGLSTRUCTURES_H_
#define _SGLSTRUCTURES_H_

// Struktura pro zachyceni tri slozek barvy
struct Color {
	public :
		Color() { r = 0.0f; g = 0.0f; b = 0.0f; };
		Color(float n_r, float n_g, float n_b) { r = n_r; g = n_g; b = n_b; };
		void Set(float n_r, float n_g, float n_b) { r = n_r; g = n_g; b = n_b; };
		
		inline Color operator/(const float k) const
		{
			return Color(r / k, g / k, b / k);
		}

		inline Color operator*(const float k) const
		{
			return Color(r * k, g * k, b * k);
		}

		inline Color operator+(const Color& B) const
		{
			return Color(r + B.r, g + B.g, b + B.b);
		}

		float r, g, b;
};

// Struktura pro homogenni souradnice
struct Vector4 {
	float x;
	float y;
	float z;
	float w;
	Vector4(float n_x, float n_y, float n_z, float n_w) { x = n_x; y = n_y; z = n_z; w = n_w;};
	Vector4(float n_x, float n_y) { x = n_x; y = n_y; z = 1; w = 1;};
	Vector4() { x = 0.0f; y = 0.0f; z = 0.0f; w = 0.0f; }
};

#include <cmath>

// Struktura svetove souradnice
struct Vector3 {
	float x;
	float y;
	float z;
	Vector3(float n_x, float n_y, float n_z) { x = n_x; y = n_y; z = n_z;};
	Vector3(float n_x, float n_y) { x = n_x; y = n_y; z = 1;};
	Vector3() { x = 0.0f; y = 0.0f; z = 0.0f; }
	Vector3(const Vector4 vec) { x = vec.x; y = vec.y; z = vec.z; }

	float Length()
	{
		return sqrt(x*x + y*y + z*z);
	}

	inline void Normalize()
	{
		float length = sqrt(x*x + y*y + z*z);
		x /= length;
		y /= length;
		z /= length;
	}

	inline Vector3 operator- () 
	{
		return Vector3(-x, -y, -z);
	}

	inline Vector3 & operator/= (float A) 
	{
		float a = 1.0f/A; x*=a;y*=a;z*=a;
		return *this;
	}
	inline Vector3 operator-(const Vector3& B) const
	{
		return Vector3(x - B.x, y - B.y, z - B.z);
	}
	inline Vector3 operator+(const Vector3& B) const
	{
		return Vector3(x + B.x, y + B.y, z + B.z);
	}
	inline Vector3 operator/(const float k) const
	{
		return Vector3(x / k, y / k, z / k);
	}
	inline Vector3 operator*(const float k) const
	{
		return Vector3(x * k, y * k, z * k);
	}
	inline friend float DotProd(const Vector3& A, const Vector3& B)
	{
		return A.x * B.x + A.y * B.y + A.z * B.z;
	}
	inline friend Vector3 CrossProd (const Vector3& A, const Vector3& B)
	{
		return Vector3(A.y * B.z - A.z * B.y, A.z * B.x - A.x * B.z, A.x * B.y - A.y * B.x);
	}
};

#include <string.h>
class MyMath {
	public :
		static bool matrixInversion(float * matrixIn, float * matrixOut)
		{
			int indxc[4],indxr[4],ipiv[4];
			double x[4][4];
			
			for(int loop1 = 0; loop1 < 4; loop1++)
			{
				for(int loop2 = 0; loop2 < 4; loop2++)
				{
					x[loop1][loop2] = matrixIn[loop2 * 4 + loop1];
				}
			}
			//memcpy(x, matrixIn, 64);

			int i,icol,irow,j,k,l,ll,n;
			float big,dum,pivinv,temp;
			// satisfy the compiler
			icol = irow = 0;
		
			// the size of the matrix
			n = 4;
		
			for ( j = 0 ; j < n ; j++) /* zero pivots */
			  ipiv[j] = 0;
		
			for ( i = 0; i < n; i++)
			  {
				big = 0.0;
				for (j = 0 ; j < n ; j++)
				  if (ipiv[j] != 1)
					for ( k = 0 ; k<n ; k++)
					  {
						if (ipiv[k] == 0)
						  {
							if (fabs(x[k][j]) >= big)
							  {
								big = fabs(x[k][j]);
								irow = j;
								icol = k;
							  }
						  }
						else
						  if (ipiv[k] > 1)
							return 1; /* singular matrix */
					  }
				++(ipiv[icol]);
				if (irow != icol)
				  {
					for ( l = 0 ; l<n ; l++)
					  {
						temp = x[l][icol];
						x[l][icol] = x[l][irow];
						x[l][irow] = temp;
					  }
				  }
				indxr[i] = irow;
				indxc[i] = icol;
				if (x[icol][icol] == 0.0)
				  return false; /* singular matrix */
				
				pivinv = 1.0 / x[icol][icol];
				x[icol][icol] = 1.0 ;
				for ( l = 0 ; l<n ; l++)
				  x[l][icol] = x[l][icol] * pivinv ;
				
				for (ll = 0 ; ll < n ; ll++)
				  if (ll != icol)
					{
					  dum = x[icol][ll];
					  x[icol][ll] = 0.0;
					  for ( l = 0 ; l<n ; l++)
						x[l][ll] = x[l][ll] - x[l][icol] * dum ;
					}
			  }
			for ( l = n; l--; )
			  {
				if (indxr[l] != indxc[l])
				  for ( k = 0; k<n ; k++)
					{
					  temp = x[indxr[l]][k];
					  x[indxr[l]][k] = x[indxc[l]][k];
					  x[indxc[l]][k] = temp;
					}
			  }
		
			//memcpy(matrixOut, x, 64);
			for(int loop1 = 0; loop1 < 4; loop1++)
				{
					for(int loop2 = 0; loop2 < 4; loop2++)
					{
						matrixOut[loop2 * 4 + loop1] = x[loop1][loop2];
					}
				}
			return true ; // matrix is regular .. inversion has been succesfull
		}
};

#endif