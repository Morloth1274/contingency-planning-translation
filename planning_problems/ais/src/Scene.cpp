#include <turtlebot_planner/Ontology/Scene.h>

#include <ontology_db/ontol_access.h>

#include <turtlebot_planner/Ontology/RotationMatrix.h>
#include <turtlebot_planner/Ontology/Vector3D.h>
#include <turtlebot_planner/Ontology/Vector2D.h>
#include <turtlebot_planner/Ontology/Shape.h>
#include <turtlebot_planner/Ontology/Face.h>
#include "../Waypoint.h"
#include "../OccupancyGridFunction.h"
#include <queue>

std::map<std::string, std::vector<Shape*>* > Scene::object_in_scene_cache_;

Scene::Scene(OntolAccess& oa, const std::string& scene_name, OccupancyGridFunction& occupancy_grid_function)
	: oa_(&oa), ontology_name_(scene_name), occupancy_grid_function_(&occupancy_grid_function)
{
	std::cout << "[Scene::Scene] Create a new scene named '" << scene_name << "'" << std::endl;
	std::vector<std::string> scenes = oa.getStringProperty(scene_name, "oslshape:'hasObject'");
	
	try
	{
		probability_ = oa.getFloatUnique(scene_name, "oslshape:'probability'");
	}
	catch (std::domain_error de)
	{
		probability_ = 1;
		std::cout << "Probabilities not yet supported." << std::endl;
	}
	
	std::cout << "Load " << scenes.size() << " objects in this scene." << std::endl;
	for (std::vector<std::string>::const_iterator ci = scenes.begin(); ci != scenes.end(); ++ci)
	{
		loadShapes(*ci);
	}
}

Scene::Scene(const std::vector<Shape*>& shapes, OccupancyGridFunction& occupancy_grid_function)
	: shapes_(shapes), occupancy_grid_function_(&occupancy_grid_function)
{
	
}

Scene::~Scene()
{
	for (std::vector<Shape*>::const_iterator ci = shapes_.begin(); ci != shapes_.end(); ++ci)
	{
		delete *ci;
	}
}

void Scene::clearCache()
{
	/*
	for (std::map<std::string, std::vector<Shape*>* >::const_iterator ci = object_in_scene_cache_.begin(); ci != object_in_scene_cache_.end(); ++ci)
	{
		std::vector<Shape*>* shapes = (*ci).second;
		for (std::vector<Shape*>::const_iterator ci = shapes->begin(); ci != shapes->end(); ++ci)
		{
			delete *ci;
		}
	}
	*/
	object_in_scene_cache_.clear();
}

bool Scene::isAccessible(float x, float y)
{
	Vector2D centre(x, y);
	Vector2D north(x, y + 2);
	Vector2D west(x + 2, y);
	Vector2D east(x - 2, y);
	Vector2D south(x, y - 2);
	
	if (canConnect(centre, north) ||
	    canConnect(centre, west) ||
	    canConnect(centre, east) ||
	    canConnect(centre, south))
	{
		return true;
	}
	return false;
}

bool Scene::canSee(const Face& face, const Vector2D& location) const
{
	Vector2D face_p1(face.getP1().x_, face.getP1().y_);
	Vector2D face_p2(face.getP2().x_, face.getP2().y_);
	
	Vector2D face_centre = (face_p1 + face_p2) / 2.0f;
	Vector2D projected_normal(face.getNormal().x_, face.getNormal().y_);
	projected_normal.normalise();
	
	// Check if the viewing angle is less than 45 angles.
	Vector2D to_view = location - face_centre;
	to_view.normalise();
	
	if (Vector2D::dot(to_view, projected_normal) < 0.5f)
	{
		return false;
	}
	
	//std::cout << "Can we see: " << face.getP1() << " - " << face.getP2() << " from " << location << "?" << std::endl;
	for (std::vector<Shape*>::const_iterator ci = shapes_.begin(); ci != shapes_.end(); ++ci)
	{
		const Shape* shape = *ci;
		for (std::vector<const Face*>::const_iterator ci = shape->getFaces().begin(); ci != shape->getFaces().end(); ++ci)
		{
			const Face* other_face = *ci;
			if (other_face == &face)
			{
				continue;
			}
			
			Vector2D other_face_p1(other_face->getP1().x_, other_face->getP1().y_);
			Vector2D other_face_p2(other_face->getP2().x_, other_face->getP2().y_);
			
			Vector2D intersection;
			if (Vector2D::getIntersectionSegments(location, face_p1, other_face_p1, other_face_p2, intersection) &&
			   intersection.getDistance(face_p1) > 0.01f)
			{
				//std::cout << "No! The face: " << other_face->getP1() << " - " << other_face->getP2() << " blocks it!" << std::endl;
				return false;
			}
			
			if (Vector2D::getIntersectionSegments(location, face_p2, other_face_p1, other_face_p2, intersection) &&
			   intersection.getDistance(face_p2) > 0.01f)
			{
				//std::cout << "No! The face: " << other_face->getP1() << " - " << other_face->getP2() << " blocks it!" << std::endl;
				return false;
			}
			
			/*
			if (getDistance(Vector3D(location.x_, location.y_, 0), face.getP1(), other_face->getP1(), other_face->getP2()) < 0.15f ||
			    getDistance(Vector3D(location.x_, location.y_, 0), face.getP2(), other_face->getP1(), other_face->getP2()) < 0.15f)
			{
				std::cout << "No! The face: " << other_face->getP1() << " - " << other_face->getP2() << " blocks it!" << std::endl;
				return false;
			}
			*/
		}
	}
	//std::cout << "Yes we can!" << std::endl;
	return true;
}
	
