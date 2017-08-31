#ifndef TURTLEBOT_PLANNER_ONTOLOGY_VECTOR_2D
#define TURTLEBOT_PLANNER_ONTOLOGY_VECTOR_2D

#include <ostream>
#include <math.h>

struct Vector2D
{
	Vector2D(float x, float y)
		: x_(x), y_(y)
	{
		
	}
	
	Vector2D()
		: x_(0), y_(0)
	{
		
	}
	
	static float dot(const Vector2D& v1, const Vector2D& v2);
	
	float getLength() const 
	{
		return sqrt(x_ * x_ + y_ * y_);
	}
	
	const Vector2D operator-(const Vector2D& v) const
	{
		return Vector2D(*this) -= v;
	}
	
	Vector2D& operator-=(const Vector2D& v)
	{
		x_ -= v.x_;
		y_ -= v.y_;
		return *this;
	}
	
	const Vector2D operator+(const Vector2D& v) const
	{
		return Vector2D(*this) += v;
	}
	
	Vector2D& operator+=(const Vector2D& v)
	{
		x_ += v.x_;
		y_ += v.y_;
		return *this;
	}
	
	Vector2D& operator*=(float f)
	{
		x_ *= f;
		y_ *= f;
		return *this;
	}
	
	Vector2D operator*(float f) const
	{
		return Vector2D(*this) *= f;
	}
	
	Vector2D& operator/=(float f)
	{
		x_ /= f;
		y_ /= f;
		return *this;
	}
	
	Vector2D operator/(float f) const
	{
		return Vector2D(*this) /= f;
	}
	
	float getDistance(const Vector2D& v) const
	{
		return sqrt((x_ - v.x_) * (x_ - v.x_) + (y_ - v.y_) * (y_ - v.y_));
	}
	
	void normalise()
	{
		x_ /= getLength();
		y_ /= getLength();
	}
	
	void rotate(float angle)
	{
		float cs = cos(angle);
		float sn = sin(angle);
		float tmp_x = x_;
		float tmp_y = y_;
		x_ = tmp_x * cs - tmp_y * sn; 
		y_ = tmp_x * sn + tmp_y * cs;
	}
	
	bool operator<(const Vector2D& other) const
	{
		if (other.x_ < x_) return true;
		else if (other.x_ == x_ && other.y_ < y_) return true;
		return false;
	}
	
	bool operator>(const Vector2D& other) const
	{
		if (other.x_ > x_) return true;
		else if (other.x_ == x_ && other.y_ > y_) return true;
		return false;
	}
	
	bool operator==(const Vector2D& other) const
	{
		return other.x_ == x_ && other.y_ == y_;
	}
	
	/**
	 * Get the intersection between two lines and put the result in @ref{p1}.
	 * @param p1_begin A point on the first line.
	 * @param p1_end A point on the first line, not equal to @ref{p1_begin}.
	 * @param p2_begin A point on the second line.
	 * @param p2_end A point on the second line, not equal to @ref{p2_begin}.
	 * @param p1 The location where the intersection will be stored, if any.
	 * @return True if an intersection was found, false otherwise; this means that the lines are parallel.
	 */
	static bool getIntersection(const Vector2D& p1_begin, const Vector2D& p1_end, const Vector2D& p2_begin, Vector2D& p2_end, Vector2D& p1);

	/**
	 * Get the intersection between two line segments and put the result in @ref{p1}.
	 * @param p1_begin The begin point of the first line segment.
	 * @param p1_end The end point of the first line segment, not equal to @ref{p1_begin}.
	 * @param p2_begin The being point on the second line segment.
	 * @param p2_end The end point of the second line segment, not equal to @ref{p2_begin}.
	 * @param p1 The location where the intersection will be stored, if any.
	 * @return True if an intersection was found, false otherwise; this means that the lines are parallel or that the intersection falls outside the segments.
	 */
	static bool getIntersectionSegments(const Vector2D& p1_begin, const Vector2D& p1_end, const Vector2D& p2_begin, Vector2D& p2_end, Vector2D& p1);
	
	float x_, y_;
};

const Vector2D operator*(float f, const Vector2D& v);
const Vector2D operator/(float f, const Vector2D& v);

std::ostream& operator<<(std::ostream& os, const Vector2D& vector);

#endif
