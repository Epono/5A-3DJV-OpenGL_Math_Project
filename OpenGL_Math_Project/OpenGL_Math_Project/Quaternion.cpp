#include <glm/matrix.hpp>
#include <glm/vec3.hpp>
#include "Quaternion.h"
#include <math.h>

Quaternion::Quaternion()
	: x_(0.), y_(0.), z_(0.), w_(0.)
{
}

Quaternion::Quaternion(float x_, float y_, float z, float w)
	: x_(x_), y_(y_), z_(z), w_(w)
{
}

inline Quaternion::Quaternion(const glm::mat4& mat)
{
	*this = mat;
}

inline bool Quaternion::operator==(const Quaternion& other) const
{
	return ((x_ == other.x_) &&
			(y_ == other.y_) &&
			(z_ == other.z_) &&
			(w_ == other.w_));
}

inline bool Quaternion::operator!=(const Quaternion& other) const
{
	return !((*this) == other);
}

Quaternion& Quaternion::operator=(const Quaternion& other)
{
	if(this != &other)
	{
		x_ = other.x_;
		y_ = other.y_;
		z_ = other.z_;
		w_ = other.w_;
	}
	return *this;
}

Quaternion& Quaternion::operator=(const glm::mat4& m)
{
	const float diag = m[0].operator[](0) + m[1].operator[](1) + m[2].operator[](2) + 1.0f;

	if(diag > 0.0f)
	{
		const float scale = sqrtf(diag) * 2.0f;

		x_ = (m[2][1] - m[1][2]) / scale;
		y_ = (m[0][2] - m[2][0]) / scale;
		z_ = (m[1][0] - m[0][1]) / scale;
		w_ = 0.25f * scale;
	}
	else
	{
		if(m[0][0] > m[1][1] && m[0][0] > m[2][2])
		{
			const float scale = sqrtf(1.0f + m[0][0] - m[1][1] - m[2][2]) * 2.0f;

			x_ = 0.25f * scale;
			y_ = (m[0][1] + m[1][0]) / scale;
			z_ = (m[0][2] + m[2][0]) / scale;
			w_ = (m[2][1] - m[1][2]) / scale;
		}
		else if(m[1][1] > m[2][2])
		{
			const float scale = sqrtf(1.0f + m[1][1] - m[0][0] - m[2][2]) * 2.0f;

			x_ = (m[0][1] + m[1][0]) / scale;
			y_ = 0.25f * scale;
			z_ = (m[1][2] + m[2][1]) / scale;
			w_ = (m[1][0] - m[0][1]) / scale;
		}
		else
		{
			const float scale = sqrt(1.0f + m[2][2] - m[0][0] - m[1][1]) * 2.0f;

			x_ = (m[0][2] + m[2][0]) / scale;
			y_ = (m[1][2] + m[2][1]) / scale;
			z_ = 0.25f * scale;
			w_ = (m[1][0] - m[0][1]) / scale;
		}
	}

	return normalize();
}

inline Quaternion Quaternion::operator+(const Quaternion& b) const
{
	return Quaternion(x_ + b.x_, y_ + b.y_, z_ + b.z_, w_ + b.w_);
}

Quaternion Quaternion::operator*(const Quaternion& other) const
{
	Quaternion tmp;

	tmp.x_ = (other.w_ * x_) + (other.x_ * w_) + (other.y_ * z_) - (other.z_ * y_);
	tmp.y_ = (other.w_ * y_) + (other.y_ * w_) + (other.z_ * x_) - (other.x_ * z_);
	tmp.z_ = (other.w_ * z_) + (other.z_ * w_) + (other.x_ * y_) - (other.y_ * x_);
	tmp.w_ = (other.w_ * w_) - (other.x_ * x_) - (other.y_ * y_) - (other.z_ * z_);

	return tmp;
}

inline Quaternion Quaternion::operator*(float s) const
{
	return Quaternion(s*x_, s*y_, s*z_, s*w_);
}

inline Quaternion& Quaternion::operator*=(const Quaternion& other)
{
	return (*this = other * (*this));
}

inline Quaternion& Quaternion::operator*=(float s)
{
	x_ *= s;
	y_ *= s;
	z_ *= s;
	w_ *= s;
	return *this;
}

inline float Quaternion::dotProduct(const Quaternion& q2) const
{
	return (x_ * q2.x_) + (y_ * q2.y_) + (z_ * q2.z_) + (w_ * q2.w_);
}

inline Quaternion& Quaternion::set(float x_, float y_, float z, float w)
{
	x_ = x_;
	y_ = y_;
	z_ = z;
	w_ = w;
	return *this;
}

inline Quaternion& Quaternion::set(const Quaternion& quat)
{
	return (*this = quat);
}

inline bool Quaternion::equals(const Quaternion& other, const float tolerance) const
{
	return (abs(x_ - other.x_) <= tolerance) &&
		(abs(y_ - other.y_) <= tolerance) &&
		(abs(z_ - other.z_) <= tolerance) &&
		(abs(w_ - other.w_) <= tolerance);
}

float Quaternion::length() const
{
	return sqrt(x_ * x_ + y_ * y_ +
				z_ * z_ + w_ * w_);
}

Quaternion Quaternion::normalize()
{
	Quaternion Q;
	float L = this->length();

	Q.x_ = x_ / L;
	Q.y_ = y_ / L;
	Q.z_ = z_ / L;
	Q.w_ = w_ / L;

	return Q;
}

Quaternion Quaternion::conjugate()
{
	Quaternion Q;
	Q.x_ = -x_;
	Q.y_ = -y_;
	Q.z_ = -z_;

	return Q;
}

//! axis must be unit length, angle in radians
inline Quaternion& Quaternion::fromAngleAxis(float angle, const glm::vec3& axis)
{
	const float HalfAngle = 0.5f * angle;
	const float Sin = sinf(HalfAngle);
	w_ = cosf(HalfAngle);
	x_ = Sin * axis.x;
	y_ = Sin * axis.y;
	z_ = Sin * axis.z;
	return *this;
}

// set Quaternion to identity_
inline Quaternion& Quaternion::makeIdentity()
{
	w_ = 1.f;
	x_ = 0.f;
	y_ = 0.f;
	z_ = 0.f;
	return *this;
}

glm::mat4 Quaternion::toRotationMatrix()
{
	this->normalize();
	float qxx = x_ * x_;
	float qyy = y_ * y_;
	float qzz = z_ * z_;
	float qxz = x_ * z_;
	float qxy = x_ * y_;
	float qyz = y_ * z_;
	float qwx = w_ * x_;
	float qwy = w_ * y_;
	float qwz = w_ * z_;

	glm::mat4 result(
		(1 - 2 * (qyy + qzz)),
		(2 * (qxy + qwz)),
		(2 * (qxz - qwy)),
		0,

		(2 * (qxy - qwz)),
		(1 - 2 * (qxx + qzz)),
		(2 * (qyz + qwx)),
		0,

		(2 * (qxz + qwy)),
		(2 * (qyz - qwx)),
		(1 - 2 * (qxx + qyy)),
		0,

		0, 0, 0, 1
		);

	return result;
}