#include <turtlebot_planner/Ontology/Environment.h>

#include <ontology_db/ontol_access.h>
#include <turtlebot_planner/Ontology/Scene.h>
#include <turtlebot_planner/Ontology/Face.h>
#include <turtlebot_planner/Ontology/Shape.h>

#include "../OccupancyGridFunction.h"

Environment::Environment(OntolAccess& oa, OccupancyGridFunction& occupancy_grid_function)
	: oa_(&oa), occupancy_grid_function_(&occupancy_grid_function)
{
	
}

Environment::~Environment()
{
	for (std::vector<Scene*>::const_iterator ci = scenes_.begin(); ci != scenes_.end(); ++ci)
	{
		delete *ci;
	}
}

bool Environment::isAccessible(float x, float y)
{
	for (std::vector<Scene*>::const_iterator ci = scenes_.begin(); ci != scenes_.end(); ++ci)
	{
		if (!(*ci)->isAccessible(x, y))
		{
			return false;
		}
	}
	
	return true;
}

void Environment::reloadScenes(bool ontology_enabled)
{
	std::cout << "[Environment::reloadScenes] Delete previous scenes." << std::endl;
	Scene::clearCache();
	for (std::vector<Scene*>::const_iterator ci = scenes_.begin(); ci != scenes_.end(); ++ci)
	{
		delete *ci;
	}
	scenes_.clear();
	Face::deleteFaces();
	
	if (ontology_enabled)
	{
		std::cout << "[Environment::reloadScenes] Load new scenes." << std::endl;
		std::vector<std::string> scenes = oa_->getStringProperty("oslshape:'TheEnvironment'", "oslshape:'hasScene'");
		
		std::cout << "[Environment::reloadScenes] Found " << scenes.size() << " scenes in the ontology." << std::endl;
		
		for (std::vector<std::string>::const_iterator ci = scenes.begin(); ci != scenes.end(); ++ci)
		{
			Scene* scene = new Scene(*oa_, *ci, *occupancy_grid_function_);
			scenes_.push_back(scene);
			/*
			if (scenes_.size() == 5)
			{
				std::cout << "Only consider 5 scenes at most!" << std::endl;
				break;
			}
			*/
		}
	}
	
	// If there are no scenes then we will load an empty one.
	if (scenes_.empty())
	{
		std::vector<Shape*> shapes;
		Scene* empty_scene = new Scene(shapes, *occupancy_grid_function_);
		scenes_.push_back(empty_scene);
	}
}

bool Environment::isFullyConnected(Waypoint& enter_waypoint, Waypoint& exit_waypoint, const std::vector<Waypoint*>& waypoints, const std::vector<Waypoint*>& inspection_points, std::vector<Waypoint*>& unconnected_inspection_points)
{
	for (std::vector<Scene*>::const_iterator ci = scenes_.begin(); ci != scenes_.end(); ++ci)
	{
		Scene* scene = *ci;
		if (!scene->isFullyConnected(enter_waypoint, exit_waypoint, waypoints, inspection_points, unconnected_inspection_points))
		{
			return false;
		}
	}
	return true;
}

bool Environment::isFullyConnected(Waypoint& enter_waypoint, Waypoint& exit_waypoint, const std::vector<Waypoint*>& waypoints, const std::vector<Waypoint*>& view_cones, const std::vector<Vector2D>& view_points, const std::vector<const Face*>& faces, std::vector<Waypoint*>& unconnected_inspection_points)
{
	for (std::vector<Scene*>::const_iterator ci = scenes_.begin(); ci != scenes_.end(); ++ci)
	{
		Scene* scene = *ci;
		if (!scene->isFullyConnected(enter_waypoint, exit_waypoint, waypoints, view_cones, view_points, faces, unconnected_inspection_points))
		{
			return false;
		}
	}
	return true;
}

const Face* Environment::getFace(const std::string& face_name) const
{
	for (std::vector<Scene*>::const_iterator ci = scenes_.begin(); ci != scenes_.end(); ++ci)
	{
		Scene* scene = *ci;
		const Face* face = scene->getFace(face_name);
		
		if (face != NULL)
		{
			return face;
		}
	}
	
	exit(1);
	return NULL;
}

void Environment::getMostProbableScenes(std::vector<Scene*>& scenes, unsigned int nr_scenes) const
{
	if (nr_scenes >= scenes_.size())
	{
		scenes.insert(scenes.end(), scenes_.begin(), scenes_.end());
	}
	else
	{
		bool mask[scenes_.size()];
		memset(&mask[0], false, sizeof(bool) * scenes_.size());
		for (unsigned int i = 0; i < nr_scenes; ++i)
		{
			Scene* least_prob_scene = NULL;
			unsigned int least_prob_scene_index = 0;
			for (unsigned j = 0; j < scenes_.size(); ++j)
			{
				if (mask[j]) continue;
				
				Scene* scene = scenes_[j];
				if (least_prob_scene == NULL || scene->getProbability() < least_prob_scene->getProbability())
				{
					least_prob_scene = scene;
					least_prob_scene_index = j;
				}
			}
			
			scenes.push_back(least_prob_scene);
			mask[least_prob_scene_index] = true;
		}
	}
}
