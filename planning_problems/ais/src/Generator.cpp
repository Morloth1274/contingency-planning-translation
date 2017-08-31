#include <stdlib.h> 

#include <iostream>
#include <fstream>
#include <algorithm>

#include "CPGenerator.h"

#include "Waypoint.h"
#include "Face.h"
#include "Scene.h"
#include "Environment.h"
#include "Shape.h"

Generator::Generator(ros::NodeHandle& ros_node, OccupancyGridFunction& occupancy_grid_function, const std::string& planner_command_line, bool disable_ontology)
	: ros_node_(&ros_node), occupancy_grid_function_(&occupancy_grid_function), planner_command_line_(planner_command_line), oa_(new OntolAccess(ros_node)), environment_(new Environment(*oa_, occupancy_grid_function)), disable_ontology_(disable_ontology)
{
	/*
	aoi_min_x_ = -3;
	aoi_min_y_ = -5;
	aoi_max_x_ = 5.5;
	aoi_max_y_ = 3;
	*/
	aoi_min_x_ = -2;
	aoi_min_y_ = -5;
	aoi_max_x_ = 7;
	aoi_max_y_ = 2;
	
	Waypoint::initWaypoints(ros_node, *oa_);
	
	srand(time(NULL));
}

bool CPGenerator::generatePlan(turtlebot_common::GeneratePlan::Request  &req, turtlebot_common::GeneratePlan::Response &res)
{
	std::cout << "[CPGenerator::generatePlan] " << std::endl;
	// The amount of time the planner is allowed to run.
	float time_limit = req.time_limit;
	ros::WallTime start_gen_plan = ros::WallTime::now();
	
	// Store all the names of the waypoints and inspection points as they are used in the planner.
	std::vector<Waypoint*> waypoints;
	std::vector<Waypoint*> inspection_points;
	
	// Map the names as they are used by the planner to the names as they are stored in the ontology.
	std::string aoi_id = req.aoi;
	std::vector<Vector2D> view_points;
	std::vector<const Face*> faces;
	
#ifdef USE_ALTERNATIVE_VIEW_POINT_GENERATION
	view_point_generator_->generateWaypoints(*environment_, view_points, faces, 10);
	
	// Check if there are faces we want to observe, if then these could be targets. Focus on them!
	for (std::vector<const Face*>::const_iterator ci = faces.begin(); ci != faces.end(); ++ci)
	{
		const Face* face = *ci;
		const Vector3D& normal_vector = face->getNormal();
		Vector3D face_centre = (face->getP1() + face->getP2()) / 2.0f;
		
		Vector2D normal_vector_2d(normal_vector.x_, normal_vector.y_);
		Vector2D face_centre_2d(face_centre.x_, face_centre.y_);
		Vector2D orthogonal_normal_vector_2d(-normal_vector.y_, normal_vector.x_);
		
		unsigned int most_scenes = 0;
		Vector2D best_viewing_location;
		
		for (float distance = 5.0f; distance >= 1.0f; distance -= 0.5f)
		{
			for (float side = -distance; side < distance; side += 0.5f)
			{
				Vector2D viewing_location = face_centre_2d + normal_vector_2d * distance + orthogonal_normal_vector_2d * side;
				
				if (!environment_->isAccessible(viewing_location.x_, viewing_location.y_)) continue;
				
				unsigned int visibility = 0;
				for (std::vector<Scene*>::const_iterator ci = environment_->getScenes().begin(); ci != environment_->getScenes().end(); ++ci)
				{
					const Scene* scene = *ci;
					if (scene->canSee(*face, viewing_location))
					{
						++visibility;
					}
				}
				
				if (visibility > most_scenes)
				{
					best_viewing_location = viewing_location;
					most_scenes = visibility;
				}
			}
		}
		
		// Generate its waypoint.
		if (most_scenes > 0)
		{
			Waypoint& inspection_point = Waypoint::generateWaypoint(best_viewing_location.x_, best_viewing_location.y_, atan2(face_centre.y_ - best_viewing_location.y_, face_centre.x_ - best_viewing_location.x_));
			inspection_points.push_back(&inspection_point);
		}
	}
	
	// If there are no interesting faces, we are interested in looking 'behind' faces we have observed.
	if (inspection_points.empty())
	{
		for (std::vector<Vector2D>::const_iterator ci = view_points.begin(); ci != view_points.end(); ++ci)
		{
			const Vector2D& view_point = *ci;
			unsigned int most_scenes = 0;
			Vector2D best_viewing_location;
			
			for (float distance = 1.0f; distance < 5.0f; distance += 0.5f)
			{
				for (float angle = 0; angle < M_PI * 2; angle += M_PI / 8.0f)
				{
					Vector2D view(1, 0);
					view.rotate(angle);
					Vector2D viewing_location =  view_point + view * distance;
					
					unsigned int visibility = 0;
					for (std::vector<Scene*>::const_iterator ci = environment_->getScenes().begin(); ci != environment_->getScenes().end(); ++ci)
					{
						const Scene* scene = *ci;
						if (scene->canSee(view_point, viewing_location))
						{
							++visibility;
						}
					}
					
					std::cout << " *** " << viewing_location << " -> " << visibility << ". Angle: " << angle << " - rot " << view << std::endl;
					
					if (visibility > most_scenes)
					{
						best_viewing_location = viewing_location;
						most_scenes = visibility;
					}
				}
			}
			
			// Generate its waypoint.
			if (most_scenes > 0)
			{
				Waypoint& inspection_point = Waypoint::generateWaypoint(best_viewing_location.x_, best_viewing_location.y_, atan2(view_point.y_ - best_viewing_location.y_, view_point.x_ - best_viewing_location.x_));
				inspection_points.push_back(&inspection_point);
			}
		}
	}
	
	// Show these inspection points in the visualiser.
	visualization_msgs::MarkerArray inspection_marker_array;
	for (std::vector<Waypoint*>::const_iterator ci = inspection_points.begin(); ci != inspection_points.end(); ++ci)
	{
		const Waypoint* waypoint = *ci;
		
		visualization_msgs::Marker marker;
		marker.header.frame_id = "/map";
		marker.header.stamp = ros::Time::now();
		marker.header.seq = inspection_marker_array.markers.size();
		marker.id = inspection_marker_array.markers.size();
		marker.ns = "Waypoints";
		marker.type = visualization_msgs::Marker::TRIANGLE_LIST;
		marker.action = visualization_msgs::Marker::ADD;
		marker.pose.orientation.w = 1.0f;
		marker.scale.x = 1;
		marker.scale.y = 1;
		marker.scale.z = 1;
		marker.color.a = 0.5;
		marker.color.b = 1;
		
		tf::Vector3 axis(0, 0, 1);
		tf::Quaternion q(axis, waypoint->theta_);
		
		marker.pose.orientation.x = q.x();
		marker.pose.orientation.y = q.y();
		marker.pose.orientation.z = q.z();
		marker.pose.orientation.w = q.w();
		marker.pose.position.x = waypoint->x_;
		marker.pose.position.y = waypoint->y_;
		marker.pose.position.z = 0.1f;
		geometry_msgs::Point point;
		marker.points.push_back(point);
		point.x = 2;
		point.y = -0.75;
		marker.points.push_back(point);
		point.y = 0.75;
		marker.points.push_back(point);
		inspection_marker_array.markers.push_back(marker);
	}
	inspection_viz_publisher_.publish(inspection_marker_array);
	faces.clear();
	view_points.clear();
#endif
	if (inspection_points.empty())
	{
		bool found_at_least_one_inspection_point = false;
		while (!found_at_least_one_inspection_point)
		{
			getInspectionPoints(inspection_points, aoi_id);
			
			std::cout << "Found " << inspection_points.size() << " inspection points.!" << std::endl;

			// Generate waypoints that are relatively close to the inspection points and make the inspection
			// points reachable from these waypoints.
			//for (std::vector<Waypoint*>::const_iterator ci = inspection_points.begin(); ci != inspection_points.end(); ++ci)
			for (std::vector<Waypoint*>::reverse_iterator ri = inspection_points.rbegin(); ri != inspection_points.rend(); ++ri)
			{
				Waypoint* inspection_point = *ri;
				Vector2D inspection_point_2d(inspection_point->x_, inspection_point->y_);
				bool is_valid_inspection_point = true;
				for (std::vector<Scene*>::const_iterator ci = environment_->getScenes().begin(); ci != environment_->getScenes().end(); ++ci)
				{
					Scene* scene = *ci;
					
					if (scene->isBlocked(inspection_point_2d, 0.25f))
					{
						is_valid_inspection_point = false;
						break;
					}
				}
				
				if (!is_valid_inspection_point)
				{
					inspection_points.erase((ri + 1).base());
					std::cout << "Ignore the inspection point as it is too close to possible obstructions: " << *inspection_point << "." << std::endl;
					continue;
				}
				
				Waypoint& observe_waypoint = Waypoint::generateWaypoint(inspection_point->x_, inspection_point->y_ + 1.0f, inspection_point->theta_);
				inspection_point->addEdge(observe_waypoint);
				observe_waypoint.addEdge(*inspection_point);
				waypoints.push_back(&observe_waypoint);
				
				std::cout << "Inspection point: (" << inspection_point->x_ << ", " << inspection_point->y_ << "); Theta: " << inspection_point->theta_ << std::endl;
				std::cout << "Waypoint: (" << observe_waypoint.x_ << ", " << observe_waypoint.y_ << "); Theta: " << observe_waypoint.theta_ << std::endl;
			}
			if (inspection_points.size() > 0)
			{
				found_at_least_one_inspection_point = true;
			}
			else
			{
				std::cout << "Found no suitable inspection points, retry!" << std::endl;
			}
		}
	}
	else
	{
		std::cout << "We are using alternative view cones!" << std::endl;
	}
	std::cout << "Loaded " << inspection_points.size() << " inspection points!" << std::endl;
	
	// Get all the scenes that are stored in the ontology.
#ifdef FLYING_TURTLEBOT_PLANNING_USE_ONTOLOGY
	START_TIMER; clock_t start = clock();

	environment_->reloadScenes(!disable_ontology_);

	END_TIMER("reloading scenes from ontology"); clock_t endt = clock();
	std::cout << "  - CPU time: " << (double(endt - start) / CLOCKS_PER_SEC) << "s" << std::endl;
#else
	environment_->reloadTestScenario();
#endif
	const std::vector<Scene*>& scenes = environment_->getScenes();

#ifdef FLYING_TURTLEBOT_PLANNING_USE_ONTOLOGY
	// The location where we start is the location of the robot.
	Waypoint& enter_waypoint = Waypoint::generateWaypoint(req.turtlebot_location.x, req.turtlebot_location.y, req.turtlebot_location.theta);
	waypoints.push_back(&enter_waypoint);
#else
	Waypoint& enter_waypoint = Waypoint::generateWaypoint(0, -4, 0);
	waypoints.push_back(&enter_waypoint);
#endif
	
	// We use the bottom left corner of the AoI as an exit, for now.
#ifdef FLYING_TURTLEBOT_PLANNING_USE_ONTOLOGY
	std::string bottom_left_corner_id = oa_->getStringUnique(aoi_id, "plan:'hasBottomLeft'");
	Waypoint& exit_waypoint = Waypoint::generateWaypoint(oa_->getFloatUnique(bottom_left_corner_id, "knowrob:'xCoord'"), oa_->getFloatUnique(bottom_left_corner_id, "knowrob:'yCoord'"), 0.0f);
	waypoints.push_back(&exit_waypoint);
#else
	Waypoint& exit_waypoint = Waypoint::generateWaypoint(0, -5, 0);
	waypoints.push_back(&exit_waypoint);
#endif
	
	std::cout << "Robot's location is at: (" << enter_waypoint.x_ << ", " << enter_waypoint.y_ << ")" << std::endl;

	// The inspectin points are a subset of all the waypoints; the only difference is that we expect to find the target by sensing
	// from an inspection point and the waypoints are solely there to get the robot to the inspection points.
	waypoints.insert(waypoints.end(), inspection_points.begin(), inspection_points.end());

	/**
	 * Debug :).
	 */
#ifndef FLYING_TURTLEBOT_PLANNING_USE_ONTOLOGY
	waypoints.push_back(&Waypoint::generateWaypoint(-7, 0, 0));
	waypoints.push_back(&Waypoint::generateWaypoint(7, 0, 0));
	waypoints.push_back(&Waypoint::generateWaypoint(6, 3, 0));
	waypoints.push_back(&Waypoint::generateWaypoint(-6, 3, 0));
#endif
	
	std::vector<Waypoint*> all_waypoints(waypoints);
	
	std::cout << "Found view points: " << std::endl;
	for (std::vector<Vector2D>::const_iterator ci = view_points.begin(); ci != view_points.end(); ++ci)
	{
		std::cout << "\t" << *ci << std::endl;
	}
	std::cout << "Found faces: " << std::endl;
	for (std::vector<const Face*>::const_iterator ci = faces.begin(); ci != faces.end(); ++ci)
	{
		std::cout << "\t" << **ci << std::endl;
	}
	//view_points.clear();
	//faces.clear();

	// Generate new waypoints until we can link every exit, entry, and inspection point in every possible scene.
	unsigned int loop_count = 0;
	std::vector<Waypoint*> unconnected_inspection_points;
	while (!environment_->isFullyConnected(enter_waypoint, exit_waypoint, all_waypoints, inspection_points, view_points, faces, unconnected_inspection_points))
	{
		bool is_valid = false;
		while (!is_valid)
		{
			++loop_count;
			if (all_waypoints.size() % 10 == 0)
			{
				std::cout << "(" << all_waypoints.size() << ")";
			}
			
			// Try reducing the number of inspection points if we fail to connect them up.
			if (all_waypoints.size() > 50)
			{
				std::cout << "Reduce the number of inspection points we are considering." << std::endl;
				for (int i = inspection_points.size() - 1; i > -1; --i)
				{
					if (std::find(unconnected_inspection_points.begin(), unconnected_inspection_points.end(), inspection_points[i]) != unconnected_inspection_points.end())
					{
						std::cout << "Remove: " << *inspection_points[i] << "." << std::endl;
						inspection_points.erase(inspection_points.begin() + i);
					}
				}
			}
			unconnected_inspection_points.clear();
			
			/*
			if (loop_count > 10000)
			{
				std::cerr << "Plan generation failed, cannot achieve all objectives!" << std::endl;
				return true;
			}
			*/
			// Add some random waypoints, that are not too close to each other.
			//Waypoint& waypoint = generateWaypoint(20.0f * ((float)rand() / (float)RAND_MAX) - 10.0f, 20.0f * ((float)rand() / (float)RAND_MAX) - 10.0f, 10.0f * ((float)rand() / (float)RAND_MAX) - 5.0f);
			float x_diff = aoi_max_x_ - aoi_min_x_;
			float y_diff = aoi_max_y_ - aoi_min_y_;
			
			//Waypoint& waypoint = generateWaypoint(x_diff * ((float)rand() / (float)RAND_MAX) + aoi_min_x_, y_diff * ((float)rand() / (float)RAND_MAX) + aoi_min_y_, 0);
			Waypoint* waypoint = new Waypoint("", "", x_diff * ((float)rand() / (float)RAND_MAX) + aoi_min_x_, y_diff * ((float)rand() / (float)RAND_MAX) + aoi_min_y_, 0);
			std::cerr << "New WP: " << waypoint->x_ << ", " << waypoint->y_ << std::endl;
			
			// Check if this waypoint is not too close to an existing waypoint.
			bool valid_waypoint = true;
			/*
			for (std::vector<Waypoint*>::const_iterator ci = all_waypoints.begin(); ci != all_waypoints.end(); ++ci)
			{
				float distance = sqrt(((*ci)->x_ - waypoint->x_) * ((*ci)->x_ - waypoint->x_) + ((*ci)->y_ - waypoint->y_) * ((*ci)->y_ - waypoint->y_));
				
				if (distance < 1.0f)
				{
					valid_waypoint = false;
					break;
				}
			}
			*/
			if (!valid_waypoint)
			{
				delete waypoint;
				continue;
			}
			else
			{
				is_valid = true;
				all_waypoints.push_back(waypoint);
			}
		}
	}
	
	// Check whether there are still some inspection points left.
	if (inspection_points.empty())
	{
		std::cout << "There are no valid inspection points, replanning." << std::endl;
		return true;
	}
	
	std::cout << "Reduce the number of waypoints. " << all_waypoints.size() << std::endl;
	
	
	/**
	 * Find the shortest paths between the inspection points and the starting point and 
	 * use that set as the waypoints we expose to the planner.
	 */
	std::set<Waypoint*> necessary_waypoints;
	for (std::vector<Scene*>::const_iterator ci = environment_->getScenes().begin(); ci != environment_->getScenes().end(); ++ci)
	{
		const Scene* scene = *ci;
		
		for (int i = 0; i < inspection_points.size(); ++i)
		{
			std::vector<Waypoint*> path = scene->findPath(enter_waypoint, *inspection_points[i], all_waypoints);
			necessary_waypoints.insert(path.begin(), path.end());
			
			for (int j = i + 1; j < inspection_points.size(); ++j)
			{
				path = scene->findPath(*inspection_points[i], *inspection_points[j], all_waypoints);
				necessary_waypoints.insert(path.begin(), path.end());
			}
		}
	}
	
	for (int i = all_waypoints.size() - 1; i > waypoints.size(); --i)
	{
		Waypoint* waypoint = all_waypoints[i];
		if (necessary_waypoints.count(all_waypoints[i]) != 1)
		{
			all_waypoints.erase(all_waypoints.begin() + i);
			delete waypoint;
		}
	}
	/*
	// Reduce the number of waypoints.
	for (int i = all_waypoints.size() - 1; i > waypoints.size(); --i)
	{
		Waypoint* waypoint = all_waypoints[i];
		all_waypoints.erase(all_waypoints.begin() + i);
		
		if (!environment_->isFullyConnected(enter_waypoint, exit_waypoint, all_waypoints, inspection_points, view_points, faces, unconnected_inspection_points))
		{
			all_waypoints.push_back(waypoint);
		}
		else
		{
			std::cout << "Remove " << *waypoint << "." << std::endl;
			delete waypoint;
		}
		unconnected_inspection_points.clear();
	}
	*/
	std::cout << "Create actual waypoints. " << std::endl;
	// Those remaining will be added to the actual waypoints.
	//for (std::vector<Waypoint*>::const_iterator ci = all_waypoints.begin(); ci != all_waypoints.end(); ++ci)
	for (unsigned int i = waypoints.size(); i < all_waypoints.size(); ++i)
	{
		Waypoint* tmp_waypoint = all_waypoints[i];
		
		Waypoint& waypoint = Waypoint::generateWaypoint(tmp_waypoint->x_, tmp_waypoint->y_, 0);
		waypoints.push_back(&waypoint);
		delete tmp_waypoint;
	}
	all_waypoints.clear();
	
	// Create a mapping from the predicates to the waypoint instances. The predicate string is always uppercase.
	visualization_msgs::MarkerArray marker_array;
	std::map<std::string, Waypoint*> predicate_to_waypoint_mapping;
	for (std::vector<Waypoint*>::const_iterator ci = waypoints.begin(); ci != waypoints.end(); ++ci)
	{
		const Waypoint* waypoint = *ci;
		predicate_to_waypoint_mapping[(*ci)->predicate_] = *ci;
		std::cout << "Mapped: " << (*ci)->predicate_ << " to " << (*ci)->ontology_id_ << " (" << (*ci)->x_ << ", " << (*ci)->y_ << ", " << (*ci)->theta_ << ")" << std::endl;
		
		visualization_msgs::Marker marker;
		marker.header.frame_id = "/map";
		marker.header.stamp = ros::Time::now();
		marker.header.seq = marker_array.markers.size();
		marker.id = marker_array.markers.size();
		marker.ns = "Waypoints";
		marker.type = visualization_msgs::Marker::CUBE;
		marker.action = visualization_msgs::Marker::ADD;
		marker.pose.orientation.w = 1.0f;
		marker.scale.x = 0.1f;
		marker.scale.y = 0.1f;
		marker.scale.z = 0.1f;
		marker.color.a = 1;
		marker.color.b = 1;
		
		marker.pose.orientation.w = 1;
		marker.pose.position.x = waypoint->x_;
		marker.pose.position.y = waypoint->y_;
		
		marker_array.markers.push_back(marker);
		
		// Add the text too.
		marker.header.seq = marker_array.markers.size();
		marker.type = visualization_msgs::Marker::TEXT_VIEW_FACING;
		marker.scale.x = 0.1f;
		marker.scale.y = 0.1f;
		marker.scale.z = 0.1f;
		marker.text = waypoint->predicate_;
		marker_array.markers.push_back(marker);
		
		for (std::vector<Waypoint*>::const_iterator ci = waypoints.begin(); ci != waypoints.end(); ++ci)
		{
			const Waypoint* other_waypoint = *ci;
			bool can_connect = true;
			for (std::vector<Scene*>::const_iterator ci = environment_->getScenes().begin(); ci != environment_->getScenes().end(); ++ci)
			{
				const Scene* scene = *ci;
				
				if (!scene->canConnect(Vector2D(other_waypoint->x_, other_waypoint->y_), Vector2D(waypoint->x_, waypoint->y_)))
				{
					can_connect = false;
					break;
				}
			}
			
			if (can_connect)
			{
				visualization_msgs::Marker line_marker;
				line_marker.header.frame_id = "/map";
				line_marker.header.stamp = ros::Time::now();
				line_marker.header.seq = marker_array.markers.size();
				line_marker.id = marker_array.markers.size();
				line_marker.ns = "Waypoints";
				line_marker.type = visualization_msgs::Marker::LINE_STRIP;
				line_marker.action = visualization_msgs::Marker::ADD;
				line_marker.pose.orientation.w = 1.0f;
				line_marker.scale.x = 0.01f;
				line_marker.scale.y = 0.01f;
				line_marker.scale.z = 0.01f;
				line_marker.color.a = 1;
				line_marker.color.b = 1;
				line_marker.color.g = 0.5;
				
				line_marker.pose.orientation.w = 1;
				//line_marker.pose.position.x = waypoint->x_;
				//line_marker.pose.position.y = waypoint->y_;
				
				geometry_msgs::Point p1;
				p1.x = waypoint->x_;
				p1.y = waypoint->y_;
				p1.z = 0;
				
				line_marker.points.push_back(p1);
				
				std_msgs::ColorRGBA colour;
				
				colour.r = 0.0;
				colour.g = 0.0;
				colour.b = 1.0;
				colour.a = 1.0;
				line_marker.colors.push_back(colour);
				
				geometry_msgs::Point p2;
				p2.x = other_waypoint->x_;
				p2.y = other_waypoint->y_;
				p2.z = 0;
				
				line_marker.points.push_back(p2);
				
				colour.r = 0.0;
				colour.g = 0.0;
				colour.b = 1.0;
				colour.a = 1.0;
				line_marker.colors.push_back(colour);
				marker_array.markers.push_back(line_marker);
			}
		}
	}
	waypoint_viz_publisher_.publish(marker_array);
	
	std::cout << "Processed " << inspection_points.size() << " inspection points." << std::endl;
	std::cout << "Processed " << scenes.size() << " scenes." << std::endl;
	std::cout << "All the points are fully connected with " << waypoints.size() << " waypoints." << std::endl;
	
	std::vector<Scene*> most_probably_scenes;
	environment_->getMostProbableScenes(most_probably_scenes, 10);
	
	// Create the planning domain and problem.
	std::ofstream domain_file;
	domain_file.open("mars_domain.pddl");
	generateDomainFile(domain_file, inspection_points, waypoints, enter_waypoint, exit_waypoint, view_points, faces, most_probably_scenes);
	domain_file.close();

	std::ofstream problem_file;
	problem_file.open("mars_problem.pddl");
	generateProblemFile(problem_file, inspection_points, waypoints, enter_waypoint, exit_waypoint, view_points, faces, most_probably_scenes);
	problem_file.close();

	// Run the planner.
	std::cout << "Start the planner!" << std::endl;
	ros::WallTime start_ff = ros::WallTime::now();
	std::stringstream ss;
	ss << planner_command_line_ << " -o mars_domain.pddl -f mars_problem.pddl &> out";
	FILE* file = popen(ss.str().c_str(), "r");
	
	char c;
	std::stringstream plan_ss;
	while ((c = fgetc(file)) != EOF)
	{
		plan_ss << c;
		printf("%c", c);
	}
	pclose(file);
	ros::WallTime end_ff = ros::WallTime::now();
	std::cout << "Found a plan!" << std::endl;
	//system(ss.str().c_str());

	ontology_db::CreateInstanceOfClass create_instance;
	create_instance.request.class_name = "plan:'ContingentPlan'";
	if (!create_instances_client_.call(create_instance))
	{
		std::cout << "Failed to add a contingent plan to the ontology!" << std::endl;
	}
	std::cout << "Created a contingent plan instance with the name: " << create_instance.response.instance_name << std::endl;
	std::string contingent_plan_instance_name = create_instance.response.instance_name;
	
	// Add this contingency plan to the AoI.
	ontology_db::CreateStringPropertyValues aoi_has_plan_property;
	aoi_has_plan_property.request.instance_name = aoi_id;
	aoi_has_plan_property.request.property_name = "plan:'planForArea'";
	aoi_has_plan_property.request.values.push_back(contingent_plan_instance_name);
	if (!create_sproperty_client_.call(aoi_has_plan_property))
	{
		std::cerr << "Could not add the contingency plan: " << contingent_plan_instance_name << " to the AoI " << aoi_id << std::endl;
	}
	else
	{
		std::cout << "Add the contingency plan: " << contingent_plan_instance_name << " to the AoI " << aoi_id << std::endl;
	}

	// Read the out file and store the resulting plan in the ontology.
	size_t prev_i = 0;
	size_t next_i = 0;
	bool found_first_action = false;
	std::vector<std::string> stack;
	std::string last_added_action;

	BRANCH current_branch = FIRST;
	
	while ((next_i = plan_ss.str().find('\n', prev_i)) != std::string::npos)
	{
		std::string s = plan_ss.str().substr(prev_i, next_i - prev_i);
		
		std::cout << "Process: " << s << std::endl;
		
		prev_i = next_i + 1;
		if (!found_first_action && s.find("step    0:") != std::string::npos)
		{
			found_first_action = true;
		}

		if (!found_first_action) continue;

		size_t action_i = s.find(':');	
		if (action_i != std::string::npos)
		{
			std::cout << "A: " << s.substr(action_i + 2, s.size() - (action_i + 2)) << std::endl;
			std::string full_action = s.substr(action_i + 2, s.size() - (action_i + 2));

			std::string action_name = s.substr(action_i + 2, s.find(" ", action_i + 2) - (action_i + 2));
			std::cout << "Processing: " << action_name << std::endl;
			if ("RAMINIFICATE" == action_name)
			{
				std::cout << "Skipping " << action_name << std::endl;
				continue;
			}
			else if ("POP" == action_name)
			{
				std::cout << "POP! - stack size " << stack.size() << std::endl;
				last_added_action = stack[stack.size() - 1];
				stack.erase(stack.end() - 1);
				current_branch = FAIL;
				continue;
			}
			
			// Add the action to the response.
			std::vector<std::string> action_parameters;
			char * action_copy = new char[full_action.length() + 1];
			std::strcpy(action_copy, full_action.c_str());
			char *p = strtok(action_copy, " ");
			while (p)
			{
				action_parameters.push_back(std::string(p));
				p = strtok(NULL, " ");
			}
			
			//if ("SENSE" == action_name || "OBSERVE-WALL" == action_name || "SENSE-VIEW-CONE" == action_name)
			if ("OBSERVE-WALL" == action_name)
			{
				create_instance.request.class_name = "plan:'ObservationAction'";
			}
			else
			{
				create_instance.request.class_name = "plan:'PlanAction'";
			}
			if (!create_instances_client_.call(create_instance))
			{
				std::cout << "Failed to add an action to the ontology!" << std::endl;
			}
			std::string action_instance_name = create_instance.response.instance_name;
			std::cout << "Created an action instance with the name: " << action_instance_name << std::endl;
			
			// Set the properties.
			std::vector<std::string> name_list;
			name_list.push_back(full_action);
			ontology_db::CreateStringPropertyValues set_svalues;
			set_svalues.request.instance_name = action_instance_name;
			set_svalues.request.property_name = "plan:'actionName'";
			set_svalues.request.values = name_list;
			if (!create_sproperty_client_.call(set_svalues))
			{
				std::cerr << "Could not set the action name for the action: " << action_instance_name << " " << full_action << std::endl;
			}
			else
			{
				std::cout << "Action name is set: " << action_instance_name << " " << full_action << std::endl;
			}
			
			// Set the waypoint, this is the destination for navigate action and the target for sensing actions.
			if ("NAVIGATE" == action_name)
			{
				std::vector<std::string> waypoint_list;
				waypoint_list.push_back(predicate_to_waypoint_mapping[action_parameters[3]]->ontology_id_);
				set_svalues.request.property_name = "plan:'hasWaypoint'";
				set_svalues.request.values = waypoint_list;
				
				std::cout << "Create the waypoint " << waypoint_list[0] << std::endl;
				
				if (!create_sproperty_client_.call(set_svalues))
				{
					std::cerr << "Could not set the waypoint for the action: " << action_instance_name << " " << full_action << std::endl;
				}
				else
				{
					std::cout << "Set the waypoint to: " << predicate_to_waypoint_mapping[action_parameters[3]]->ontology_id_ << "(" << predicate_to_waypoint_mapping[action_parameters[3]]->ontology_id_ <<")." << std::endl;
				}
			}
			else if ("SENSE-VIEW-CONE" == action_name || "OBSERVE-WALL" == action_name || "OBSERVE-WALL-NO-BRANCH" == action_name)
			{
				std::string view_pose = ("SENSE-VIEW-CONE" == action_name) ? action_parameters[2] : action_parameters[3];
				std::cerr << "Sense-type action, view-pose=" << view_pose << std::endl;
				// TODO: Need to extract the targets from the ontology.
				std::string inspection_point_name;
				for (std::vector<Waypoint*>::const_iterator ci = waypoints.begin(); ci != waypoints.end(); ++ci)
				{
					if (view_pose == (*ci)->predicate_)
					{
						inspection_point_name = (*ci)->ontology_id_;
					}
				}
				
				std::vector<std::string> waypoint_list;
				//waypoint_list.push_back("target");
				waypoint_list.push_back(inspection_point_name);
				set_svalues.request.property_name = "plan:'hasWaypoint'";
				set_svalues.request.values = waypoint_list;
				if (!create_sproperty_client_.call(set_svalues))
				{
					std::cerr << "Could not set the waypoint for the target: " << action_instance_name << " " << full_action << std::endl;
				}
			}
			else if ("OBSERVE-VIEW-CONE" == action_name)
			{
				// TODO: Need to extract the targets from the ontology.
				std::string inspection_point_name;
				for (std::vector<Waypoint*>::const_iterator ci = waypoints.begin(); ci != waypoints.end(); ++ci)
				{
					if (action_parameters[1] == (*ci)->predicate_)
					{
						inspection_point_name = (*ci)->ontology_id_;
					}
				}
				
				std::vector<std::string> waypoint_list;
				//waypoint_list.push_back("target");
				waypoint_list.push_back(inspection_point_name);
				set_svalues.request.property_name = "plan:'hasWaypoint'";
				set_svalues.request.values = waypoint_list;
				if (!create_sproperty_client_.call(set_svalues))
				{
					std::cerr << "Could not set the waypoint for the target: " << action_instance_name << " " << full_action << std::endl;
				}
			}

			std::vector<std::string> action_list;
			action_list.push_back(action_instance_name);
			set_svalues.request.instance_name = last_added_action;
			set_svalues.request.values = action_list;
			last_added_action = action_instance_name;

			switch (current_branch)
			{
				case FIRST: 
					res.first_action = action_instance_name;
					set_svalues.request.property_name = "plan:'hasFirstAction'"; 
					// Over-write previously set value for instance name
					set_svalues.request.instance_name = contingent_plan_instance_name;
					std::cout << "Add hasFirstAction: instance: " << set_svalues.request.instance_name <<
											"; Property:" << set_svalues.request.property_name << "; Values: " <<
											set_svalues.request.values[0] << std::endl;
					break;
				case NORMAL: 
					set_svalues.request.property_name = "plan:'hasNextAction'"; 
					break;
				case SUCCESS: 
					set_svalues.request.property_name = "plan:'hasSuccessAction'"; 
					break;
				case FAIL: 
					set_svalues.request.property_name = "plan:'hasFailureAction'"; 
					break;
				default: 
				{
					std::cout << "IMPOSSIBLE!" << std::endl;
					return -1;
				}
			}

			if ("OBSERVE-WALL" == action_name)
			{
				std::cout << "Branch here!" << std::endl;
				stack.push_back(action_instance_name);
				current_branch = SUCCESS;
			}
			else
			{
				current_branch = NORMAL;
			}

			if (!create_sproperty_client_.call(set_svalues))
			{
				std::cerr << "Could not set the properties for the action: " << last_added_action << std::endl;
			}
			//current_pddl_action = next_pddl_action;
		}
		else if (found_first_action)
		{
			break;
		}
	}
	
	// Clean the memory.
	for (std::vector<Waypoint*>::const_iterator ci = waypoints.begin(); ci != waypoints.end(); ++ci)
	{
		delete *ci;
	}

	// Record plan generation statistics
	ros::WallTime end_gen_plan = ros::WallTime::now();
	ros::WallDuration ff_duration = end_ff - start_ff;
	ros::WallDuration plan_duration = (start_ff - start_gen_plan) + (end_gen_plan - end_ff);
	static const std::string ENV("oslshape:'TheEnvironment'");
	float ff_time_so_far = oa_->getFloatUnique(ENV, "oslshape:'ffRuntime'");
	float plan_time_so_far = oa_->getFloatUnique(ENV, "oslshape:'plannerRuntime'");
	int runs_so_far = oa_->getIntUnique(ENV, "oslshape:'numPlanGenerations'");
	oa_->removeAllInstancePropertyValues(ENV, "oslshape:'ffRuntime'");
	oa_->removeAllInstancePropertyValues(ENV, "oslshape:'plannerRuntime'");
	oa_->removeAllInstancePropertyValues(ENV, "oslshape:'numPlanGenerations'");
	oa_->setFloatProperty(ENV, "oslshape:'ffRuntime'", ff_time_so_far + ff_duration.toSec());
	oa_->setFloatProperty(ENV, "oslshape:'plannerRuntime'", plan_time_so_far + plan_duration.toSec());
	oa_->setIntProperty(ENV, "oslshape:'numPlanGenerations'", runs_so_far + 1);
	return true;
}

