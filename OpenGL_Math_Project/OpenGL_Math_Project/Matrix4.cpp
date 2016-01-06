#include "Matrix4.h"

Mat4::Mat4()
{
	for (unsigned int ind = 0; ind < 16; ind++)
		_Entries[ind] = 0.0;
}

Mat4::Mat4(const Vec3 &V0, const Vec3 &V1, const Vec3 &V2)
{
	_Entries[0] = V0.x_;
	_Entries[4] = V0.y_;
	_Entries[8] = V0.z_;
	_Entries[12] = 0.0;

	_Entries[1] = V1.x_;
	_Entries[5] = V1.y_;
	_Entries[9] = V1.z_;
	_Entries[13] = 0.0;

	_Entries[2] = V2.x_;
	_Entries[6] = V2.y_;
	_Entries[10] = V2.z_;
	_Entries[14] = 0.0;

	_Entries[3] = 0.0;
	_Entries[7] = 0.0;
	_Entries[11] = 0.0;
	_Entries[15] = 1.0;
}

Mat4::Mat4(const Mat4 &M)
{
	for (unsigned int ind = 0; ind < 16; ind++)
		_Entries[ind] = M[ind];
}

Mat4& Mat4::operator=(const Mat4 &M)
{
	if (this != &M)
	{
		for (unsigned int ind = 0; ind < 16; ind++)
			_Entries[ind] = M[ind];
	}
	return (*this);
}

Mat4 Mat4::operator*(const Mat4 &Right) const
{
	Mat4 Result;
	for (unsigned int i = 0; i < 4; i++)
	{
		for (unsigned int j = 0; j < 4; j++)
		{
			double Total = 0.0;
			for (unsigned int h = 0; h < 4; h++)
				Total += (*this)[i*4+h] * Right[h*4+j];
			Result._Entries[i * 4 + j] = Total;
		}
	}
	return Result;
}

Mat4 Mat4::operator+(const Mat4 &Right) const
{
	Mat4 Result;
	for (unsigned int i = 0; i < 4; i++)
		Result._Entries[i] = (*this)[i] + Right[i];

	return Result;
}

Mat4 Mat4::operator-(const Mat4 &Right) const
{
	Mat4 Result;
	for (unsigned int i = 0; i < 4; i++)
	{
		Result._Entries[i] = (*this)[i] - Right[i];
	}
	return Result;
}