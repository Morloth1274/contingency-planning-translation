#ifndef TURTLEBOT_PLANNER_ONTOLOGY_VECTOR_3D
#define TURTLEBOT_PLANNER_ONTOLOGY_VECTOR_3D

#include <ostream>
#include <math.h>

#include "Vector2D.h"

struct Vector3D
{
	Vector3D(float x, float y, float z)
		: x_(x), y_(y), z_(z)
	{
		
	}
	
	Vector3D()
		: x_(0), y_(0)
	{
		
	}
	
	static float dot(const Vector3D& v1, const Vector3D& v2);
	
	float getLength() const 
	{
		return sqrt(x_ * x_ + y_ * y_ + z_ * z_);
	}
	
	const Vector3D operator-(const Vector3D& v) const
	{
		return Vector3D(*this) -= v;
	}
	
	Vector3D& operator-=(const Vector3D& v)
	{
		x_ -= v.x_;
		y_ -= v.y_;
		z_ -= v.z_;
		return *this;
	}
	
	const Vector3D operator+(const Vector3D& v) const
	{
		return Vector3D(*this) += v;
	}
	
	Vector3D& operator+=(const Vector3D& v)
	{
		x_ += v.x_;
		y_ += v.y_;
		z_ += v.z_;
		return *this;
	}
	
	Vector3D& operator*=(float f)
	{
		x_ *= f;
		y_ *= f;
		z_ *= f;
		return *this;
	}
	
	Vector3D& operator/=(float f)
	{
		x_ /= f;
		y_ /= f;
		z_ /= f;
		return *this;
	}
	
	Vector3D operator*(float f) const
	{
		return Vector3D(*this) *= f;
	}
	
	Vector3D operator/(float f) const
	{
		return Vector3D(*this) /= f;
	}
	
	float getDistance(const Vector3D& v) const
	{
		return sqrt((x_ - v.x_) * (x_ - v.x_) + (y_ - v.y_) * (y_ - v.y_) + (z_ - v.z_) * (z_ - v.z_));
	}
	
	void normalise()
	{
		x_ /= getLength();
		y_ /= getLength();
		z_ /= getLength();
	}
	
	bool operator==(const Vector3D& other) const
	{
		return other.x_ == x_ && other.y_ == y_ && other.z_ == z_;
	}
	
	operator Vector2D()
	{
		return Vector2D(x_, y_);
	}
	
	/**
	 * Get the distance between two line segments.
	 * @param p1_begin One point of the first line segment.
	 * @param p1_end Second point of the second line segment.
	 * @param p2_begin One point of the second line segment.
	 * @param p2_end Second point of the second line segment.
	 * @return The distance between both line segments.
	 */
	static float getDistance(const Vector3D& p1_begin, const Vector3D& p1_end, const Vector3D& p2_begin, const Vector3D& p2_end);
	
	/**
	 * Get the distance from a point to a line segment.
	 */
	static float getDistance(const Vector3D& point, const Vector3D& begin, const Vector3D& end);
	
	
	float x_, y_, z_;
};

const Vector3D operator*(float f, const Vector3D& v);
const Vector3D operator/(float f, const Vector3D& v);

std::ostream& operator<<(std::ostream& os, const Vector3D& vector);

#endif
