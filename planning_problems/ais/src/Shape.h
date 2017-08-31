#ifndef TURTLEBOT_PLANNER_ONTOLOGY_SHAPE_H
#define TURTLEBOT_PLANNER_ONTOLOGY_SHAPE_H

#include <string>
#include <vector>
#include <map>

#include "RotationMatrix.h"
#include "Vector3D.h"

class Face;
class OntolAccess;

/**
 * The ontology stores scenes which are based on the known shapes an observations so far.
 */
class Shape
{
public:
	/**
	 * Load the shape from the ontology.
	 */
	Shape(OntolAccess& oa, const std::string& shape_name, const Vector3D& location, const RotationMatrix& rotation, const std::vector<std::string>& observed_faces);
	
	/**
	 * Load the shape given the set of faces.
	 */
	Shape(const Vector3D& location, const RotationMatrix& rotation);
	
	/**
	 * Copy constructor.
	 */
	Shape(const Shape& shape);
	
	void addFace(const Face& face);
	
	bool isOpenShape() const;
	
	~Shape();
	
	const Vector3D& getLocation() const { return location_; }
	const RotationMatrix& getRotationMatrix() const { return rotation_; }
	const std::vector<const Face*>& getFaces() const { return faces_; }
	bool isTarget() const { return is_target_; }
	
	/**
	 * Find a face with the name @ref{face_name} in this shape.
	 * @param face_name The name of the face (as appears in PDDL).
	 * @return The Face object if it was found, NULL otherwise.
	 */
	const Face* getFace(const std::string& face_name) const;
	
	/**
	 * For every shape we determine the interesting points as those that are the intersection between
	 * the normal vectors of two faces. These lines should not be intersected by any other faces that.
	 */
	const std::vector<Vector2D>& getInterestingPoints() const { return interesting_points_; }
	
	/**
	 * Get the name as the shape is stored in the ontology.
	 * @return The name as it is stored in the ontology.
	 */
	const std::string& getOntologyName() const { return ontology_name_; }
private:
	
	void calculateInterestingPoints();
	
	std::string ontology_name_;
	Vector3D location_;          // The transformation of the shape in the global reference frame.
	RotationMatrix rotation_;    // The rotation matrix of the shame in the global reference frame.
	bool is_target_;             // Property that states whether this shape is a target.
	
	std::vector<const Face*> faces_;
	std::vector<Vector2D> interesting_points_;
	
	static std::map<std::string, const Shape*> cached_shapes_;
	
	friend std::ostream& operator<<(std::ostream& os, const Shape& shape);
};

std::ostream& operator<<(std::ostream& os, const Shape& shape);

#endif