bool CPGenerator::getFaceCoordinates(turtlebot_common::GetFaceCoordinates::Request &req, turtlebot_common::GetFaceCoordinates::Response& res)
{
	// Find the face name and report back the coordinates (if it is found).
	const Face* face = environment_->getFace(req.face_name);
	if (face != NULL)
	{
		res.p1_x = face->getP1().x_;
		res.p1_y = face->getP1().y_;
		res.p2_x = face->getP2().x_;
		res.p2_y = face->getP2().y_;
		res.hue = face->getHue();
		res.saturation = face->getSaturation();
		res.value = face->getValue();
		return true;
	}
	return false;
}

void CPGenerator::generateProblemFile(std::ofstream& o, const std::vector<Waypoint*>& inspection_points, const std::vector<Waypoint*>& waypoints, const Waypoint& enter, const Waypoint& exit, const std::vector<Vector2D>& view_points, const std::vector<const Face*>& faces, const std::vector<Scene*>& scenes)
{
	//unsigned int nr_states = inspection_points.size() * scenes.size();
	unsigned int nr_states = scenes.size();
	std::cout << "Number of states is: " << nr_states << std::endl;
	o << "(define (problem Mars-3)" << std::endl;
	o << "(:domain Mars)" << std::endl;
	o << "(:objects" << std::endl;
	
	for (unsigned int i = 0; i < faces.size() + view_points.size() + 2; ++i)
	{
		o << "l" << i << " - LEVEL" << std::endl;
	}
	
	for (unsigned int i = 0; i < view_points.size(); ++i)
	{
		o << "view_target" << i << " - objective" << std::endl;
	}
	
	for (std::vector<Face*>::const_iterator ci = Face::getFaces().begin(); ci != Face::getFaces().end(); ++ci)
	{
		o << (*ci)->getPDDLName() << " - wall" << std::endl;
	}
	o << ")" << std::endl;
	o << " (:init" << std::endl;
	o << " (resolve-axioms)" << std::endl;
	o << " (lev l0)" << std::endl;
	
	// NOTE: Temporary 'fix' to make the planning process quicker.
	for (unsigned int i = 0; i < faces.size() + view_points.size() + 1; ++i)
	{
		o << "(next l" << i << " l" << (i + 1) << ")" << std::endl;
	}
	for (unsigned int i = 0; i < nr_states; ++i)
	{
		o << "(m s" << i << ")" << std::endl;
		o << "(at turtlebot " << enter.predicate_ <<" s" << i << ")" << std::endl;
		//o << "(at turtlebot enter s" << i << ")" << std::endl;
	}
	
	for (std::vector<Waypoint*>::const_iterator ci = inspection_points.begin(); ci != inspection_points.end(); ++ci)
	{
		o << "(isViewCone " << (*ci)->predicate_ << ")" << std::endl;
	}
	
	// Process the scenes and store all the faces (and their relationship to the scenes) in here.
	std::vector<Face*> processed_faces;
	unsigned int scene_nr = 0;
	for (std::vector<Scene*>::const_iterator ci = scenes.begin(); ci != scenes.end(); ++ci, ++scene_nr)
	{
		const Scene* scene = *ci;
		
		// Connect the waypoints given this scene.
		// * canTraverse
		//for (std::vector<Waypoint*>::const_iterator ci = waypoints.begin(); ci != waypoints.end(); ++ci)
		for (unsigned int i = 0; i < waypoints.size(); ++i)
		{
			const Waypoint* waypoint = waypoints[i];
			unsigned int view_point_id = 0;
			for (std::vector<Vector2D>::const_iterator ci = view_points.begin(); ci != view_points.end(); ++ci)
			{
				if (scene->canConnect(*ci, Vector2D(waypoint->x_, waypoint->y_)))
				{
					o << "(visibleFrom view_target" << view_point_id << " " << waypoint->predicate_ << " s" << scene_nr << ")" << std::endl;
				}
				++view_point_id;
			}
			
			//for (std::vector<std::pair<float, Waypoint*> >::const_iterator ci = waypoint->edges_.begin(); ci != waypoint->edges_.end(); ++ci)
			//for (std::vector<Waypoint*>::const_iterator ci2 = waypoints.begin(); ci2 != waypoints.end(); ++ci2)
			for (unsigned int j = i + 1; j < waypoints.size(); ++j)
			{
				const Waypoint* other_waypoint = waypoints[j];
				
				if (waypoint == other_waypoint) continue;
				
				if (waypoint->predicate_ == other_waypoint->predicate_)
				{
					std::cerr << "Waypoints cannot have the same predicate!" << std::endl;
					::exit(1);
				}
				
				if (scene->canConnect(Vector2D(waypoint->x_, waypoint->y_), Vector2D(other_waypoint->x_, other_waypoint->y_)))
				{
					//std::cout << waypoint->predicate_ << " is connected to " << other_waypoint->predicate_ << std::endl;
					o << "(canTraverse turtlebot " << other_waypoint->predicate_ << " " << waypoint->predicate_ <<  " s" << scene_nr << ")" << std::endl;
					o << "(canTraverse turtlebot " << waypoint->predicate_ << " " << other_waypoint->predicate_ <<  " s" << scene_nr << ")" << std::endl;
				}
			}
		}
		
		// Now process all the shapes and faces for each scene and record these in the planning problem.
		// * hasWall
		// * canSee
		for (std::vector<Shape*>::const_iterator ci = scene->getShapes().begin(); ci != scene->getShapes().end(); ++ci)
		{
			Shape* shape = *ci;
			for (std::vector<const Face*>::const_iterator ci = shape->getFaces().begin(); ci != shape->getFaces().end(); ++ci)
			{
				const Face* face = *ci;
				o << "(hasWall " << face->getPDDLName() << " s" << scene_nr << ")" << std::endl;
			}
		}
		
		for (std::vector<Face*>::const_iterator ci = Face::getFaces().begin(); ci != Face::getFaces().end(); ++ci)
		{
			const Face* face = *ci;
			for (std::vector<Waypoint*>::const_iterator ci = waypoints.begin(); ci != waypoints.end(); ++ci)
			{
				const Waypoint* waypoint = *ci;
				if (scene->canSee(*face, Vector2D(waypoint->x_, waypoint->y_)) ||
				    scene->getFace(face->getPDDLName()) == NULL)
				{
					//for (unsigned int state_nr = 0; state_nr < scenes.size(); ++state_nr)
					{
						//o << "(canObserve " << face->getPDDLName() << " " << waypoint->predicate_ << " s" << state_nr << ")" << std::endl;
						o << "(canObserve " << face->getPDDLName() << " " << waypoint->predicate_ << " s" << scene_nr << ")" << std::endl;
					}
				}
			}
		}
	}

	o << " )" << std::endl;
	o << " (:goal (and" << std::endl;
	for (unsigned int i = 0; i < nr_states; ++i)
	{
		for (std::vector<const Face*>::const_iterator ci = faces.begin(); ci != faces.end(); ++ci)
		{
			o << "\t\t\t(observedWall " << (*ci)->getPDDLName() << " s" << i <<")" << std::endl;
		}
		for (unsigned int j = 0; j < view_points.size(); ++j)
		{
			o << "\t\t\t(observed view_target" << j << " s" << i << ")" << std::endl;
		}
		for (std::vector<Waypoint*>::const_iterator ci = inspection_points.begin(); ci != inspection_points.end(); ++ci)
		{
			o << "\t\t\t(observedViewCone " << (*ci)->predicate_ << " s" << i << ")" << std::endl;
		}
//		o << "(at turtlebot " << exit.predicate_ << " s" << i << ")" << std::endl;
	}
	o << " )" << std::endl;
	o << ")" << std::endl;
	o << ")" << std::endl;
}