bool Scene::canConnect(const Vector2D& from, const Vector2D& to) const
{
	geometry_msgs::Point from_point;
	from_point.x = from.x_;
	from_point.y = from.y_;
	from_point.z = 0;
	
	geometry_msgs::Point to_point;
	to_point.x = to.x_;
	to_point.y = to.y_;
	to_point.z = 0;
	Vector3D face_p1(from.x_, from.y_, 0.0f);
	Vector3D face_p2(to.x_, to.y_, 0.0f);
	if (!occupancy_grid_function_->canConnect(from_point, to_point, 0.25f))
	{
		return false;
	}
	
	Vector2D intersection;
	for (std::vector<Shape*>::const_iterator ci = shapes_.begin(); ci != shapes_.end(); ++ci)
	{
		const Shape* shape = *ci;
		for (std::vector<const Face*>::const_iterator ci = shape->getFaces().begin(); ci != shape->getFaces().end(); ++ci)
		{
			const Face* other_face = *ci;
			Vector3D other_face_p1(other_face->getP1().x_, other_face->getP1().y_, 0.0f);
			Vector3D other_face_p2(other_face->getP2().x_, other_face->getP2().y_, 0.0f);
			
			if (Vector3D::getDistance(face_p1, face_p2, other_face_p1, other_face_p2) < 0.25f)
			{
				return false;
			}
			/*
			if (Vector2D::getIntersectionSegments(from, to, other_face_p1, other_face_p2, intersection))
			{
				return false;
			}
			*/
		}
	}
	return true;
}

bool Scene::canSee(const Vector2D& location, const Vector2D& point) const
{
	geometry_msgs::Point from_point;
	from_point.x = location.x_;
	from_point.y = location.y_;
	from_point.z = 0;
	
	geometry_msgs::Point to_point;
	to_point.x = point.x_;
	to_point.y = point.y_;
	to_point.z = 0;
	Vector3D face_p1(location.x_, location.y_, 0.0f);
	Vector3D face_p2(point.x_, point.y_, 0.0f);
	if (!occupancy_grid_function_->canConnect(from_point, to_point, 0.01f))
	{
		return false;
	}
	
	Vector2D intersection;
	for (std::vector<Shape*>::const_iterator ci = shapes_.begin(); ci != shapes_.end(); ++ci)
	{
		const Shape* shape = *ci;
		for (std::vector<const Face*>::const_iterator ci = shape->getFaces().begin(); ci != shape->getFaces().end(); ++ci)
		{
			const Face* other_face = *ci;
			Vector3D other_face_p1(other_face->getP1().x_, other_face->getP1().y_, 0.0f);
			Vector3D other_face_p2(other_face->getP2().x_, other_face->getP2().y_, 0.0f);
			
			if (Vector3D::getDistance(face_p1, face_p2, other_face_p1, other_face_p2) < 0.01f)
			{
				return false;
			}
		}
	}
	return true;
}

