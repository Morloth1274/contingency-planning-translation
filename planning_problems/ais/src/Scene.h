#ifndef TURTLEBOT_PLANNER_ONTOLOGY_SCENE_H
#define TURTLEBOT_PLANNER_ONTOLOGY_SCENE_H

#include <string>
#include <vector>
#include <map>

#include "Vector3D.h"

class OccupancyGridFunction;
class Face;
class Vector2D;
class OntolAccess;
class Shape;
class Waypoint;
class OctomapBuilder;

/**
 * The ontology stores scenes which are based on the known shapes an observations so far.
 */
class Scene
{
public:
	/**
	 * Load the scene data from the ontology.
	 */
	Scene(OntolAccess& oa, const std::string& scene_name, OccupancyGridFunction& occupancy_grid_function);
	
	/**
	 * Load a scene with the given shapes -- debug purposes.
	 */
	Scene(const std::vector<Shape*>& shapes, OccupancyGridFunction& occupancy_grid_function);
	
	~Scene();
	
	static void clearCache();
	
	const std::vector<Shape*>& getShapes() const { return shapes_; }
	
	float getProbability() const { return probability_; }
	
	/**
	 * Check if the given point is reachable, the test is not comprehensive, it just checks if there is nothing
	 * nearby that would make it impossible to reach it.
	 */
	bool isAccessible(float x, float y);
	
	/**
	 * Check if the face is visible from the given location.
	 * @param face The face we try to determine the visibility of from @ref{location}.
	 * @param location The location we try to view the face from.
	 * @return True if the @ref{face} is visible from the @ref{location}.
	 */
	bool canSee(const Face& face, const Vector2D& location) const;
	
	/**
	 * Check if these two waypoints can be connected.
	 * @param from The first part of the line segment.
	 * @param to The second part of the line segment.
	 * @return True if the line segment from @ref{from} to @ref{to} is not colliding with any faces in this scene.
	 */
	bool canConnect(const Vector2D& from, const Vector2D& to) const;
	
	/**
	 * Find the shortest path @ref{from} to @ref{to}.
	 */
	std::vector<Waypoint*> findPath(Waypoint& from, Waypoint& to, const std::vector<Waypoint*>& all_waypoints) const;
	
	/**
	 * Check if the we can see @ref{point} from @ref{location}, we allow a wall to
	 * obscure this point if it is 'near' that wall.
	 * @param location The location from where we try to observe the point.
	 * @param point The point we like to observe.
	 * @return True if the point is observable from the location, false otherwise.
	 */
	bool canSee(const Vector2D& location, const Vector2D& point) const;
	
	/**
	 * Check if the @ref{enter_waypoint}, @ref{exit_waypoint}, @ref{inspection_points} are connected by the given @ref{waypoints}.
	 */
	bool isFullyConnected(Waypoint& enter_waypoint, Waypoint& exit_waypoint, const std::vector<Waypoint*>& waypoints, const std::vector<Waypoint*>& inspection_points, std::vector<Waypoint*>& unconnected_inspection_points);
	
	bool isFullyConnected(Waypoint& enter_waypoint, Waypoint& exit_waypoint, const std::vector<Waypoint*>& waypoints, const std::vector<Waypoint*>& view_cones, const std::vector<Vector2D>& view_points, const std::vector<const Face*>& faces, std::vector<Waypoint*>& unconnected_inspection_points);
	
	/**
	 * Check if the given point is accessable and not too close to any walls.
	 * @param point The point we want to check on the map.
	 * @param min_distance The minimal distance any object needs to be away from this point.
	 * @return True if the point is not blocked, false otherwise.
	 */
	bool isBlocked(const Vector2D& point, float min_distance) const;
	
	/**
	 * Find a face with the name @ref{face_name} in this scene.
	 * @param face_name The name of the face (as appears in PDDL).
	 * @return The Face object if it was found, NULL otherwise.
	 */
	const Face* getFace(const std::string& face_name) const;
private:
	void loadShapes(const std::string& object_name);
	
	OntolAccess* oa_;
	std::string ontology_name_;
	OccupancyGridFunction* occupancy_grid_function_;
	float probability_;
	
	std::vector<Shape*> shapes_;
	
	static std::map<std::string, std::vector<Shape*>* > object_in_scene_cache_;
	
	friend std::ostream& operator<<(std::ostream& os, const Scene& scene);
};

std::ostream& operator<<(std::ostream& os, const Scene& scene);

#endif
