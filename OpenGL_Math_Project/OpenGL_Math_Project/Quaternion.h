#ifndef __QUATERNION_H__
#define __QUATERNION_H__

class Quaternion
{
public:
	Quaternion();
	Quaternion(float, float, float, float);
	Quaternion(const glm::mat4&);

	bool operator==(const Quaternion&) const;
	bool operator!=(const Quaternion&) const;

	Quaternion& operator=(const Quaternion&);
	Quaternion& operator=(const glm::mat4&);

	Quaternion operator+(const Quaternion&) const;

	Quaternion operator*(const Quaternion&) const;
	Quaternion operator*(float) const;

	Quaternion& operator*=(const Quaternion&);
	Quaternion& operator*=(float);

	inline float dotProduct(const Quaternion&) const;

	inline Quaternion& set(float, float, float, float);
	inline Quaternion& set(const Quaternion&);

	inline bool equals(const Quaternion& ,
					   const float = 0.001) const;

	float length() const;

	Quaternion normalize();

	Quaternion conjugate();

	//Mat4 getMatrix() const;

	Quaternion& fromAngleAxis(float, const glm::vec3&);

	Quaternion& makeIdentity();

	glm::mat4 toMatrixUnit();


	float x_, y_, z_, w_;
};

#endif //__QUATERNION_H__