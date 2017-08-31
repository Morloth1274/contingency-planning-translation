#include "Waypoint.h"
#include "Edge.h"

#include <iostream>
#include <sstream>

#include <ontology_db/ontol_access.h>

#define FLYING_TURTLEBOT_PLANNING_USE_ONTOLOGY

unsigned int Waypoint::waypoint_id_ = 0;
ros::ServiceClient Waypoint::create_instances_client_;
OntolAccess* Waypoint::oa_ = NULL;

void Waypoint::initWaypoints(ros::NodeHandle& ros_node, OntolAccess& oa)
{
	oa_ = &oa;
	create_instances_client_ = ros_node.serviceClient<ontology_db::CreateInstanceOfClass>("osl_ontology/create_instance_of_class");
}

#ifdef FLYING_TURTLEBOT_PLANNING_USE_ONTOLOGY
Waypoint& Waypoint::generateWaypoint(float x, float y, float theta)
{
	ontology_db::CreateInstanceOfClass create_instance;
	create_instance.request.class_name = "plan:'Pose2D'";
	if (!create_instances_client_.call(create_instance))
	{
		std::cout << "Failed to add a pose to the ontology!" << std::endl;
	}
	std::string pose2d_instance_name = create_instance.response.instance_name;

	oa_->setFloatProperty(pose2d_instance_name, "knowrob:'xCoord'", x);
	oa_->setFloatProperty(pose2d_instance_name, "knowrob:'yCoord'", y);
	
	// Create the rotation matrix.
	create_instance.request.class_name = "knowrob:'RotationMatrix2D'";
	if (!create_instances_client_.call(create_instance))
	{
		std::cout << "Failed to add a 2D rotation matrix to the ontology!" << std::endl;
	}
	std::string rotation_matrix_instance_name = create_instance.response.instance_name;
	oa_->setStringProperty(pose2d_instance_name, "knowrob:'orientation'", rotation_matrix_instance_name);
	
	// Set elements of rotation matrix
	float cos_theta = cos(theta);
	float sin_theta = sin(theta);
	oa_->setFloatProperty(rotation_matrix_instance_name, "knowrob:'m00'", cos_theta);
	oa_->setFloatProperty(rotation_matrix_instance_name, "knowrob:'m01'", -sin_theta);
	oa_->setFloatProperty(rotation_matrix_instance_name, "knowrob:'m10'", sin_theta);
	oa_->setFloatProperty(rotation_matrix_instance_name, "knowrob:'m11'", cos_theta);

	std::string predicate = pose2d_instance_name.substr(pose2d_instance_name.find("#") + 1);
	Waypoint* waypoint = new Waypoint(pose2d_instance_name, predicate, x, y, theta);
	
	std::cout << "Created a Pose2D instance with the name: " << pose2d_instance_name << " and with the rotation matrix: " << rotation_matrix_instance_name << "!" << std::endl;
	return *waypoint;
}
#else
Waypoint& Waypoint::generateWaypoint(float x, float y, float theta)
{
	std::stringstream ss;
	ss << "Waypoint" << waypoint_id_;
	++waypoint_id_;
	
	Waypoint* waypoint = new Waypoint(ss.str(), ss.str(), x, y, theta);
	std::cout << "Created a Pose2D instance with the name: " << ss.str() << "(" << x << ", " << y << "; " << theta << ") and with the rotation matrix: DUMMY" << "!" << std::endl;
	return *waypoint;
}
#endif

std::ostream& operator<<(std::ostream& os, const Waypoint& waypoint)
{
	os << "(" << waypoint.x_ << ", " << waypoint.y_ << ")";
	return os;
}
