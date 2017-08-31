#include <limits>

#include <turtlebot_planner/Ontology/Face.h>
#include <turtlebot_planner/Ontology/Shape.h>
#include <turtlebot_planner/Ontology/RotationMatrix.h>

#include <ontology_db/ontol_access.h>

std::vector<Face*> Face::faces_;
//std::map<const Face*, std::string> Face::face_to_pddl_name_;

const std::vector< Face*>& Face::getFaces() { return faces_; }

void Face::deleteFaces()
{
	for (std::vector<Face*>::const_iterator ci = faces_.begin(); ci != faces_.end(); ++ci)
	{
		delete *ci;
	}
	faces_.clear();
}

Face* Face::getFace(Shape& shape, const Face& face, bool face_is_observed)
{
	Face* new_face = new Face(shape, face, face_is_observed);
	if (new_face->ignore_face_)
	{
		delete new_face;
		return NULL;
	}
	
	// Check if this face is similar to an existing face. If so, we take that face's name!
	bool found_similar_face = false;
	for (std::vector<Face*>::const_iterator ci = faces_.begin(); ci != faces_.end(); ++ci)
	{
		Face* face = *ci;
		if (face->isSimilarTo(*new_face))
		{
			delete new_face;
			face->shapes_.push_back(&shape);
			return face;
		}
	}
	
	std::stringstream ss;
	ss << "FACE" << faces_.size();
	new_face->pddl_name_ = ss.str();
	faces_.push_back(new_face);
	return new_face;
}

Face* Face::getFace(Shape& shape, OntolAccess& oa, const std::string& face_name, bool face_is_observed)
{
	Face* new_face = new Face(shape, oa, face_name, face_is_observed);
	if (new_face->ignore_face_)
	{
		delete new_face;
		return NULL;
	}
	
	// Check if this face is similar to an existing face. If so, we take that face's name!
	bool found_similar_face = false;
	for (std::vector<Face*>::const_iterator ci = faces_.begin(); ci != faces_.end(); ++ci)
	{
		Face* face = *ci;
		if (face->isSimilarTo(*new_face))
		{
			delete new_face;
			face->shapes_.push_back(&shape);
			return face;
		}
	}
	
	std::stringstream ss;
	ss << "FACE" << faces_.size();
	new_face->pddl_name_ = ss.str();
	faces_.push_back(new_face);
	return new_face;
}

Face::Face(Shape& shape, OntolAccess& oa, const std::string& face_name, bool face_is_observed)
	: face_name_(face_name), has_been_observed_(face_is_observed)
{
	//std::cout << "[Face::Face] Create a face with the name '" << face_name << "'" << std::endl;
	std::string colour = oa.getStringUnique(face_name, "oslshape:'hasColor'");
	hue_ = oa.getIntUnique(colour, "oslshape:'colorHue'");
	saturation_ = oa.getIntUnique(colour, "oslshape:'colorSaturation'");
	value_ = oa.getIntUnique(colour, "oslshape:'colorValue'");
	
	//std::cout << "[Face::Face] Set the colour to: H=" << hue_ <<"; S=" << saturation_ << "; V=" << value_ << "." << std::endl;
	
	std::vector<std::string> point = oa.getStringProperty(face_name, "oslshape:'hasCorner'");
	
	// We only store the two lowest corners as we don't care about the hight.
	float max_distance = 0;
	
	std::vector<Vector3D> points;
	for (std::vector<std::string>::const_iterator ci = point.begin(); ci != point.end(); ++ci)
	{
		const std::string& point_name = *ci;
		float x = oa.getFloatUnique(point_name, "knowrob:'xCoord'");
		float y = oa.getFloatUnique(point_name, "knowrob:'yCoord'");
		float z = oa.getFloatUnique(point_name, "knowrob:'zCoord'");
		
		points.push_back(Vector3D(x, y, z));
		//std::cout << "[Face] (" << x << ", " << y << " ," << z << ")" << std::endl;
	}
	
	if (points.size() < 2)
	{
		std::cerr << "Too few points in this face!" << std::endl;
		return;
	}
	
	for (int i = 0; i <points.size() - 1; ++i)
	{
		for (int j = i + 1; j < points.size(); ++j)
		{
			float d = points[i].getDistance(points[j]);
			if (d > max_distance)
			{
				max_distance = d;
				p1_ = points[i];
				p2_ = points[j];
			}
		}
	}
	
	p1_transformed_ = shape.getRotationMatrix().rotate(p1_);
	p2_transformed_ = shape.getRotationMatrix().rotate(p2_);
	
	p1_transformed_ += shape.getLocation();
	p2_transformed_ += shape.getLocation();
	
	// If this face is too small, ignore it!
	Vector2D p1_proj(p1_transformed_.x_, p1_transformed_.y_);
	Vector2D p2_proj(p2_transformed_.x_, p2_transformed_.y_);
	
	if (p1_proj.getDistance(p2_proj) < 0.1f)
	{
		ignore_face_ = true;
		return;
	}
	ignore_face_ = false;
	//std::cout << "[Face::Face] Untransformed points: " << p1_ << " - " << p2_ << "." << std::endl;
	//std::cout << "[Face::Face] Transformed points: " << p1_transformed_ << " - " << p2_transformed_ << "." << std::endl;
	
	// Get the normal vector.
	std::string normal_name = oa.getStringUnique(face_name, "oslshape:'hasNormal'");
	float vector_x = oa.getFloatUnique(normal_name, "knowrob:'vectorX'");
	float vector_y = oa.getFloatUnique(normal_name, "knowrob:'vectorY'");
	float vector_z = oa.getFloatUnique(normal_name, "knowrob:'vectorZ'");
	
	normal_vector_ = Vector3D(vector_x, vector_y, vector_z);
	normal_vector_transformed_ = shape.getRotationMatrix().rotate(normal_vector_);
	
	std::cout << "Normal of face: " << normal_vector_ << ". Transformed: << " << normal_vector_transformed_ << std::endl;
	std::cout << "Loaded: " << *this << std::endl;
}

