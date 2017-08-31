#include <exception>

#include <turtlebot_planner/Ontology/Shape.h>
#include <turtlebot_planner/Ontology/Vector3D.h>
#include <turtlebot_planner/Ontology/Face.h>

#include <ontology_db/ontol_access.h>

std::map<std::string, const Shape*> Shape::cached_shapes_;

Shape::Shape(OntolAccess& oa, const std::string& ontology_name, const Vector3D& location, const RotationMatrix& rotation, const std::vector<std::string>& observed_faces)
	: ontology_name_(ontology_name), location_(location), rotation_(rotation)
{
	// Check if this shape has been cached before.
	const Shape* shape = cached_shapes_[ontology_name];
	if (shape != NULL)
	{
		std::cout << "Used cached shape: " << ontology_name << std::endl;
		std::cout << *shape << std::endl;
		for (std::vector<const Face*>::const_iterator ci = shape->faces_.begin(); ci != shape->faces_.end(); ++ci)
		{
			bool face_is_observed = false;
			if (std::find(observed_faces.begin(), observed_faces.end(), (*ci)->getName()) != observed_faces.end())
			{
				face_is_observed = true;
			}
			
			Face* face = Face::getFace(*this, **ci, face_is_observed);
			if (face == NULL) continue;
			faces_.push_back(face);
		}
		
		is_target_ = shape->is_target_;
	}
	else
	{
		std::vector<std::string> faces = oa.getStringProperty(ontology_name, "oslshape:'hasFace'");
		
		for (std::vector<std::string>::const_iterator ci = faces.begin(); ci != faces.end(); ++ci)
		{
			bool face_is_observed = false;
			if (std::find(observed_faces.begin(), observed_faces.end(), *ci) != observed_faces.end())
			{
				face_is_observed = true;
			}
			
			//Face* face = new Face(*this, *oa_, *ci, face_is_observed);
			Face* face = Face::getFace(*this, oa, *ci, face_is_observed);
			if (face == NULL) continue;
			faces_.push_back(face);
		}
		
		try
		{
			is_target_ = oa.getBoolUnique(ontology_name, "oslshape:'isGroundTarget'");
		}
		catch(std::exception& e)
		{
			is_target_ = false;
		}
		
		// Create a cache of this shape.
		Shape* clone = new Shape(*this);
		cached_shapes_[ontology_name] = clone;
	}
	
	calculateInterestingPoints();
}

Shape::Shape(const Vector3D& location, const RotationMatrix& rotation)
	: location_(location), rotation_(rotation)
{
	
}

Shape::Shape(const Shape& shape)
	: ontology_name_(shape.ontology_name_), location_(shape.location_), rotation_(shape.rotation_), is_target_(shape.is_target_),  interesting_points_(shape.interesting_points_)
{
	for (std::vector<const Face*>::const_iterator ci = shape.faces_.begin(); ci != shape.faces_.end(); ++ci)
	{
		faces_.push_back(new Face(**ci));
	}
}

void Shape::addFace(const Face& face)
{
	faces_.push_back(&face);
}

Shape::~Shape()
{
	
}

const Face* Shape::getFace(const std::string& face_name) const
{
	std::string lower_face_name = face_name;
	std::transform(lower_face_name.begin(), lower_face_name.end(), lower_face_name.begin(), ::tolower);
	
	for (std::vector<const Face*>::const_iterator ci = faces_.begin(); ci != faces_.end(); ++ci)
	{
		const Face* face = *ci;
		
		std::string lower_pddl_name = face->getPDDLName();
		std::transform(lower_pddl_name.begin(), lower_pddl_name.end(), lower_pddl_name.begin(), ::tolower);
		
		if (lower_pddl_name == lower_face_name)
		{
			return face;
		}
	}
	
	return NULL;
}

void Shape::calculateInterestingPoints()
{
	for (std::vector<const Face*>::const_iterator ci = faces_.begin(); ci != faces_.end(); ++ci)
	{
		const Face* face1 = *ci;
		Vector3D face1_centre = (face1->getP1() + face1->getP2()) / 2.0f;
		Vector2D projected_face1_centere(face1_centre.x_, face1_centre.y_);
		
		Vector2D projected_face1_normal(face1->getNormal().x_ * 10.0f, face1->getNormal().y_ * 10.0f);
		projected_face1_normal += projected_face1_centere;
		
		for (std::vector<const Face*>::const_iterator ci = faces_.begin(); ci != faces_.end(); ++ci)
		{
			const Face* face2 = *ci;
			Vector3D face2_centre = (face2->getP1() + face2->getP2()) / 2.0f;
			Vector2D projected_face2_centere(face2_centre.x_, face2_centre.y_);
			
			Vector2D projected_face2_normal(face2->getNormal().x_ * 10.0f, face2->getNormal().y_ * 10.0f);
			projected_face2_normal += projected_face2_centere;
			
			Vector2D intersection;
			if (Vector2D::getIntersectionSegments(projected_face1_centere, projected_face1_normal, projected_face2_centere, projected_face2_normal, intersection))
			{
				bool is_valid_intersection = true;
				// Check if the intersection intersects with any other face.
				for (std::vector<const Face*>::const_iterator ci = faces_.begin(); ci != faces_.end(); ++ci)
				{
					const Face* other_face = *ci;
					/*
					// Check that the distance to other faces is far enough away.
					Vector3D intersection_3d(intersection.x_, intersection.y_, 0.0f);
					if (Vector3D::getDistance(intersection_3d, other_face->getP1(), other_face->getP2()) < 0.10f)
					{
						is_valid_intersection = false;
						break;
					}
					*/
					
					if (other_face == face1 || other_face == face2)
					{
						continue;
					}
					
					Vector2D projected_other_face_p1(other_face->getP1().x_, other_face->getP1().y_);
					Vector2D projected_other_face_p2(other_face->getP2().x_, other_face->getP2().y_);
					
					Vector2D test_intersection;
					if (Vector2D::getIntersectionSegments(projected_face1_centere, intersection, projected_other_face_p1, projected_other_face_p2, test_intersection) ||
					    Vector2D::getIntersectionSegments(projected_face2_centere, intersection, projected_other_face_p1, projected_other_face_p2, test_intersection))
					{
						is_valid_intersection = false;
						break;
					}
				}
				
				// Store this point as a good point to look at.
				if (is_valid_intersection)
				{
					interesting_points_.push_back(intersection);
				}
			}
		}
	}
}

std::ostream& operator<<(std::ostream& os, const Shape& shape)
{
	os << " ** SHAPE **" << (shape.isTarget() ? " Not a target" : " Is a target") << std::endl;
	for (std::vector<const Face*>::const_iterator ci = shape.getFaces().begin(); ci != shape.getFaces().end(); ++ci)
	{
		os << **ci << std::endl;
	}
	return os;
}
