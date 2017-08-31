#ifndef FLYING_TURTLEBOT_PLANNING_WAYPOINT_H
#define FLYING_TURTLEBOT_PLANNING_WAYPOINT_H

#include <vector>
#include <ostream>
#include <algorithm>
#include <math.h>
#include <limits>

#include <ros/ros.h>

class OntolAccess;

struct Waypoint
{
	Waypoint(const std::string& ontology_id, const std::string& predicate, float x, float y, float theta)
		: ontology_id_(ontology_id), predicate_(predicate), x_(x), y_(y), theta_(theta)
	{
		// Store predicates always upper case.
		std::transform(predicate_.begin(), predicate_.end(), predicate_.begin(), ::toupper);
	}
	
	/**
	 * Add an edge to this waypoint.
	 * @param edge The waypoint this one is connected with.
	 */
	void addEdge(Waypoint& edge)
	{
		// Make sure we don't add duplicates.
		for (std::vector<std::pair<float, Waypoint*> >::iterator ci = edges_.begin(); ci != edges_.end(); ++ci)
		{
			if ((*ci).second == &edge)
			{
				edges_.erase(ci);
				break;
			}
		}
		
		float distance = sqrt((x_ - edge.x_) * (x_ - edge.x_) + (y_ - edge.y_) * (y_ - edge.y_));
		edges_.push_back(std::make_pair(distance, &edge));
	}
	
	/**
	 * Get the distance to an edge (if any).
	 */
	float getDistanceTo(const Waypoint& other) const
	{
		return sqrt((x_ - other.x_) * (x_ - other.x_) + (y_ - other.y_) * (y_ - other.y_));
		/*
		for (std::vector<std::pair<float, Waypoint*> >::const_iterator ci = edges_.begin(); ci != edges_.end(); ++ci)
		{
			if (&other == (*ci).second)
			{
				return (*ci).first;
			}
		}
		return std::numeric_limits<float>::max();
		*/
	}
	
	/**
	 * Make an inspection point visible from this waypoint.
	 * @param inspection_point The inspection point that is visible from this waypoint.
	 *
	void makeVisible(InspectionPoint& inspection_point)
	{
		visible_inspection_points_.push_back(&inspection_point);
	}*/
	
	/**
	 * Generate a waypoint and store it in the ontology (if enabled).
	 * @param x The X coordinate.
	 * @param y The Y coordinate.
	 * @param theta The angle.
	 * @return The generated waypoint.
	 */
	static Waypoint& generateWaypoint(float x, float y, float theta);
	
	static void initWaypoints(ros::NodeHandle& ros_node, OntolAccess& oa);
	
	static ros::ServiceClient create_instances_client_;
	static OntolAccess *oa_; // Interface to gain access to the ontology.
	
	std::string ontology_id_; // The handle by which this waypoint is stored in the ontology.
	std::string predicate_; // The name by which this inspection point is known in the planner.
	float x_, y_, theta_; // The location and orientation of the waypoint.
	std::vector<std::pair<float, Waypoint*> > edges_; // Connections to other waypoints.
	
	static unsigned int waypoint_id_;
};

struct WaypointNode : public std::binary_function<WaypointNode*, WaypointNode*, bool>
{
	WaypointNode()
	{
		
	}
	
	WaypointNode(Waypoint& waypoint, WaypointNode* parent, Waypoint& to, float edge_cost, const std::vector<WaypointNode*>& path)
		: waypoint_(&waypoint), to_(&to), path_(path)
	{
		path_.push_back(this);
		
		// Determine whether we went 'straight' or whether we took a turn.
		cost_ = edge_cost;
		
		if (parent != NULL)
		{
			cost_ += parent->cost_;
		}
		
		estimated_cost_to_goal_ = waypoint.getDistanceTo(to);
	}
	
	/**
	 * Generate children for this node.
	 */
	void getChildren(std::vector<WaypointNode*>& children)
	{
		for (std::vector<std::pair<float, Waypoint*> >::const_iterator ci = waypoint_->edges_.begin(); ci != waypoint_->edges_.end(); ++ci)
		{
			children.push_back(new WaypointNode(*(*ci).second, this, *to_, (*ci).first, path_));
		}
	}
	
	bool operator()(const WaypointNode* lhs, const WaypointNode* rhs) const
	{
		return lhs->cost_ + lhs->estimated_cost_to_goal_ > rhs->cost_ + rhs->estimated_cost_to_goal_;
	}
	
	Waypoint* waypoint_;
	Waypoint* to_;
	float cost_;
	float estimated_cost_to_goal_;
 	std::vector<WaypointNode*> path_;
};

std::ostream& operator<<(std::ostream& os, const Waypoint& waypoint);

#endif
