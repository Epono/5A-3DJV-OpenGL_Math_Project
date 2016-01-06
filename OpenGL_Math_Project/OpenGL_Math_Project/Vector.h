#ifndef __VECTOR_H__
#define __VECTOR_H__

class Vec3
{
public:
	Vec3(double x, double y, double z)
		: x_(x), y_(y), z_(z) {}

	double x_, y_, z_;
};

class Vec4
{
public:
	Vec4(double x, double y, double z, double w)
		: x_(x), y_(y), z_(z), w_(w) {}

	double x_, y_, z_, w_;
};

#endif //__VECTOR_H__