void Scene::loadShapes(const std::string& object_name)
{
	std::vector<Shape*>* cached_shapes = object_in_scene_cache_[object_name];
	if (cached_shapes != NULL)
	{
		//shapes_ = *cached_shapes;
		for (std::vector<Shape*>::const_iterator ci = cached_shapes->begin(); ci != cached_shapes->end(); ++ci)
		{
			// For every scene, get all the shapes.
			Shape* shape = new Shape(**ci);
			shapes_.push_back(shape);
		}
		return;
	}
	
	// Get the location and rotation matrix of this object.
	std::string rotation_name = "";
	try
	{
		rotation_name = oa_->getStringUnique(object_name, "knowrob:'orientation'");
	}
	catch (std::domain_error de)
	{
		std::cout << "Orientation not found for: " << object_name << "." << std::endl;
		throw;
	}
	float m00 = oa_->getFloatUnique(rotation_name, "knowrob:'m00'");
	float m01 = oa_->getFloatUnique(rotation_name, "knowrob:'m01'");
	float m10 = oa_->getFloatUnique(rotation_name, "knowrob:'m10'");
	float m11 = oa_->getFloatUnique(rotation_name, "knowrob:'m11'");
	
	RotationMatrix rotation_matrix(m00, m10, m01, m11);
	
	float x = oa_->getFloatUnique(object_name, "knowrob:'xCoord'");
	float y = oa_->getFloatUnique(object_name, "knowrob:'yCoord'");
	float z = oa_->getFloatUnique(object_name, "knowrob:'zCoord'");
	
	Vector3D location(x, y, z);
	
	std::cout << "[Scene::loadShapes] Load the object with the name '" << object_name << "'" << std::endl;
	//std::cout << rotation_matrix << std::endl;
	//std::cout << location << std::endl;
	
	std::vector<std::string> observed_faces = oa_->getStringProperty(object_name, "oslshape:'hasObservedFace'");
	
	std::cout << "Observed faces(" << observed_faces.size() << "): " << std::endl;
	for (std::vector<std::string>::const_iterator ci = observed_faces.begin(); ci != observed_faces.end(); ++ci)
	{
		std::cout << "\t" << *ci << std::endl;
	}
	
	std::vector<std::string> shapes = oa_->getStringProperty(object_name, "oslshape:'hasShape'");
	for (std::vector<std::string>::const_iterator ci = shapes.begin(); ci != shapes.end(); ++ci)
	{
		// For every scene, get all the shapes.
		Shape* shape = new Shape(*oa_, *ci, location, rotation_matrix, observed_faces);
		shapes_.push_back(shape);
	}
	
	object_in_scene_cache_[object_name] = &shapes_;
}

bool Scene::isFullyConnected(Waypoint& enter_waypoint, Waypoint& exit_waypoint, const std::vector<Waypoint*>& waypoints, const std::vector<Waypoint*>& inspection_points, std::vector<Waypoint*>& unconnected_inspection_points)
{
	std::vector<Vector2D> view_points;
	std::vector<const Face*> faces;
	return isFullyConnected(enter_waypoint, exit_waypoint, waypoints, inspection_points, view_points, faces, unconnected_inspection_points);
	/*
	std::set<const Waypoint*> reachable_waypoints;
	reachable_waypoints.insert(&enter_waypoint);
	
	bool finished = false;
	while (!finished)
	{
		finished = true;
		for (std::vector<Waypoint*>::const_iterator ci = waypoints.begin(); ci != waypoints.end(); ++ci)
		{
			if (reachable_waypoints.count(*ci) == 1)
			{
				continue;
			}
			const Waypoint* wp1 = *ci;
			//bool can_connect = shapes_.empty();
			bool can_connect = false;
			
			//std::cout << "Try connecting: " << *wp1 << std::endl;
				
			for (std::set<const Waypoint*>::const_iterator ci = reachable_waypoints.begin(); ci != reachable_waypoints.end(); ++ci)
			{
				const Waypoint* wp2 = *ci;
				if (canConnect(Vector2D(wp1->x_, wp1->y_), Vector2D(wp2->x_, wp2->y_)))
				{
					//std::cout << *wp1 << " can connect with:" << *wp2 << std::endl;
					can_connect = true;
					break;
				}

			}
			
			if (can_connect)
			{
				finished = false;
				reachable_waypoints.insert(wp1);
			}
		}
	}
	// Check if all the vital waypoints are connected.
	bool view_cones_accessable = true;
	for (std::vector<Waypoint*>::const_iterator ci = inspection_points.begin(); ci != inspection_points.end(); ++ci)
	{
		if (reachable_waypoints.count(*ci) == 0)
		{
			std::cout << "Could not reach the inspection point: (" << (*ci)->x_ << ", " << (*ci)->y_ << ")" << std::endl;
			
			unconnected_inspection_points.push_back(*ci);
			view_cones_accessable = false;
		}
		//std::cout << "Reach the inspection point: (" << (*ci)->x_ << ", " << (*ci)->y_ << ")" << std::endl;
	}
	return view_cones_accessable;
*/
}

