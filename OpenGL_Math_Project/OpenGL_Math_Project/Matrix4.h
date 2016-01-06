#ifndef __MATRIX4_H__
#define __MATRIX4_H__

#include "Vector.h"

class Mat4
{
public:
	Mat4();
	Mat4(const Vec3 &, const Vec3 &, const Vec3 &);
	Mat4(const Mat4 &);

	Mat4& operator = (const Mat4 &M);

	Mat4 operator*(const Mat4 &Right) const;
	Mat4 operator+(const Mat4 &Right) const;
	Mat4 operator-(const Mat4 &Right) const;

	double Determinant() const;
	
	inline double operator[](int ind)
	{
		return _Entries[ind];
	}
	inline const double operator[](int ind) const
	{
		return _Entries[ind];
	}

private:
	double _Entries[16];
};

#endif //__MATRIX4_H__