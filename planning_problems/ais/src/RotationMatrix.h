#ifndef TURTLEBOT_PLANNER_ONTOLOGY_ROTATION_MATRIX_H
#define TURTLEBOT_PLANNER_ONTOLOGY_ROTATION_MATRIX_H

class Vector2D;
class Vector3D;

class RotationMatrix
{
public:
	RotationMatrix(float m00, float m01, float m10, float m11);
	
	Vector2D rotate(const Vector2D& vector) const;
	Vector3D rotate(const Vector3D& vector) const;
	
	bool operator==(const RotationMatrix& other) const;
	
private:
	float m00_, m01_, m10_, m11_;
	
	friend std::ostream& operator<<(std::ostream& os, const RotationMatrix& rotation);
};

std::ostream& operator<<(std::ostream& os, const RotationMatrix& rotation);

#endif