void CPGenerator::generateDomainFile(std::ofstream& o, const std::vector<Waypoint*>& inspection_points, const std::vector<Waypoint*>& waypoints, const Waypoint& enter, const Waypoint& exit, const std::vector<Vector2D>& view_points, const std::vector<const Face*>& faces, const std::vector<Scene*>& scenes)
{
	unsigned int nr_states = scenes.size();
	bool consider_walls = scenes.size() > 1 || !scenes[0]->getShapes().empty();
	o << "(define (domain Mars)" << std::endl;
	o << "(:requirements :typing :conditional-effects :negative-preconditions :disjunctive-preconditions)" << std::endl;
	o << "(:types rover waypoint objective wall LEVEL STATE)" << std::endl;

	o << "(:predicates " << std::endl;
	o << "\t(at ?r - rover ?w - waypoint ?s - STATE) " << std::endl;
	o << "\t(Rat ?r - rover ?w - waypoint ?s - STATE)" << std::endl;
	
	o << "\t(canTraverse ?r - rover ?x - waypoint ?y - waypoint ?s - STATE)" << std::endl;
	o << "\t(RcanTraverse ?r - rover ?x - waypoint ?y - waypoint ?s - STATE)" << std::endl;
	
	if (view_points.size() > 0)
	{
		o << "\t(visibleFrom ?o - objective ?w - waypoint ?s - STATE)" << std::endl;
		o << "\t(RvisibleFrom ?o - objective ?w - waypoint ?s - STATE)" << std::endl;
		
		o << "\t(observed ?o - objective ?s - STATE)" << std::endl;
		o << "\t(Robserved ?o - objective ?s - STATE)" << std::endl;
	}
	
	o << "\t;Some waypoints are view cones that need to be observed." << std::endl;
	o << "\t(isViewCone ?w - waypoint)" << std::endl;
	o << "\t(observedViewCone ?w - waypoint ?s - STATE)" << std::endl;
	o << "\t(RobservedViewCone ?w - waypoint ?s - STATE)" << std::endl;

	if (consider_walls)
	{
		o << "\t(hasWall ?w - wall ?s - STATE)" << std::endl;
		o << "\t(RhasWall ?w - wall ?s - STATE)" << std::endl;
		o << "\t(canObserve ?w - wall ?w - waypoint ?s - STATE)" << std::endl;
		o << "\t(RcanObserve ?w - wall ?w - waypoint ?s - STATE)" << std::endl;
		o << "\t(observedWall ?w - wall ?s - STATE)" << std::endl;
		o << "\t(RobservedWall ?w - wall ?s - STATE)" << std::endl;
	}
	
	o << "\t; Specific for the translation." << std::endl;
	o << "\t(lev ?l - LEVEL)" << std::endl;
	o << "\t(m ?s - STATE)" << std::endl;
	o << "\t(stack ?s - STATE ?l - LEVEL)" << std::endl;
	o << "\t(next ?l ?l2 - LEVEL)" << std::endl;
	o << "\t(resolve-axioms)" << std::endl;
	o << "\t(Rgoal-reached)" << std::endl;
	o << ")" << std::endl;

	o << "(:constants" << std::endl;
	o << "\t; All the waypoints." << std::endl;
	o << "\tenter - waypoint" << std::endl;
	for (std::vector<Waypoint*>::const_iterator ci = waypoints.begin(); ci != waypoints.end(); ++ci)
	{
		o << "\t" << (*ci)->predicate_ << " - waypoint" << std::endl;
	}
	o << "\texit - waypoint" << std::endl;

	o << "\t; The turtlebot." << std::endl;
	o << "\tturtlebot - rover" << std::endl;

	o << "\t; All the states." << std::endl;
	for (unsigned int i = 0; i < nr_states; ++i)
	{
		o << "\ts" << i << " - STATE" << std::endl;
	}
	o << ")" << std::endl;

	/**
	 * Raminificate action.
	 */
	o << "(:action raminificate" << std::endl;
	o << "\t:parameters ()" << std::endl;
	o << "\t:precondition (resolve-axioms)" << std::endl;
	o << "\t:effect (and " << std::endl;
	o << "\t\t; at enter" << std::endl;

	for (unsigned int i = 0; i < nr_states; ++i)
	{
		for (std::vector<Waypoint*>::const_iterator ci = waypoints.begin(); ci != waypoints.end(); ++ci)
		{
			o << "\t\t(when (or (at turtlebot " << (*ci)->predicate_ << " s" << i << ") (not (m s" << i << ")))" << std::endl;
			o << "\t\t(Rat turtlebot " << (*ci)->predicate_ << " s" << i << "))" << std::endl;
			o << "\t\t(when (and (not (at turtlebot " << (*ci)->predicate_ << " s" << i << ")) (m s" << i << "))" << std::endl;
			o << "\t\t(not (Rat turtlebot " << (*ci)->predicate_ << " s" << i << ")))" << std::endl;
		}
	}

	for (unsigned int i = 0; i < nr_states; ++i)
	{
		// Process the inspection points.
		for (std::vector<Waypoint*>::const_iterator ci = inspection_points.begin(); ci != inspection_points.end(); ++ci)
		{
			o << "\t(when (or (observedViewCone " << (*ci)->predicate_ << " s" << i << ") (not (m s" << i << ")))" << std::endl;
			o << "\t(RobservedViewCone " << (*ci)->predicate_ << " " << "s" << i << "))" << std::endl;
			o << "\t(when (and (not (observedViewCone " << (*ci)->predicate_ << " s" << i << ")) (m s" << i << "))" << std::endl;
			o << "\t(not (RobservedViewCone " << (*ci)->predicate_ << " s" << i << ")))" << std::endl;
		}
		
		for (unsigned int j = 0; j < view_points.size(); ++j)
		{
			o << "\t\t(when (or (observed view_target" << j << " s" << i << ") (not (m s" << i << ")))" << std::endl;
			o << "\t\t(Robserved view_target" << j << " s" << i << "))" << std::endl;
			o << "\t\t(when (and (not (observed view_target" << j << " s" << i << ")) (m s" << i << "))" << std::endl;
			o << "\t\t(not (Robserved view_target" << j << " s" << i << ")))" << std::endl;
		}
		
		for (std::vector<Waypoint*>::const_iterator ci = waypoints.begin(); ci != waypoints.end(); ++ci)
		{
			const Waypoint* waypoint = *ci;
			o << "\t\t;s" << i << std::endl;
			//o << "\t\t(when (or (visibleFrom target " << (*ci)->predicate_ << " s" << i << ") (not (m s" << i << ")))" << std::endl;
			//o << "\t\t(RvisibleFrom target " << (*ci)->predicate_ << " s" << i << "))" << std::endl;
			//o << "\t\t(when (and (not (visibleFrom target " << (*ci)->predicate_ << " s" << i << ")) (m s" << i << "))" << std::endl;
			//o << "\t\t(not (RvisibleFrom target " << (*ci)->predicate_ << " s" << i << ")))" << std::endl;
			
			for (std::vector<Waypoint*>::const_iterator ci2 = waypoints.begin(); ci2 != waypoints.end(); ++ci2)
			{
				o << "\t(when (or (canTraverse turtlebot " << (*ci)->predicate_ << " " << (*ci2)->predicate_ << " s" << i << ") (not (m s" << i << ")))" << std::endl;
				o << "\t(RcanTraverse turtlebot " << (*ci)->predicate_ << " " << (*ci2)->predicate_ << " " << "s" << i << "))" << std::endl;
				o << "\t(when (and (not (canTraverse turtlebot " << (*ci)->predicate_ << " " << (*ci2)->predicate_ << " s" << i << ")) (m s" << i << "))" << std::endl;
				o << "\t(not (RcanTraverse turtlebot " << (*ci)->predicate_ << " " << (*ci2)->predicate_ << " s" << i << ")))" << std::endl;
			}
			
			for (unsigned int j = 0; j < view_points.size(); ++j)
			{
				o << "\t\t(when (or (visibleFrom view_target" << j << " " << (*ci)->predicate_ << " s" << i << ") (not (m s" << i << ")))" << std::endl;
				o << "\t\t(RvisibleFrom view_target" << j << " " << (*ci)->predicate_ << " s" << i << "))" << std::endl;
				o << "\t\t(when (and (not (visibleFrom view_target" << j << " " << (*ci)->predicate_ << " s" << i << ")) (m s" << i << "))" << std::endl;
				o << "\t\t(not (RvisibleFrom view_target" << j << " " << (*ci)->predicate_ << " s" << i << ")))" << std::endl;
			}
			
			for (std::vector<Face*>::const_iterator ci = Face::getFaces().begin(); ci != Face::getFaces().end(); ++ci)
			{
				const Face* face = *ci;
				//std::cout << face->getName() << std::endl;
				//std::flush(std::cout);
				//std::cout << face->getPDDLName() << std::endl;
				//std::flush(std::cout);
				
				o << "\t(when (or (canObserve " << (*ci)->getPDDLName() << " " << waypoint->predicate_ << " s" << i << ") (not (m s" << i << ")))" << std::endl;
				o << "\t(RcanObserve " << (*ci)->getPDDLName() << " " << waypoint->predicate_ << " s" << i << "))" << std::endl;
				o << "\t(when (and (not (canObserve " << (*ci)->getPDDLName() << " " << waypoint->predicate_ << " s" << i << ")) (m s" << i << "))" << std::endl;
				o << "\t(not (RcanObserve " << (*ci)->getPDDLName() << " " << waypoint->predicate_ << " s" << i << ")))" << std::endl;
				
				
				o << "\t(when (or (observedWall " << (*ci)->getPDDLName() << " " << " s" << i << ") (not (m s" << i << ")))" << std::endl;
				o << "\t(RobservedWall " << (*ci)->getPDDLName() << " " << " s" << i << "))" << std::endl;
				o << "\t(when (and (not (observedWall " << (*ci)->getPDDLName() << " " << " s" << i << ")) (m s" << i << "))" << std::endl;
				o << "\t(not (RobservedWall " << (*ci)->getPDDLName() << " " << " s" << i << ")))" << std::endl;
				
				o << "\t(when (or (hasWall " << (*ci)->getPDDLName() << " " << " s" << i << ") (not (m s" << i << ")))" << std::endl;
				o << "\t(RhasWall " << (*ci)->getPDDLName() << " " << " s" << i << "))" << std::endl;
				o << "\t(when (and (not (hasWall " << (*ci)->getPDDLName() << " " << " s" << i << ")) (m s" << i << "))" << std::endl;
				o << "\t(not (RhasWall " << (*ci)->getPDDLName() << " " << " s" << i << ")))" << std::endl;
			}
		}
	}
	o << "\t\t(when (and " << std::endl;
	for (unsigned int i = 0; i < nr_states; ++i)
	{
		for (std::vector<const Face*>::const_iterator ci = faces.begin(); ci != faces.end(); ++ci)
		{
			o << "\t\t\t(observedWall " << (*ci)->getPDDLName() << " s" << i <<")" << std::endl;
		}
		for (unsigned int j = 0; j < view_points.size(); ++j)
		{
			o << "\t\t\t(observed view_target" << j << " s" << i << ")" << std::endl;
		}
		for (std::vector<Waypoint*>::const_iterator ci = inspection_points.begin(); ci != inspection_points.end(); ++ci)
		{
			o << "\t\t\t(observedViewCone " << (*ci)->predicate_ << " s" << i << ")" << std::endl;
		}
		//o << "\t\t\t(observed target s" << i << ")" << std::endl;
		//o << "\t\t\t(at turtlebot " << exit.predicate_ << " s" << i << ")" << std::endl;
	}
	o << "\t\t) (Rgoal-reached))" << std::endl;
	o << "\t\t(not (resolve-axioms)))" << std::endl;
	o << ")" << std::endl;

	/**
	 * POP action.
	 */
	o << "(:action pop" << std::endl;
	o << "\t :parameters (?l - LEVEL ?l2 - LEVEL)" << std::endl;
	o << "\t :precondition (and (lev ?l) (next ?l2 ?l) (not (resolve-axioms))" << std::endl;
	for (unsigned int i = 0; i < nr_states; ++i)
	{
		//o << "\t(Robserved target s" << i << ")" << std::endl;
		
		for (std::vector<const Face*>::const_iterator ci = faces.begin(); ci != faces.end(); ++ci)
		{
			o << "\t\t\t(RobservedWall " <<(*ci)->getPDDLName() << " s" << i <<")" << std::endl;
		}
		for (unsigned int j = 0; j < view_points.size(); ++j)
		{
			o << "\t\t\t(Robserved view_target" << j << " s" << i << ")" << std::endl;
		}
		for (std::vector<Waypoint*>::const_iterator ci = inspection_points.begin(); ci != inspection_points.end(); ++ci)
		{
			o << "\t\t\t(RobservedViewCone " << (*ci)->predicate_ << " s" << i << ")" << std::endl;
		}
		//o << "\t\t\t(observed target s" << i << ")" << std::endl;
		//o << "\t\t\t(at turtlebot " << exit.predicate_ << " s" << i << ")" << std::endl;
	}
	o << "\t)" << std::endl;

	o << "\t :effect (and (not (lev ?l)) (lev ?l2) (resolve-axioms)" << std::endl;
	for (unsigned int i = 0; i < nr_states; ++i)
	{
		o << "\t (when (m s" << i << ") (not (m s" << i << ")))" << std::endl;
		o << "\t (when (stack s" << i << " ?l2) (and (m s" << i << ") (not (stack s" << i << " ?l2))))" << std::endl;
	}
	o << "\t)" << std::endl;
	o << ")" << std::endl;

	/**
	 * Navigate action.
	 */
	o << "(:action navigate" << std::endl;
	o << " :parameters (?x - rover ?y - waypoint ?z - waypoint) " << std::endl;
	o << ";:duration (= ?duration (timeToTraverse ?x ?y ?z))" << std::endl;
	o << " :precondition (and " << std::endl;
	o << " (not (resolve-axioms))" << std::endl;
	o << " (not (Rgoal-reached))" << std::endl;
	for (unsigned int i = 0; i < nr_states; ++i)
	{
		o << " (Rat ?x ?y s" << i << ")" << std::endl;
		o << " (RcanTraverse ?x ?y ?z s" << i << ")" << std::endl;
	}
	o << " )" << std::endl;
	o << " :effect (and " << std::endl;
	for (unsigned int i = 0; i < nr_states; ++i)
	{
		o << " (when (m s" << i << ")" << std::endl;
		o << " (and (not (at ?x ?y s" << i << ")) (at ?x ?z s" << i << ") (not (Rat ?x ?y s" << i << ")) (Rat ?x ?z s" << i << "))" << std::endl;
		o << " )" << std::endl;
	}
	o << " )" << std::endl;
	o << ")" << std::endl;

	/**
	 * Non-branching sense action.
	 *
	if (view_points.size() > 0)
	{
		o << "(:action sense" << std::endl;
		o << "; :duration (= ?duration 4)" << std::endl;
		o << " :parameters (?r - rover ?o - objective ?w - waypoint)" << std::endl;
		o << " :precondition (and " << std::endl;
		for (unsigned int i = 0; i < nr_states; ++i)
		{
			o << " (Rat ?r ?w s" << i << ")" << std::endl;
			o << " (RvisibleFrom ?o ?w s" << i << ")" << std::endl;
		}
		o << " (not (resolve-axioms))" << std::endl;
		o << " (not (Rgoal-reached))" << std::endl;
		o << " )" << std::endl;
		o << " :effect (and " << std::endl;
		
		for (unsigned int i = 0; i < nr_states; ++i)
		{
			o << " (when (m s" << i << ")" << std::endl;
			o << " (and (observed ?o s" << i << ") (Robserved ?o s" << i << "))" << std::endl;
			o << " )" << std::endl;
		}
		o << " )" << std::endl;
		o << ")" << std::endl;
	}*/
	
	/**
	 * Observe view cone action.
	 */
	o << "(:action sense-view-cone" << std::endl;
	o << "; :duration (= ?duration 4)" << std::endl;
	o << " :parameters (?r - rover ?viewcone - waypoint)" << std::endl;
	o << " :precondition (and " << std::endl;
	o << " (isViewCone ?viewcone)" << std::endl;
	for (unsigned int i = 0; i < nr_states; ++i)
	{
		o << " (Rat ?r ?viewcone s" << i << ")" << std::endl;
	}
	o << " (not (resolve-axioms))" << std::endl;
	o << " (not (Rgoal-reached))" << std::endl;
	o << " )" << std::endl;
	o << " :effect (and " << std::endl;
	
	for (unsigned int i = 0; i < nr_states; ++i)
	{
		o << " (when (m s" << i << ")" << std::endl;
		o << " (and (observedViewCone ?viewcone s" << i << ") (RobservedViewCone ?viewcone s" << i << "))" << std::endl;
		o << " )" << std::endl;
	}
	o << " )" << std::endl;
	o << ")" << std::endl;
	
	/**
	 * Observe wall action.
	 */
	o << "(:action observe-wall" << std::endl;
	o << "; :duration (= ?duration 4)" << std::endl;
	o << " :parameters (?r - rover ?o - wall ?w - waypoint ?l - LEVEL ?l2 - LEVEL)" << std::endl;
	o << " :precondition (and " << std::endl;
	if (consider_walls)
	{
		for (unsigned int i = 0; i < nr_states; ++i)
		{
			o << "(Rat ?r ?w s" << i << ")" << std::endl;
			o << "(RcanObserve ?o ?w s" << i << ")" << std::endl;
		}
	}
	o << " (not (resolve-axioms))" << std::endl;
	o << " (not (Rgoal-reached))" << std::endl;
	o << " (next ?l ?l2)" << std::endl;
	o << " (lev ?l)" << std::endl;
	/// TODO
	o << " ; Do not allow the execution if there are no" << std::endl;
	o << " ; states that can be disambiguated by this sensing action." << std::endl;
	o << " (exists (?s - STATE) (and (m ?s) (hasWall ?o ?s)))" << std::endl;
	o << " (exists (?s - STATE) (and (m ?s) (not (hasWall ?o ?s))))" << std::endl;
	o << " )" << std::endl;
	o << " :effect (and " << std::endl;
	o << "  (not (lev ?l)) " << std::endl;
	o << "  (lev ?l2)" << std::endl;
	for (unsigned int i = 0; i < nr_states; ++i)
	{
		o << "  (when (and (m s" << i << ") (not (hasWall ?o s" << i << ")))" << std::endl;
		o << "     (and (stack s" << i << " ?l) (not (m s" << i << ")))" << std::endl;
		o << "  )" << std::endl;
	}
	o << "  (resolve-axioms)" << std::endl;
	o << " )" << std::endl;
	o << ")" << std::endl;
	
	/**
	 * Observe wall action -- not branching.
	 *
	o << "(:action observe-wall-no-branching" << std::endl;
	o << "; :duration (= ?duration 4)" << std::endl;
	o << " :parameters (?r - rover ?o - wall ?w - waypoint)" << std::endl;
	o << " :precondition (and " << std::endl;
	if (consider_walls)
	{
		for (unsigned int i = 0; i < nr_states; ++i)
		{
			o << "(Rat ?r ?w s" << i << ")" << std::endl;
			o << "(RcanObserve ?o ?w s" << i << ")" << std::endl;
			o << "(RhasWall ?o s" << i << ")" << std::endl;
		}
	}
	o << " (not (resolve-axioms))" << std::endl;
	o << " (not (Rgoal-reached))" << std::endl;
	/// TODO
	o << " )" << std::endl;
	o << " :effect (and " << std::endl;
	for (unsigned int i = 0; i < nr_states; ++i)
	{
		o << " (when (m s" << i << ")" << std::endl;
		o << " (and (observedWall ?o s" << i << ") (RobservedWall ?o s" << i << "))" << std::endl;
		o << " )" << std::endl;
	}
	o << " )" << std::endl;
	o << ")" << std::endl;
	*/
	o << ")" << std::endl;
}