bool Scene::isFullyConnected(Waypoint& enter_waypoint, Waypoint& exit_waypoint, const std::vector<Waypoint*>& waypoints, const std::vector<Waypoint*>& view_cones, const std::vector<Vector2D>& view_points, const std::vector<const Face*>& faces, std::vector<Waypoint*>& unconnected_inspection_points)
{
	std::set<const Face*> observed_faces;
	std::set<Vector2D> observed_view_points;
	
	Vector2D enter_loc(enter_waypoint.x_, enter_waypoint.y_);
		
	for (std::vector<const Face*>::const_iterator ci = faces.begin(); ci != faces.end(); ++ci)
	{
		if (canSee(**ci, enter_loc))
		{
			observed_faces.insert(*ci);
		}
	}
	
	
	for (std::vector<Vector2D>::const_iterator ci = view_points.begin(); ci != view_points.end(); ++ci)
	{
		if (canConnect(*ci, enter_loc))
		{
			observed_view_points.insert(*ci);
		}
	}
	
	std::vector<Waypoint*> all_waypoints(waypoints);
	all_waypoints.insert(all_waypoints.end(), view_cones.begin(), view_cones.end());
	all_waypoints.push_back(&enter_waypoint);
	all_waypoints.push_back(&exit_waypoint);
	
	// Establish the edges of the waypoints.
	for (int i = 0; i < all_waypoints.size(); ++i)
	{
		Waypoint* wp = all_waypoints[i];
		wp->edges_.clear();
	}
	
	for (int i = 0; i < all_waypoints.size(); ++i)
	{
		Waypoint* wp = all_waypoints[i];
		//wp->edges_.clear();
		for (int j = i + 1; j < all_waypoints.size(); ++j)
		{
			Waypoint* other_wp = all_waypoints[j];
			
			if (canConnect(Vector2D(wp->x_, wp->y_), Vector2D(other_wp->x_, other_wp->y_)))
			{
				wp->addEdge(*other_wp);
				other_wp->addEdge(*wp);
			}
		}
	}
	
	std::vector<const Waypoint*> open_queue;
	open_queue.push_back(&enter_waypoint);
	
	std::set<const Waypoint*> reachable_waypoints;
	//reachable_waypoints.insert(&enter_waypoint);
	
	while (open_queue.size() > 0)
	{
		const Waypoint* current_waypoint = *open_queue.begin();
		open_queue.erase(open_queue.begin());
		
		if (reachable_waypoints.count(current_waypoint) == 1)
		{
			continue;
		}
		reachable_waypoints.insert(current_waypoint);
		
		for (std::vector<const Face*>::const_iterator ci = faces.begin(); ci != faces.end(); ++ci)
		{
			if (canSee(**ci, Vector2D(current_waypoint->x_, current_waypoint->y_)))
			{
				observed_faces.insert(*ci);
			}
		}
		
		
		for (std::vector<Vector2D>::const_iterator ci = view_points.begin(); ci != view_points.end(); ++ci)
		{
			if (canSee(*ci, Vector2D(current_waypoint->x_, current_waypoint->y_)))
			{
				observed_view_points.insert(*ci);
			}
		}
		
		for (std::vector<std::pair<float, Waypoint*> >::const_iterator ci = current_waypoint->edges_.begin(); ci != current_waypoint->edges_.end(); ++ci)
		{
			open_queue.push_back((*ci).second);
		}
	}
	
	/*
	bool finished = false;
	while (!finished)
	{
		finished = true;
		for (std::vector<Waypoint*>::const_iterator ci = waypoints.begin(); ci != waypoints.end(); ++ci)
		{
			if (reachable_waypoints.count(*ci) == 1)
			{
				continue;
			}
			const Waypoint* wp1 = *ci;
			bool can_connect = false;
			
			//std::cout << "Try connecting: " << *wp1 << std::endl;
				
			for (std::set<const Waypoint*>::const_iterator ci = reachable_waypoints.begin(); ci != reachable_waypoints.end(); ++ci)
			{
				const Waypoint* wp2 = *ci;
				if (canConnect(Vector2D(wp1->x_, wp1->y_), Vector2D(wp2->x_, wp2->y_)))
				{
					//std::cout << *wp1 << " can connect with:" << *wp2 << std::endl;
					can_connect = true;
					break;
				}
			}
			
			if (can_connect)
			{
				finished = false;
				reachable_waypoints.insert(wp1);
				
				for (std::vector<const Face*>::const_iterator ci = faces.begin(); ci != faces.end(); ++ci)
				{
					if (canSee(**ci, Vector2D(wp1->x_, wp1->y_)))
					{
						observed_faces.insert(*ci);
					}
				}
				
				
				for (std::vector<Vector2D>::const_iterator ci = view_points.begin(); ci != view_points.end(); ++ci)
				{
					//if (canConnect(*ci, Vector2D(wp1->x_, wp1->y_)))
					if (canSee(*ci, Vector2D(wp1->x_, wp1->y_)))
					{
						observed_view_points.insert(*ci);
					}
				}
			}
		}
	}
	*/
	if (observed_faces.size() != faces.size() || observed_view_points.size() != view_points.size())
	{
		std::cout << observed_faces.size() << "/" << faces.size() << "::" << observed_view_points.size() << "/" << view_points.size() << std::endl;
		return false;
	}

	// Check if all the vital waypoints are connected.
	bool view_cones_accessable = true;
	for (std::vector<Waypoint*>::const_iterator ci = view_cones.begin(); ci != view_cones.end(); ++ci)
	{
		if (reachable_waypoints.count(*ci) == 0)
		{
			std::cout << (*ci)->ontology_id_ << std::endl;
			unconnected_inspection_points.push_back(*ci);
			view_cones_accessable = false;
		}
	}
	/*
	if (reachable_waypoints.count(&exit_waypoint) == 0)
	{
		std::cout << "Exit not linked!" << std::endl;
		return false;
	}
	*/
	return view_cones_accessable;
}