Face::Face(Shape& shape, const Face& face, bool face_is_observed)
	: face_name_(face.face_name_), has_been_observed_(face_is_observed)
{
	//std::cout << "[Face::Face] Create a face with the name '" << face_name << "'" << std::endl;
	hue_ = face.hue_;
	saturation_ = face.saturation_;
	value_ = face.value_;
	
	p1_ = face.p1_;
	p2_ = face.p2_;
	
	p1_transformed_ = shape.getRotationMatrix().rotate(p1_);
	p2_transformed_ = shape.getRotationMatrix().rotate(p2_);
	
	p1_transformed_ += shape.getLocation();
	p2_transformed_ += shape.getLocation();
	
	// If this face is too small, ignore it!
	Vector2D p1_proj(p1_transformed_.x_, p1_transformed_.y_);
	Vector2D p2_proj(p2_transformed_.x_, p2_transformed_.y_);
	
	if (p1_proj.getDistance(p2_proj) < 0.1f)
	{
		ignore_face_ = true;
		return;
	}
	ignore_face_ = false;
	
	// Get the normal vector.
	normal_vector_ = face.normal_vector_;
	normal_vector_transformed_ = shape.getRotationMatrix().rotate(normal_vector_);
	
	std::cout << "Normal of face: " << normal_vector_ << ". Transformed: << " << normal_vector_transformed_ << std::endl;
	std::cout << "Loaded: " << *this << std::endl;
}

bool Face::isSimilarTo(const Face& other) const
{
	return isSimilarTo(other.hue_, other.saturation_, other.value_, other.p1_transformed_, other.p2_transformed_);
}

bool Face::isSimilarTo(int other_hue, int other_saturation, int other_value, const Vector3D& other_p1_transformed, const Vector3D& other_p2_transformed) const
{
/*
	std::cout << "[Face::isSimilarTo " << std::endl;
	std::cout << "\tHUE: " << hue_ << " == " << other_hue << std::endl;
	std::cout << "\tSATURATION: " << saturation_ << " == " << other_saturation << std::endl;
	std::cout << "\tVALUE: " << value_ << " == " << other_value << std::endl;
	std::cout << "\tP1: " << p1_transformed_ << " == " << other_p1_transformed << std::endl;
	std::cout << "\tP2: " << p2_transformed_ << " == " << other_p2_transformed << std::endl;
*/
	// Check if the colour is similar.
	if (hue_ != other_hue) return false;
	if (saturation_ != other_saturation) return false;
	if (value_ != other_value) return false;
	
	float lowest_point_difference = std::abs(std::min(p1_transformed_.z_, p2_transformed_.z_) - std::min(other_p1_transformed.z_, other_p2_transformed.z_));
	float highest_point_difference = std::abs(std::max(p1_transformed_.z_, p2_transformed_.z_) - std::max(other_p1_transformed.z_, other_p2_transformed.z_));
	
	if (lowest_point_difference > 0.05f || highest_point_difference > 0.05f)
	{
		//std::cout << "\tHeight difference: " << height_difference << std::endl;
		return false;
	}
	
	Vector2D projected_p1_transformed(p1_transformed_.x_, p1_transformed_.y_);
	Vector2D projected_p2_transformed(p2_transformed_.x_, p2_transformed_.y_);
	
	Vector2D other_projected_p1_transformed(other_p1_transformed.x_, other_p1_transformed.y_);
	Vector2D other_projected_p2_transformed(other_p2_transformed.x_, other_p2_transformed.y_);
	
	float location_diff1 = projected_p1_transformed.getDistance(other_projected_p2_transformed) + projected_p2_transformed.getDistance(other_projected_p1_transformed);
	float location_diff2 = projected_p1_transformed.getDistance(other_projected_p1_transformed) + projected_p2_transformed.getDistance(other_projected_p2_transformed);
	/*
	std::cout << "[Face::isSimilarTo " << std::endl;
	std::cout << "\tHUE: " << hue_ << " == " << other_hue << std::endl;
	std::cout << "\tSATURATION: " << saturation_ << " == " << other_saturation << std::endl;
	std::cout << "\tVALUE: " << value_ << " == " << other_value << std::endl;
	std::cout << "\tP1: " << p1_transformed_ << " == " << other_p1_transformed << std::endl;
	std::cout << "\tP2: " << p2_transformed_ << " == " << other_p2_transformed << std::endl;
	
	std::cout << "\tDifferences: " << location_diff1 << " and " << location_diff2 << std::endl;
	*/
	if (location_diff1 < 0.05f || location_diff2 < 0.05f)
	{
		return true;
	}
	return false;
}

std::ostream& operator<<(std::ostream& os, const Face& face)
{
	os << face.getPDDLName() << ": " << face.getP1() << " <-> " << face.getP2() << "HSV: " << face.getHue() << ", " << face.getSaturation() << ", " << face.getValue() << "; observed? " << face.isObserved();
	return os;
}
