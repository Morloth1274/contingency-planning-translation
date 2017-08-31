#ifndef TURTLEBOT_PLANNER_ONTOLOGY_ENVIRONMENT_H
#define TURTLEBOT_PLANNER_ONTOLOGY_ENVIRONMENT_H

#include <vector>
#include <string>

#include "Vector2D.h"

class OctomapBuilder;
class Scene;
class OntolAccess;
class Waypoint;
class OccupancyGridFunction;
class Face;

/**
 * The ontology stores scenes which are based on the known shapes an observations so far. This class 
 * manages all these scenes and provides an interface to query some of this information stored in 
 * the ontology.
 */
class Environment
{
public:
	Environment(OntolAccess& oa, OccupancyGridFunction& occupancy_grid_function);
	~Environment();
	
	/**
	 * Check if the given point is reachable for all scenes.
	 */
	bool isAccessible(float x, float y);
	
	/**
	 * Get all the scenes from the ontology and replace the existing set.
	 */
	void reloadScenes(bool ontology_enabled);
	
	/**
	 * Load a test environment for testing.
	 */
	void reloadTestScenario();
	
	/**
	 * Get all the scenes that are stored localy. Call @ref{reloadScenes} to get the new scenes from the 
	 * ontology.
	 */
	const std::vector<Scene*>& getScenes() const { return scenes_; }
	
	/**
	 * Check if the waypoint graph is fully connected and the waypoints are generated such that @ref{exit_waypoint} and @ref{inspection_points}
	 * are reachable from @ref{enter_waypoint} via the graph @ref{waypoint} in all known scenes.
	 * @param enter_waypoint The waypoint where the robot starts.
	 * @param exit_Waypoint The waypoint where the robot needs to go to after completing the mission.
	 * @param waypoints All the waypoints that have generated.
	 * @param inspection_points The points we need to visit and look at.
	 * @param unconnected_inspection_points The set of inspection points that are not reachable.
	 * @return True if @ref{exit_waypoint} and @ref{inspection_points} are reachable from @ref{enter_waypoint}, false otherwise.
	 */
	bool isFullyConnected(Waypoint& enter_waypoint, Waypoint& exit_waypoint, const std::vector<Waypoint*>& waypoints, const std::vector<Waypoint*>& inspection_points, std::vector<Waypoint*>& unconnected_inspection_points);
	
	/**
	 * Check if the waypoint graph is fully connected and the waypoints are generated such that:
	 * 1) @ref{view_points} are visible from at least one waypoint in all scenes.
	 * 2) @ref{faces} are visible from at least one waypoint in all scenes.
	 * 3) @ref{view_cones} have been visited and we have made an observation action from.
	 * @param enter_waypoint The waypoint where the robot starts.
	 * @param exit_Waypoint The waypoint where the robot needs to go to after completing the mission.
	 * @param waypoints All the waypoints that have generated, when this function terminates it contains all the necessary waypoints 
	 * to fullfil points (1) and (2).
	 * @param view_cones The set of waypoints we need to visit and do an observe action from.
	 * @param inspection_points The points we need to visit and look at.
	 * @param view_points The points we want to look at.
	 * @param faces The faces we want to observe.
	 * @param unconnected_inspection_points The set of inspection points that are not reachable.
	 * @return True if a set of waypoints to fullfil (1) and (2) have been found, false otherwise.
	 */
	bool isFullyConnected(Waypoint& enter_waypoint, Waypoint& exit_waypoint, const std::vector<Waypoint*>& waypoints, const std::vector<Waypoint*>& view_cones, const std::vector<Vector2D>& view_points, const std::vector<const Face*>& faces, std::vector<Waypoint*>& unconnected_inspection_points);
	
	/**
	 * Find a face with the name @ref{face_name} in this environment.
	 * @param face_name The name of the face (as appears in PDDL).
	 * @return The Face object if it was found, NULL otherwise.
	 */
	const Face* getFace(const std::string& face_name) const;
	
	/**
	 * Get the most probable scenes.
	 * @param scenes The list where the @ref{nr_scenes} most probable scenes will be added to.
	 * @param nr_scenes The number of scenes that are recovered.
	 */
	void getMostProbableScenes(std::vector<Scene*>& scenes, unsigned int nr_scenes) const;
private:
	std::vector<Scene*> scenes_;
	OntolAccess *oa_;
	OccupancyGridFunction* occupancy_grid_function_;
};

#endif