#ifdef FLYING_TURTLEBOT_PLANNING_USE_ONTOLOGY
void CPGenerator::getInspectionPoints(std::vector<Waypoint*>& inspection_points, const std::string& aoi_name)
{
	// Call the waypoint server to generate the waypoints.
	turtlebot_common::GenerateWaypoint::Request req;
	req.n_random_waypoints = 200;
#ifdef USE_ALTERNATIVE_VIEW_POINT_GENERATION
	req.n_selected_waypoints = 1;
#else
	req.n_selected_waypoints = 5;
#endif
	req.inflation_radius = 0.25f;
	
	geometry_msgs::Point32 p;
	p.x = aoi_min_x_;
	p.y = aoi_max_y_;
	p.z = 0;
	req.aoi.points.push_back(p);
	
	p.x = aoi_max_x_;
	p.y = aoi_max_y_;
	req.aoi.points.push_back(p);
	
	p.x = aoi_min_x_;
	p.y = aoi_min_y_;
	req.aoi.points.push_back(p);
	
	p.x = aoi_max_x_;
	p.y = aoi_min_y_;
	req.aoi.points.push_back(p);
	req.aoi_name = aoi_name;
	
	turtlebot_common::GenerateWaypoint::Response res;
	if (!get_waypoints_client_.call(req, res))
	{
		std::cout << "Failed to generate waypoints!" << std::endl;
		exit(1);
	}
	
	std::cout << "[CPGenerator::getInspectionPoints] " << aoi_name << std::endl;
	std::vector<std::string> view_poses = oa_->getStringProperty(aoi_name, "plan:'hasViewPose'");
	
	for (std::vector<std::string>::const_iterator ci = view_poses.begin(); ci != view_poses.end(); ++ci)
	{
		// Get the location of these inspection points and store them.
		float x = oa_->getFloatUnique(*ci, "knowrob:'xCoord'");
		float y = oa_->getFloatUnique(*ci, "knowrob:'yCoord'");
		
		float theta = 0;
		std::string orientation = oa_->getStringUnique(*ci, "knowrob:'orientation'");
		float m00 = oa_->getFloatUnique(orientation, "knowrob:'m00'");
		float m10 = oa_->getFloatUnique(orientation, "knowrob:'m10'");
		if (m10 >= 0) // sin(theta) is positive, so 0 <= theta <= pi
		{
			theta = acos(m00);
		}
		else
		{
			theta = 2*M_PI - acos(m00);
		}
		
		// Check if this inspection point is accessible.
		if (!environment_->isAccessible(x, y))
		{
			std::cout << "The inspection point (" << x << ", " << y << ") is not accessibly, ignoring it!" << std::endl;
			continue;
		}
		
		// Transform the ontology id to the name that will be used by the planner, by only retain the bit after '#'.
		std::string predicate = (*ci).substr((*ci).find("#") + 1);
		//inspection_points.push_back(new Waypoint(*ci, predicate, x, y, theta));
		inspection_points.push_back(new Waypoint(*ci, predicate, x, y, theta));
		
		std::cout << "Loaded inspection point (" << x << ", " << y << ": " << theta << ") " << predicate << std::endl;
	}
}
#else