const Face* Scene::getFace(const std::string& face_name) const
{
	for (std::vector<Shape*>::const_iterator ci = shapes_.begin(); ci != shapes_.end(); ++ci)
	{
		Shape* shape = *ci;
		const Face* face = shape->getFace(face_name);
		if (face != NULL)
		{
			return face;
		}
	}
	
	return NULL;
}

std::vector<Waypoint*> Scene::findPath(Waypoint& from, Waypoint& to, const std::vector<Waypoint*>& all_waypoints) const
{
	std::vector<WaypointNode*> all_nodes;
	
	// Find a path!
	std::priority_queue<WaypointNode*, std::vector<WaypointNode*>, WaypointNode> queue;
	
	std::set<Waypoint*> closed_set;
	
	std::vector<WaypointNode*> path;
	queue.push(new WaypointNode(from, NULL, to, 0, path));
	
	while (queue.size() > 0)
	{
		WaypointNode* current_node = queue.top();
		queue.pop();
		
		if (closed_set.count(current_node->waypoint_) == 1)
		{
			continue;
		}
		closed_set.insert(current_node->waypoint_);
		
		// Check if this is the goal.
		if (current_node->waypoint_ == &to)
		{
			const std::vector<WaypointNode*>& path = current_node->path_;
			std::vector<Waypoint*> final_path;
			for (std::vector<WaypointNode*>::const_iterator ci = path.begin(); ci != path.end(); ++ci)
			{
				WaypointNode* node = *ci;
				final_path.insert(final_path.end(), node->waypoint_);
			}
			
			for (std::vector<WaypointNode*>::const_iterator ci = all_nodes.begin(); ci != all_nodes.end(); ++ci)
			{
				delete *ci;
			}
			return final_path;
		}
		
		std::vector<WaypointNode*> children;
		current_node->getChildren(children);
		for (std::vector<WaypointNode*>::const_iterator ci = children.begin(); ci != children.end(); ++ci)
		{
			queue.push(*ci);
			all_nodes.push_back(*ci);
		}
	}
	
	for (std::vector<WaypointNode*>::const_iterator ci = all_nodes.begin(); ci != all_nodes.end(); ++ci)
	{
		delete *ci;
	}
	
	std::vector<Waypoint*> final_path;
	return final_path;
}

bool Scene::isBlocked(const Vector2D& point, float min_distance) const
{
	geometry_msgs::Point p;
	p.x = point.x_;
	p.y = point.y_;
	if (occupancy_grid_function_->isBlocked(p, min_distance))
	{
		return true;
	}
	
	Vector3D point_3d(point.x_, point.y_, 0.0f);
	for (std::vector<Shape*>::const_iterator ci = shapes_.begin(); ci != shapes_.end(); ++ci)
	{
		const Shape* shape = *ci;
		for (std::vector<const Face*>::const_iterator ci = shape->getFaces().begin(); ci != shape->getFaces().end(); ++ci)
		{
			const Face* face = *ci;
			if (Vector3D::getDistance(point_3d, face->getP1(), face->getP2()) < min_distance)
			{
				return true;
			}
		}
	}
	return false;
}

std::ostream& operator<<(std::ostream& os, const Scene& scene)
{
	os << " *** SCENE *** " << std::endl;
	for (std::vector<Shape*>::const_iterator ci = scene.shapes_.begin(); ci != scene.shapes_.end(); ++ci)
	{
		os << **ci << std::endl;
	}
	return os;
}
