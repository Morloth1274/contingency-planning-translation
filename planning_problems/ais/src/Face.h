#ifndef TURTLEBOT_PLANNER_ONTOLOGY_FACE_H
#define TURTLEBOT_PLANNER_ONTOLOGY_FACE_H

#include <string>
#include <vector>
#include <map>
#include "Vector3D.h"

class OntolAccess;
class Shape;

/**
 * The ontology stores scenes which are based on the known shapes an observations so far.
 */
class Face
{
public:
	/**
	 * Construct a face here.
	 */
	Face(Shape& shape, OntolAccess& oa, const std::string& face_name, bool face_is_observed);
	
	Face(Shape& shape, const Face& face, bool face_is_observed);
	
	static Face* getFace(Shape& shape, OntolAccess& oa, const std::string& face_name, bool face_is_observed);
	static Face* getFace(Shape& shape, const Face& face, bool face_is_observed);
	static const std::vector< Face*>& getFaces();
	static void deleteFaces();
	
	bool isSimilarTo(const Face& other) const;
	bool isSimilarTo(int other_hue, int other_saturation, int other_value, const Vector3D& other_p1_transformed, const Vector3D& other_p2_transformed) const;
	
	const Vector3D& getP1() const { return p1_transformed_; }
	const Vector3D& getP2() const { return p2_transformed_; }
	
	int getHue() const { return hue_; }
	int getSaturation() const { return saturation_; }
	int getValue() const { return value_; }
	
	const Vector3D& getNormal() const { return normal_vector_transformed_; }
	
	bool isObserved() const { return has_been_observed_; }	
	const std::string& getName() const { return face_name_; }
	std::string getPDDLName() const { return pddl_name_; }
	
	bool ignoreFace() const { return ignore_face_; }
private:
	
	std::vector<Shape*> shapes_;
	std::string face_name_;
	std::string pddl_name_;
	
	int hue_, saturation_, value_;
	
	Vector3D p1_, p2_;
	Vector3D p1_transformed_, p2_transformed_;
	
	Vector3D normal_vector_;
	Vector3D normal_vector_transformed_;
	
	bool ignore_face_;       // We ignore faces that are too small.
	bool has_been_observed_; // Flag to indicate whether this face has already been observed.
	
	static std::vector<Face*> faces_;
	//static std::map<const Face*, std::string> face_to_pddl_name_; // In order to give faces that are similar we store the name globably.
};

std::ostream& operator<<(std::ostream& os, const Face& face);

#endif