void CPGenerator::getInspectionPoints(std::vector<Waypoint*>& inspection_points, const std::string& aoi_name)
{
	inspection_points.push_back(new Waypoint("blaat", "wp1", 5.0f, 0.0f, 0.0f));
	//inspection_points.push_back(new Waypoint("blaat1", "wp2", -4.0f, 1.0f, 0.0f));
	//inspection_points.push_back(new Waypoint("blaat2", "wp3", 0.0f, -3.0f, 0.0f));
	//inspection_points.push_back(new Waypoint("blaat3", "wp4", 1.0f, 5.0f, 0.0f));
}
#endif
/*
#ifdef FLYING_TURTLEBOT_PLANNING_USE_ONTOLOGY
Waypoint& CPGenerator::generateWaypoint(float x, float y, float theta)
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
Waypoint& CPGenerator::generateWaypoint(float x, float y, float theta)
{
	std::stringstream ss;
	ss << "Waypoint" << waypoint_id_;
	++waypoint_id_;
	
	Waypoint* waypoint = new Waypoint(ss.str(), ss.str(), x, y, theta);
	std::cout << "Created a Pose2D instance with the name: " << ss.str() << "(" << x << ", " << y << "; " << theta << ") and with the rotation matrix: DUMMY" << "!" << std::endl;
	return *waypoint;
}
#endif
*/
int main(int argc, char** argv)
{
	if (argc > 1)
	{
		std::cout << "Command-line options are now ignored (use ROS param 'planner_use_ontology')" << std::endl;
	}
	ros::init(argc, argv, "ContingentPlanGenerator");
	ros::NodeHandle node;
	
//	TurtlebotGroundTruth odometry(node);
//
//	std::vector<Shape*> shapes;
	OccupancyGridFunction ogf(node, "/map");
	bool use_ontol;
	if (!ros::param::get("~planner_use_ontology", use_ontol))
	{
		std::cout << "Defaulting to using ontology" << std::endl;
		use_ontol = true; // default to using the ontology
	}
	CPGenerator cp_generator(node, ogf, "ff", !use_ontol);
	
	ros::spin();
}
