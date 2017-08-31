#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <stdlib.h>
#include <map>
#include <set>

struct Type
{
	Type(const std::string& name)
		: name_(name)
	{
		
	}
	
	std::string name_;
};

struct Object
{
	Object(const std::string& name, const Object* object_below = NULL, const Object* object_on_top = NULL)
		: name_(name), object_below_(object_below), object_on_top_(object_on_top)
	{
		
	}
	
	std::string name_;
	const Object* object_below_;
	const Object* object_on_top_;
};

struct Location
{
	Location(const std::string& name, const std::vector<const Location*>& connected_locations)
		: name_(name), connected_locations_(connected_locations)
	{
		
	}
	
	std::string name_;
	std::vector<const Location*> connected_locations_;
};

struct Box
{
	Box(const std::string& name, const Location& location, const std::vector<const Object*>& objects_that_fit, const std::vector<const Object*>& objects_inside)
		: name_(name), location_(&location), objects_that_fit_(objects_that_fit), objects_inside_(objects_inside)
	{
		
	}
	
	std::string name_;
	const Location* location_;
	std::vector<const Object*> objects_that_fit_; // This is not a static property.
	std::vector<const Object*> objects_inside_;
};

struct State
{
	State(const std::string& state_name, const std::map<const Object*, const Location*>& object_location_mapping, const std::map<const Object*, const Object*>& stackable_mapping, const std::map<const Object*, const Type*>& type_mapping, const std::vector<const Object*>& pushable_objects, const std::vector<const Object*>& pickupable_objects)
		: state_name_(state_name), object_location_mapping_(object_location_mapping), stackable_mapping_(stackable_mapping), type_mapping_(type_mapping), pushable_objects_(pushable_objects), pickupable_objects_(pickupable_objects)
	{
		
	}
	
	std::string state_name_;
	std::map<const Object*, const Location*> object_location_mapping_;
	std::map<const Object*, const Object*> stackable_mapping_;
	std::map<const Object*, const Type*> type_mapping_;
	std::vector<const Object*> pushable_objects_;
	std::vector<const Object*> pickupable_objects_;
};

struct KnowledgeBase
{
	KnowledgeBase(const std::string& name)
		: name_(name)
	{
		
	}
	
	void addChild(const KnowledgeBase& knowledge_base)
	{
		children_.push_back(&knowledge_base);
	}
	
	void addState(State& state)
	{
		states_.push_back(&state);
	}
	
	std::string name_;
	std::vector<const State*> states_;
	
	std::vector<const KnowledgeBase*> children_;
};

void generateProblem(const std::string& file_name, const KnowledgeBase& current_knowledge_base, const std::vector<const KnowledgeBase*>& knowledge_base, const std::vector<const Location*>& locations, const std::vector<const Object*>& objects, const std::vector<const Box*>& boxes, const std::vector<const Type*>& types)
{
	std::vector<const State*> states;
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_base.begin(); ci != knowledge_base.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_.begin(); ci != knowledge_base->states_.end(); ++ci)
		{
			const State* state = *ci;
			states.push_back(state);
		}
	}
	
	std::ofstream myfile;
	myfile.open(file_name.c_str());
	myfile << "(define (problem Keys-0)" << std::endl;
	myfile << "(:domain find_key)" << std::endl;
	myfile << "(:objects" << std::endl;
	
	myfile << "\tl0 - LEVEL" << std::endl;
	myfile << "\tl1 - LEVEL" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	myfile << "(:init" << std::endl;
	myfile << "\t(resolve-axioms)" << std::endl;
	myfile << "\t(lev l0)" << std::endl;
	
	myfile << "\t(next l0 l1)" << std::endl;
	
	myfile << "\t(current_kb " << current_knowledge_base.name_ << ")" << std::endl;
	
	// Location of the robot.
	for (std::vector<const State*>::const_iterator ci = current_knowledge_base.states_.begin(); ci != current_knowledge_base.states_.end(); ++ci)
	{
		const State* state = *ci;
		myfile << "\t(part-of " << state->state_name_ << " " << current_knowledge_base.name_ << ")" << std::endl;
		myfile << "\t(m " << state->state_name_ << ")" << std::endl;
		myfile << "\t(robot_at robot " << locations[0]->name_ << " " << state->state_name_ << ")" << std::endl;
		myfile << "\t(gripper_empty robot " << " " << state->state_name_ << ")" << std::endl;
		
		// All the objects are clear inially.
		for (std::vector<const Object*>::const_iterator ci = objects.begin(); ci != objects.end(); ++ci)
		{
			const Object* object = *ci;
			myfile << "\t(clear " << object->name_ << " " << state->state_name_ << ")" << std::endl;
		}
	}
	
	// Location of the boxes.
	for (std::vector<const Box*>::const_iterator ci = boxes.begin(); ci != boxes.end(); ++ci)
	{
		const Box* box = *ci;
		myfile << "\t(box_at " << box->name_ << " " << box->location_->name_ << ")" << std::endl;
	}
	
	// Location constants.
	for (std::vector<const Location*>::const_iterator ci = locations.begin(); ci != locations.end(); ++ci)
	{
		const Location* location = *ci;
		for (std::vector<const Location*>::const_iterator ci = locations.begin(); ci != locations.end(); ++ci)
		{
			const Location* location2 = *ci;
			if (location == location2) continue;
			myfile << "\t(connected " << location->name_ << " " << location2->name_ << ")" << std::endl;
			myfile << "\t(connected " << location2->name_ << " " << location->name_ << ")" << std::endl;
			myfile << "\t(= (distance " << location->name_ << " " << location2->name_ << ") 10)" << std::endl;
			myfile << "\t(= (distance " << location2->name_ << " " << location->name_ << ") 10)" << std::endl;
		}	
	}
	
	// Locations of the objects.
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_base.begin(); ci != knowledge_base.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_.begin(); ci != knowledge_base->states_.end(); ++ci)
		{
			const State* state = *ci;
			myfile << "\t(part-of " << state->state_name_ << " " << knowledge_base->name_ << ")" << std::endl;
			
			
			for (std::map<const Object*, const Location*>::const_iterator ci = state->object_location_mapping_.begin(); ci != state->object_location_mapping_.end(); ++ci)
			{
				myfile << "\t(object_at " << (*ci).first->name_ << " " << (*ci).second->name_ << " " << state->state_name_ << ")" << std::endl;
			}
			
			for (std::map<const Object*, const Object*>::const_iterator ci = state->stackable_mapping_.begin(); ci != state->stackable_mapping_.end(); ++ci)
			{
				myfile << "\t(on " << (*ci).first->name_ << " " << (*ci).second->name_ << " " << state->state_name_ << ")" << std::endl;
			}
			
			for (std::map<const Object*, const Type*>::const_iterator ci = state->type_mapping_.begin(); ci != state->type_mapping_.end(); ++ci)
			{
				myfile << "\t(is_of_type " << (*ci).first->name_ << " " << (*ci).second->name_ << " " << state->state_name_ << ")" << std::endl;
			}
			
			for (std::vector<const Object*>::const_iterator ci = state->pushable_objects_.begin(); ci != state->pushable_objects_.end(); ++ci)
			{
				myfile << "\t(can_push " << (*ci)->name_ << " " << state->state_name_ << ")" << std::endl;
			}
			
			for (std::vector<const Object*>::const_iterator ci = state->pickupable_objects_.begin(); ci != state->pickupable_objects_.end(); ++ci)
			{
				myfile << "\t(can_pickup " << (*ci)->name_ << " " << state->state_name_ << ")" << std::endl;
			}
		}
		
		for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_base->children_.begin(); ci != knowledge_base->children_.end(); ++ci)
		{
			myfile << "\t(parent " << knowledge_base->name_ << " " << (*ci)->name_ << ")" << std::endl;
		}
	}
	myfile << ")" << std::endl;
	myfile << "(:goal (and" << std::endl;
	for (std::vector<const Object*>::const_iterator ci = objects.begin(); ci != objects.end(); ++ci)
	{
		const Object* object = *ci;
		for (std::vector<const State*>::const_iterator ci = current_knowledge_base.states_.begin(); ci != current_knowledge_base.states_.end(); ++ci)
		{
			myfile << "\t(tidy " << object->name_ << " " << (*ci)->state_name_ << ")" << std::endl;
		}
	}
	myfile << ")" << std::endl;
	myfile << ")" << std::endl;
	myfile << ")" << std::endl;
	myfile.close();
}

void generateDomain(const std::string& file_name, const KnowledgeBase& current_knowledge_base, const std::vector<const KnowledgeBase*>& knowledge_bases, const std::vector<const Location*>& locations, const std::vector<const Object*>& objects, const std::vector<const Box*>& boxes, const std::vector<const Type*>& types)
{
	std::vector<const State*> states;
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_.begin(); ci != knowledge_base->states_.end(); ++ci)
		{
			const State* state = *ci;
			states.push_back(state);
		}
	}
	
	std::ofstream myfile;
	myfile.open (file_name.c_str());
	myfile << "(define (domain find_key)" << std::endl;
	myfile << "(:requirements :typing :conditional-effects :negative-preconditions :disjunctive-preconditions)" << std::endl;
	myfile << std::endl;
	myfile << "(:types" << std::endl;
	myfile << "\twaypoint robot object box type" << std::endl;
	myfile << "\tlevel" << std::endl;
	myfile << "\tstate" << std::endl;
	myfile << "\tknowledgebase" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	myfile << "(:predicates" << std::endl;
	myfile << "\t(robot_at ?r - robot ?wp - waypoint ?s - state)" << std::endl;
	myfile << "\t(Rrobot_at ?r - robot ?wp - waypoint ?s - state)" << std::endl;
	myfile << "\t(object_at ?o - object ?wp - waypoint ?s - state)" << std::endl;
	myfile << "\t(Robject_at ?o - object ?wp - waypoint ?s - state)" << std::endl;
	myfile << "\t(box_at ?b - box ?wp - waypoint ?s - state)" << std::endl;
	myfile << "\t(Rbox_at ?b - box ?wp - waypoint ?s - state)" << std::endl;
	myfile << "\t(gripper_empty ?r - robot ?s - state)" << std::endl;
	myfile << "\t(Rgripper_empty ?r - robot ?s - state)" << std::endl;
	myfile << "\t(holding ?r - robot ?o - object ?s - state)" << std::endl;
	myfile << "\t(Rholding ?r - robot ?o - object ?s - state)" << std::endl;
	myfile << "\t(on ?o1 ?o2 - object ?s - state)" << std::endl;
	myfile << "\t(Ron ?o1 ?o2 - object ?s - state)" << std::endl;
	myfile << "\t(clear ?o - object ?s - state)" << std::endl;
	myfile << "\t(Rclear ?o - object ?s - state)" << std::endl;
	myfile << "\t(tidy ?o - object ?s - state)" << std::endl;
	myfile << "\t(Rtidy ?o - object ?s - state)" << std::endl;
	myfile << "\t(tidy_location ?o - object ?wp - waypoint ?s - state)" << std::endl;
	myfile << "\t(Rtidy_location ?o - object ?wp - waypoint ?s - state)" << std::endl;
	myfile << "\t(push_location ?o - object ?wp - waypoint ?s - state)" << std::endl;
	myfile << "\t(Rpush_location ?o - object ?wp - waypoint ?s - state)" << std::endl;
	myfile << "\t(inside ?object - object ?b - box ?s - state)" << std::endl;
	myfile << "\t(Rinside ?object - object ?b - box ?s - state)" << std::endl;
	myfile << "\t(can_pickup ?r - robot ?t - type ?s - state)" << std::endl;
	myfile << "\t(Rcan_pickup ?r - robot ?t - type ?s - state)" << std::endl;
	myfile << "\t(can_push ?r - robot ?t - type ?s - state)" << std::endl;
	myfile << "\t(Rcan_push ?r - robot ?t - type ?s - state)" << std::endl;
	myfile << "\t(can_fit_inside ?o - object ?b - box ?s - state)" << std::endl;
	myfile << "\t(Rcan_fit_inside ?o - object ?b - box ?s - state)" << std::endl;
	myfile << "\t(can_stack_on ?o1 ?o2 - object ?s - state)" << std::endl;
	myfile << "\t(Rcan_stack_on ?o1 ?o2 - object ?s - state)" << std::endl;
	
	myfile << "\t(connected ?from ?to - waypoint)" << std::endl;
	myfile << "\t(is_of_type ?o - object ?t -type)" << std::endl;
	
	myfile << "\t(part-of ?s - state ?kb - knowledgebase)" << std::endl;
	myfile << "\t(current_kb ?kb - knowledgebase)" << std::endl;
	myfile << "\t(parent ?kb ?kb2 - knowledgebase)" << std::endl;
	
	myfile << std::endl;
	myfile << "\t;; Bookkeeping predicates." << std::endl;
	myfile << "\t(next ?l ?l2 - level)" << std::endl;
	myfile << "\t(lev ?l - LEVEL)" << std::endl;
	myfile << "\t(m ?s - STATE)" << std::endl;
	myfile << "\t(stack ?s - STATE ?l - LEVEL)" << std::endl;
	
	myfile << "\t(resolve-axioms)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	myfile << "(:functions" << std::endl;
	myfile << "\t(distance ?wp1 ?wp2 - waypoint)" << std::endl;
	myfile << ")" << std::endl;
	myfile << "(:constants" << std::endl;
	myfile << "\t; All the balls." << std::endl;
	
	for (std::vector<const Location*>::const_iterator ci = locations.begin(); ci != locations.end(); ++ci)
	{
		myfile << "\t" << (*ci)->name_ << " - waypoint" << std::endl;
	}

	myfile << "\t; The objects." << std::endl;
	for (std::vector<const Object*>::const_iterator ci = objects.begin(); ci != objects.end(); ++ci)
	{
		myfile << "\t" << (*ci)->name_ << " - object" << std::endl;
	}
	
	myfile << "\t; The box." << std::endl;
	for (std::vector<const Box*>::const_iterator ci = boxes.begin(); ci != boxes.end(); ++ci)
	{
		myfile << "\t" << (*ci)->name_ << " - box" << std::endl;
	}
	
	myfile << "\t; The robot." << std::endl;
	myfile << "\trobot - robot" << std::endl;
	
	myfile << "\t; All the states." << std::endl;
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_.begin(); ci != knowledge_base->states_.end(); ++ci)
		{
			myfile << "\t" << (*ci)->state_name_ << " - state" << std::endl;
		}
	}

	myfile << "\t; The knowledge bases" << std::endl;
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		myfile << "\t" << knowledge_base->name_ << " - knowledgebase" << std::endl;
	}

	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * UNSTACK OBJECT.
	 */
	myfile << "(:durative-action unstack_object" << std::endl;
	myfile << "\t:parameters (?r - robot ?wp - waypoint ?o2 ?o1 - object)" << std::endl;
	myfile << "\t:duration ( = ?duration 10)" << std::endl;
	myfile << "\t:condition (and" << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_.begin(); ci != knowledge_base->states_.end(); ++ci)
		{
			myfile << "\t\t(over all (Rrobot_at ?r ?wp " << (*ci)->state_name_ << "))" << std::endl;
			myfile << "\t\t(at start (Robject_at ?o1 ?wp " << (*ci)->state_name_ << "))" << std::endl;
			myfile << "\t\t(at start (Ron ?o2 ?o1 " << (*ci)->state_name_ << "))" << std::endl;
			myfile << "\t\t(at start (Rgripper_empty ?r " << (*ci)->state_name_ << "))" << std::endl;
			myfile << "\t\t(at start (Rcan_pickup ?r ?o2 " << (*ci)->state_name_ << "))" << std::endl;
		}
	}
	
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(when (m " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t(and" << std::endl;
		myfile << "\t\t\t\t(at start (not (gripper_empty ?r " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t\t(at start (not (Rgripper_empty ?r " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t\t(at start (not (on ?o2 ?o1 " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t\t(at start (not (Ron ?o2 ?o1 " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t\t(at end (not (object_at ?o2 ?wp" << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t\t(at end (not (Robject_at ?o2 ?wp" << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t\t(at end (clear ?o1" << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(at end (Rclear ?o1" << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(at end (holding ?v ?o2" << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(at end (Rholding ?v ?o2" << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t)" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * Put object in a box.
	 */
	myfile << "(:durative-action put_object_in_box" << std::endl;
	myfile << "\t:parameters (?r - robot ?wp - waypoint ?o1 - object ?b - box)" << std::endl;
	myfile << "\t:duration ( = ?duration 10)" << std::endl;
	myfile << "\t:condition (and" << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_.begin(); ci != knowledge_base->states_.end(); ++ci)
		{
			myfile << "\t\t(over all (Rrobot_at ?r ?wp " << (*ci)->state_name_ << "))" << std::endl;
			myfile << "\t\t(at start (Rholding ?v ?o1 " << (*ci)->state_name_ << "))" << std::endl;
			myfile << "\t\t(at start (Rcan_fit_inside ?o1 ?b " << (*ci)->state_name_ << "))" << std::endl;
		}
	}
	
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(when (m " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t(and" << std::endl;
		myfile << "\t\t\t\t(at start (not (holding ?r ?o1 " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t\t(at start (not (Rholding ?r ?o1 " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t\t(at end (gripper_empty ?r " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(at end (Rgripper_empty ?r " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(at end (inside ?o1 ?b " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(at end (Rinside ?o1 ?b " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t)" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;

	/**
	 * UNSTACK OBJECT.
	 */
	myfile << "(:durative-action unstack_object" << std::endl;
	myfile << "\t:parameters (?r - robot ?wp - waypoint ?o2 ?o1 - object ?t2 - type)" << std::endl;
	myfile << "\t:duration ( = ?duration 10)" << std::endl;
	myfile << "\t:condition (and" << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_.begin(); ci != knowledge_base->states_.end(); ++ci)
		{
			myfile << "\t\t(over all (Rrobot_at ?r ?wp " << (*ci)->state_name_ << "))" << std::endl;
			myfile << "\t\t(at start (Robject_at ?o1 ?wp " << (*ci)->state_name_ << "))" << std::endl;
			myfile << "\t\t(at start (Ron ?o2 ?o1 " << (*ci)->state_name_ << "))" << std::endl;
			myfile << "\t\t(at start (Rgripper_empty ?r " << (*ci)->state_name_ << "))" << std::endl;
			myfile << "\t\t(at start (Rcan_pickup ?r ?t2 " << (*ci)->state_name_ << "))" << std::endl;
			myfile << "\t\t(over all (is_of_type ?o2 ?t2))" << std::endl;
		}
	}
	
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(when (m " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t(and" << std::endl;
		myfile << "\t\t\t\t(at start (not (gripper_empty ?r " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t\t(at start (not (Rgripper_empty ?r " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t\t(at start (not (on ?o2 ?o1 " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t\t(at start (not (Ron ?o2 ?o1 " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t\t(at end (not (object_at ?o2 ?wp " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t\t(at end (not (Robject_at ?o2 ?wp " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t\t(at end (clear ?o1 " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(at end (Rclear ?o1 " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(at end (holding ?v ?o2 " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(at end (Rholding ?v ?o2 " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t)" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * STACK OBJECT.
	 */
	myfile << "(:durative-action stack_object" << std::endl;
	myfile << "\t:parameters (?r - robot ?wp - waypoint ?o1 ?o2 - object)" << std::endl;
	myfile << "\t:duration ( = ?duration 10)" << std::endl;
	myfile << "\t:condition (and" << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_.begin(); ci != knowledge_base->states_.end(); ++ci)
		{
			myfile << "\t\t(over all (Rrobot_at ?r ?wp " << (*ci)->state_name_ << "))" << std::endl;
			myfile << "\t\t(at start (Robject_at ?o2 ?wp " << (*ci)->state_name_ << "))" << std::endl;
			myfile << "\t\t(at start (Rclear ?o2 " << (*ci)->state_name_ << "))" << std::endl;
			myfile << "\t\t(at start (Rholding ?r ?o1 " << (*ci)->state_name_ << "))" << std::endl;
			myfile << "\t\t(at start (Rcan_stack ?o1 ?o2 " << (*ci)->state_name_ << "))" << std::endl;
		}
	}
	
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(when (m " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t(and" << std::endl;
		myfile << "\t\t\t\t(at start (not (clear ?o2 " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t\t(at start (not (Rclear ?o2 " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t\t(at start (not (holding ?r ?o1 " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t\t(at start (not (Rholding ?r ?o1 " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t\t(at end (gripper_empty ?r " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(at end (Rgripper_empty ?r " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(at end (object_at ?o1 ?wp " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(at end (Robject_at ?o1 ?wp " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(at end (on ?o1 ?o2 " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(at end (Ron ?o1 ?o2 " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t)" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * PICK-UP OBJECT.
	 */
	myfile << "(:durative-action pickup_object" << std::endl;
	myfile << "\t:parameters (?r - robot ?wp - waypoint ?o - object ?t - type)" << std::endl;
	myfile << "\t:duration ( = ?duration 10)" << std::endl;
	myfile << "\t:condition (and" << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_.begin(); ci != knowledge_base->states_.end(); ++ci)
		{
			myfile << "\t\t(over all (Rrobot_at ?r ?wp " << (*ci)->state_name_ << "))" << std::endl;
			myfile << "\t\t(at start (Robject_at ?o ?wp " << (*ci)->state_name_ << "))" << std::endl;
			myfile << "\t\t(at start (Rclear ?o " << (*ci)->state_name_ << "))" << std::endl;
			myfile << "\t\t(at start (Rgripper_empty ?r " << (*ci)->state_name_ << "))" << std::endl;
			myfile << "\t\t(at start (Rcan_pickup ?r ?t " << (*ci)->state_name_ << "))" << std::endl;
			myfile << "\t\t(over all (is_of_type ?o ?t))" << std::endl;
		}
	}
	
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(when (m " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t(and" << std::endl;
		myfile << "\t\t\t\t(at start (not (gripper_empty ?r " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t\t(at start (not (Rgripper_empty ?r " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t\t(at start (not (object_at ?o ?wp " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t\t(at start (not (Robject_at ?o ?wp " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t\t(at end (holding ?r ?o " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(at end (Rholding ?r ?o " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t)" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * PUT-DOWN OBJECT.
	 */
	myfile << "(:durative-action putdown_object" << std::endl;
	myfile << "\t:parameters (?r - robot ?wp - waypoint ?o - object)" << std::endl;
	myfile << "\t:duration ( = ?duration 10)" << std::endl;
	myfile << "\t:condition (and" << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_.begin(); ci != knowledge_base->states_.end(); ++ci)
		{
			myfile << "\t\t(over all (Rrobot_at ?r ?wp " << (*ci)->state_name_ << "))" << std::endl;
			myfile << "\t\t(at start (Rholding ?r ?o " << (*ci)->state_name_ << "))" << std::endl;
		}
	}
	
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(when (m " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t(and" << std::endl;
		myfile << "\t\t\t\t(at start (not (holding ?r ?o " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t\t(at start (not (Rholding ?r ?o " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t\t(at end (gripper_empty ?r " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(at end (Rgripper_empty ?r " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(at end (object_at ?o ?wp " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(at end (Robject_at ?o ?wp " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t)" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * GOTO WAYPOINT.
	 */
	myfile << "(:durative-action goto_waypoint" << std::endl;
	myfile << "\t:parameters (?r - robot ?from ?to - waypoint)" << std::endl;
	myfile << "\t:duration ( = ?duration 10);;(* (distance ?from ?to) 10))" << std::endl;
	myfile << "\t:condition (and" << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_.begin(); ci != knowledge_base->states_.end(); ++ci)
		{
			myfile << "\t\t(over all (connected ?from ?to " << (*ci)->state_name_ << "))" << std::endl;
			myfile << "\t\t(at start (Rrobot_at ?r ?from " << (*ci)->state_name_ << "))" << std::endl;
		}
	}
	
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(when (m " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t(and" << std::endl;
		myfile << "\t\t\t\t(at start (not (robot_at ?r ?from " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t\t(at start (not (Rrobot_at ?r ?from " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t\t(at end (robot_at ?r ?to " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(at end (Rrobot_at ?r ?to " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t)" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * PUSH OBJECT.
	 */
	myfile << "(:durative-action push_object" << std::endl;
	myfile << "\t:parameters (?r - robot ?ob - object ?t - type ?from ?to ?obw - waypoint)" << std::endl;
	myfile << "\t:duration ( = ?duration 80)" << std::endl;
	myfile << "\t:condition (and" << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_.begin(); ci != knowledge_base->states_.end(); ++ci)
		{
			myfile << "\t\t(at start (Rrobot_at ?r ?from " << (*ci)->state_name_ << "))" << std::endl;
			myfile << "\t\t(at start (Robject_at ?ob ?obw " << (*ci)->state_name_ << "))" << std::endl;
			myfile << "\t\t(at start (Rpush_location ?ob ?from " << (*ci)->state_name_ << "))" << std::endl;
			myfile << "\t\t(at start (Rcan_push ?r ?t " << (*ci)->state_name_ << "))" << std::endl;
			myfile << "\t\t(is_of_type ?ob ?t)" << std::endl;
		}
	}
	
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(when (m " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t(and" << std::endl;
		myfile << "\t\t\t\t(at start (not (robot_at ?r ?from " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t\t(at start (not (Rrobot_at ?r ?from " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t\t(at start (not (object_at ?ob ?from " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t\t(at start (not (Robject_at ?ob ?from " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t\t(at end (robot_at ?r ?to " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(at end (Rrobot_at ?r ?to " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(at end (object_at ?ob ?to " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(at end (Robject_at ?ob ?to " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t)" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	
	/**
	 * TIDY OBJECT.
	 */
	myfile << "(:durative-action tidy_object" << std::endl;
	myfile << "\t:parameters (?r - robot ?o - object ?wp - waypoint)" << std::endl;
	myfile << "\t:duration ( = ?duration 5)" << std::endl;
	myfile << "\t:condition (and" << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_.begin(); ci != knowledge_base->states_.end(); ++ci)
		{
			myfile << "\t\t(at start (Rrobot_at ?r ?wp " << (*ci)->state_name_ << "))" << std::endl;
			myfile << "\t\t(at start (Robject_at ?o ?wp " << (*ci)->state_name_ << "))" << std::endl;
			myfile << "\t\t(at start (Rtidy_location ?o ?wp " << (*ci)->state_name_ << "))" << std::endl;
		}
	}
	
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(when (m " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t(and" << std::endl;
		myfile << "\t\t\t\t(at end (tidy ?o " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(at end (Rtidy ?o " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t)" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	
	/**********************
	 * SENSE actions.     *
	 *********************/
	
	/**
	 * Sense the type of an object.
	 */
	myfile << ";; Sense the type of object." << std::endl;
	myfile << "(:action observe-type" << std::endl;

	myfile << "\t:parameters (?t - type ?o - obj ?r - robot ?wp - waypoint ?l ?l2 - level ?kb - knowledgebase)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	myfile << "\t\t(next ?l ?l2)" << std::endl;
	myfile << "\t\t(lev ?l)" << std::endl;
	
	myfile << "\t\t(current_kb ?kb)" << std::endl;
	myfile << std::endl;
	
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_.begin(); ci != knowledge_base->states_.end(); ++ci)
		{
			myfile << "\t\t(Rrobot_at ?r ?wp " << (*ci)->state_name_ << ")" << std::endl;
			myfile << "\t\t(Robject_at ?o ?wp " << (*ci)->state_name_ << ")" << std::endl;
		}
	}

	myfile << "\t\t;; This action is only applicable if there are world states where the outcome can be different." << std::endl;
	
	myfile << "\t\t(exists (?s - state) (and (m ?s) (is_of_type ?o ?t ?s) (part-of ?s ?kb)))" << std::endl;
	myfile << "\t\t(exists (?s - state) (and (m ?s) (not (is_of_type ?o ?t ?s)) (part-of ?s ?kb)))" << std::endl;;
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t(not (lev ?l))" << std::endl;
	myfile << "\t\t(lev ?l2)" << std::endl;
	myfile << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(when (and (m " << (*ci)->state_name_ << ") (not (is_of_type ?o ?t " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t(and (stack " << (*ci)->state_name_ << " ?l) (not (m " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	myfile << "\t\t(resolve-axioms)" << std::endl;
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * Sense whether a type of object can be pushed.
	 */
	myfile << ";; Sense the type of object." << std::endl;
	myfile << "(:action test-push-affordability" << std::endl;

	myfile << "\t:parameters (?t - type ?o - obj ?r - robot ?wp - waypoint ?l ?l2 - level ?kb - knowledgebase)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	myfile << "\t\t(next ?l ?l2)" << std::endl;
	myfile << "\t\t(lev ?l)" << std::endl;
	
	myfile << "\t\t(current_kb ?kb)" << std::endl;
	myfile << std::endl;
	
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_.begin(); ci != knowledge_base->states_.end(); ++ci)
		{
			myfile << "\t\t(Rrobot_at ?r ?wp " << (*ci)->state_name_ << ")" << std::endl;
			myfile << "\t\t(Robject_at ?o ?wp " << (*ci)->state_name_ << ")" << std::endl;
		}
	}

	myfile << "\t\t;; This action is only applicable if there are world states where the outcome can be different." << std::endl;
	
	myfile << "\t\t(exists (?s - state) (and (m ?s) (can_push ?r ?t ?s) (part-of ?s ?kb)))" << std::endl;
	myfile << "\t\t(exists (?s - state) (and (m ?s) (not (can_push ?r ?t ?s)) (part-of ?s ?kb)))" << std::endl;;
	
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t(not (lev ?l))" << std::endl;
	myfile << "\t\t(lev ?l2)" << std::endl;
	myfile << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(when (and (m " << (*ci)->state_name_ << ") (not (can_push ?r ?t " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t(and (stack " << (*ci)->state_name_ << " ?l) (not (m " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	myfile << "\t\t(resolve-axioms)" << std::endl;
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * Sense whether a type of object can be picked up.
	 */
	myfile << ";; Sense the type of object." << std::endl;
	myfile << "(:action test-pickup-affordability" << std::endl;

	myfile << "\t:parameters (?t - type ?o - obj ?r - robot ?wp - waypoint ?l ?l2 - level ?kb - knowledgebase)" << std::endl;

	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	myfile << "\t\t(next ?l ?l2)" << std::endl;
	myfile << "\t\t(lev ?l)" << std::endl;
	
	myfile << "\t\t(current_kb ?kb)" << std::endl;
	myfile << std::endl;
	
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_.begin(); ci != knowledge_base->states_.end(); ++ci)
		{
			myfile << "\t\t(Rrobot_at ?r ?wp " << (*ci)->state_name_ << ")" << std::endl;
			myfile << "\t\t(Robject_at ?o ?wp " << (*ci)->state_name_ << ")" << std::endl;
		}
	}

	myfile << "\t\t;; This action is only applicable if there are world states where the outcome can be different." << std::endl;
	
	myfile << "\t\t(exists (?s - state) (and (m ?s) (can_pickup ?r ?t ?s) (part-of ?s ?kb)))" << std::endl;
	myfile << "\t\t(exists (?s - state) (and (m ?s) (not (can_pickup ?r ?t ?s)) (part-of ?s ?kb)))" << std::endl;;
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t(not (lev ?l))" << std::endl;
	myfile << "\t\t(lev ?l2)" << std::endl;
	myfile << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(when (and (m " << (*ci)->state_name_ << ") (not (can_pickup ?r ?t " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t(and (stack " << (*ci)->state_name_ << " ?l) (not (m " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	myfile << "\t\t(resolve-axioms)" << std::endl;
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * Sense whether an object can be stacked on top of another object.
	 */
	myfile << ";; Sense the type of object." << std::endl;
	myfile << "(:action observe-stackable-affordability" << std::endl;

	myfile << "\t:parameters (?o1 ?o2 - obj ?r - robot ?wp - waypoint ?l ?l2 - level ?kb - knowledgebase)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	myfile << "\t\t(next ?l ?l2)" << std::endl;
	myfile << "\t\t(lev ?l)" << std::endl;
	
	myfile << "\t\t(current_kb ?kb)" << std::endl;
	myfile << std::endl;
	
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_.begin(); ci != knowledge_base->states_.end(); ++ci)
		{
			myfile << "\t\t(Rrobot_at ?r ?wp " << (*ci)->state_name_ << ")" << std::endl;
			myfile << "\t\t(Robject_at ?o1 ?wp " << (*ci)->state_name_ << ")" << std::endl;
			myfile << "\t\t(Robject_at ?o2 ?wp " << (*ci)->state_name_ << ")" << std::endl;
		}
	}

	myfile << "\t\t;; This action is only applicable if there are world states where the outcome can be different." << std::endl;
	
	myfile << "\t\t(exists (?s - state) (and (m ?s) (can_stack_on ?o1 ?o2 ?s) (part-of ?s ?kb)))" << std::endl;
	myfile << "\t\t(exists (?s - state) (and (m ?s) (not (can_stack_on ?o1 ?o2 ?s)) (part-of ?s ?kb)))" << std::endl;;
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t(not (lev ?l))" << std::endl;
	myfile << "\t\t(lev ?l2)" << std::endl;
	myfile << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(when (and (m " << (*ci)->state_name_ << ") (not (can_stack_on ?o1 ?o2 " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t(and (stack " << (*ci)->state_name_ << " ?l) (not (m " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	myfile << "\t\t(resolve-axioms)" << std::endl;
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * Sense whether an object is at a location.
	 */
	myfile << ";; Sense the location of an object." << std::endl;
	myfile << "(:action observe-object-location" << std::endl;

	myfile << "\t:parameters (?o - obj ?r - robot ?wp - waypoint ?l ?l2 - level ?kb - knowledgebase)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	myfile << "\t\t(next ?l ?l2)" << std::endl;
	myfile << "\t\t(lev ?l)" << std::endl;
	myfile << "\t\t(current_kb ?kb)" << std::endl;
	myfile << std::endl;
	
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_.begin(); ci != knowledge_base->states_.end(); ++ci)
		{
			myfile << "\t\t(Rrobot_at ?r ?wp " << (*ci)->state_name_ << ")" << std::endl;
		}
	}

	myfile << "\t\t;; This action is only applicable if there are world states where the outcome can be different." << std::endl;
	
	myfile << "\t\t(exists (?s - state) (and (m ?s) (object_at ?o ?s) (part-of ?s ?kb)))" << std::endl;
	myfile << "\t\t(exists (?s - state) (and (m ?s) (not (object_at ?o ?s)) (part-of ?s ?kb)))" << std::endl;;
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t(not (lev ?l))" << std::endl;
	myfile << "\t\t(lev ?l2)" << std::endl;
	myfile << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(when (and (m " << (*ci)->state_name_ << ") (not (object_at ?o " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t(and (stack " << (*ci)->state_name_ << " ?l) (not (m " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	myfile << "\t\t(resolve-axioms)" << std::endl;
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * POP action.
	 */
	myfile << ";; Exit the current branch." << std::endl;
	myfile << "(:action pop" << std::endl;
	myfile << "\t:parameters (?l ?l2 - level)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(lev ?l)" << std::endl;
	myfile << "\t\t(next ?l2 ?l)" << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and " << std::endl;
	myfile << "\t\t(not (lev ?l))" << std::endl;
	myfile << "\t\t(lev ?l2)" << std::endl;
	myfile << "\t\t(resolve-axioms)" << std::endl;

	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(when (m " << (*ci)->state_name_ << ") " << std::endl;
		myfile << "\t\t\t(not (m " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t)" << std::endl;
		myfile << "\t\t(when (stack " << (*ci)->state_name_ << " ?l2)" << std::endl;
		myfile << "\t\t\t(and " << std::endl;
		myfile << "\t\t\t\t(m " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t\t(not (stack " << (*ci)->state_name_ << " ?l2))" << std::endl;
		myfile << "\t\t\t)" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;

	myfile << ";; Resolve the axioms manually." << std::endl;
	myfile << "(:action raminificate" << std::endl;
	myfile << "\t:parameters ()" << std::endl;
	myfile << "\t:precondition (resolve-axioms)" << std::endl;
	myfile << "\t:effect (and " << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		const State* state = *ci;
		
		myfile << "\t\t(when (or (gripper_empty robot " << state->state_name_ << ") (not (m " << state->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t(Rgripper_empty robot " << state->state_name_ << ")" << std::endl;
		myfile << "\t\t)" << std::endl;
		
		myfile << "\t\t(when (and (not (gripper_empty robot " << state->state_name_ << ")) (m " << state->state_name_ << "))" << std::endl;
		myfile << "\t\t\t(not (Rgripper_empty robot " << state->state_name_ << "))" << std::endl;
		myfile << "\t\t)" << std::endl;
		
		for (std::vector<const Object*>::const_iterator ci = objects.begin(); ci != objects.end(); ++ci)
		{
			const Object* object = *ci;
			myfile << "\t\t(when (or (holding robot " << object->name_ << " " << state->state_name_ << ") (not (m " << state->state_name_ << ")))" << std::endl;
			myfile << "\t\t\t(Rholding robot " << object->name_ << " " << state->state_name_ << ")" << std::endl;
			myfile << "\t\t)" << std::endl;
			
			myfile << "\t\t(when (and (not (holding robot " << object->name_ << " " << state->state_name_ << ")) (m " << state->state_name_ << "))" << std::endl;
			myfile << "\t\t\t(not (Rholding robot " << object->name_ << " " << state->state_name_ << "))" << std::endl;
			myfile << "\t\t)" << std::endl;
			
			myfile << "\t\t(when (or (clear " << object->name_ << " " << state->state_name_ << ") (not (m " << state->state_name_ << ")))" << std::endl;
			myfile << "\t\t\t(Rclear " << object->name_ << " " << state->state_name_ << ")" << std::endl;
			myfile << "\t\t)" << std::endl;
			
			myfile << "\t\t(when (and (not (clear " << object->name_ << " " << state->state_name_ << ")) (m " << state->state_name_ << "))" << std::endl;
			myfile << "\t\t\t(not (Rclear " << object->name_ << " " << state->state_name_ << "))" << std::endl;
			myfile << "\t\t)" << std::endl;
			
			myfile << "\t\t(when (or (tidy " << object->name_ << " " << state->state_name_ << ") (not (m " << state->state_name_ << ")))" << std::endl;
			myfile << "\t\t\t(Rtidy " << object->name_ << " " << state->state_name_ << ")" << std::endl;
			myfile << "\t\t)" << std::endl;
			
			myfile << "\t\t(when (and (not (tidy " << object->name_ << " " << state->state_name_ << ")) (m " << state->state_name_ << "))" << std::endl;
			myfile << "\t\t\t(not (Rtidy " << object->name_ << " " << state->state_name_ << "))" << std::endl;
			myfile << "\t\t)" << std::endl;
			
			for (std::vector<const Object*>::const_iterator ci = objects.begin(); ci != objects.end(); ++ci)
			{
				const Object* other_object = *ci;
				myfile << "\t\t(when (or (on " << object->name_ << " " << other_object->name_ << " " << state->state_name_ << ") (not (m " << state->state_name_ << ")))" << std::endl;
				myfile << "\t\t\t(Ron " << object->name_ << " " << other_object->name_ << " " << state->state_name_ << ")" << std::endl;
				myfile << "\t\t)" << std::endl;
				
				myfile << "\t\t(when (and (not (on " << object->name_ << " " << other_object->name_ << " " << state->state_name_ << ")) (m " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t\t(not (Ron " << object->name_ << " " << other_object->name_ << " " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t)" << std::endl;
				
				myfile << "\t\t(when (or (can_stack_on " << object->name_ << " " << other_object->name_ << " " << state->state_name_ << ") (not (m " << state->state_name_ << ")))" << std::endl;
				myfile << "\t\t\t(Rcan_stack_on " << object->name_ << " " << other_object->name_ << " " << state->state_name_ << ")" << std::endl;
				myfile << "\t\t)" << std::endl;
				
				myfile << "\t\t(when (and (not (can_stack_on " << object->name_ << " " << other_object->name_ << " " << state->state_name_ << ")) (m " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t\t(not (Rcan_stack_on " << object->name_ << " " << other_object->name_ << " " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t)" << std::endl;
			}
			
			for (std::vector<const Location*>::const_iterator ci = locations.begin(); ci != locations.end(); ++ci)
			{
				const Location* location = *ci;
				myfile << "\t\t(when (or (tidy_location " << object->name_ << " " << location->name_ << " " << state->state_name_ << ") (not (m " << state->state_name_ << ")))" << std::endl;
				myfile << "\t\t\t(Rtidy_location " << object->name_ << " " << location->name_ << " " << state->state_name_ << ")" << std::endl;
				myfile << "\t\t)" << std::endl;
				
				myfile << "\t\t(when (and (not (tidy_location " << object->name_ << " " << location->name_ << " " << state->state_name_ << ")) (m " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t\t(not (Rtidy_location " << object->name_ << " " << location->name_ << " " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t)" << std::endl;
				
				myfile << "\t\t(when (or (push_location " << object->name_ << " " << location->name_ << " " << state->state_name_ << ") (not (m " << state->state_name_ << ")))" << std::endl;
				myfile << "\t\t\t(Rpush_location " << object->name_ << " " << location->name_ << " " << state->state_name_ << ")" << std::endl;
				myfile << "\t\t)" << std::endl;
				
				myfile << "\t\t(when (and (not (push_location " << object->name_ << " " << location->name_ << " " << state->state_name_ << ")) (m " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t\t(not (Rpush_location " << object->name_ << " " << location->name_ << " " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t)" << std::endl;
			}
			
			for (std::vector<const Box*>::const_iterator ci = boxes.begin(); ci != boxes.end(); ++ci)
			{
				const Box* box = *ci;
				myfile << "\t\t(when (or (inside " << object->name_ << " " << box->name_ << " " << state->state_name_ << ") (not (m " << state->state_name_ << ")))" << std::endl;
				myfile << "\t\t\t(Rinside " << object->name_ << " " << box->name_ << " " << state->state_name_ << ")" << std::endl;
				myfile << "\t\t)" << std::endl;
				
				myfile << "\t\t(when (and (not (inside " << object->name_ << " " << box->name_ << " " << state->state_name_ << ")) (m " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t\t(not (Rinside " << object->name_ << " " << box->name_ << " " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t)" << std::endl;
				
				myfile << "\t\t(when (or (can_fit_inside " << object->name_ << " " << box->name_ << " " << state->state_name_ << ") (not (m " << state->state_name_ << ")))" << std::endl;
				myfile << "\t\t\t(Rcan_fit_inside " << object->name_ << " " << box->name_ << " " << state->state_name_ << ")" << std::endl;
				myfile << "\t\t)" << std::endl;
				
				myfile << "\t\t(when (and (not (can_fit_inside " << object->name_ << " " << box->name_ << " " << state->state_name_ << ")) (m " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t\t(not (Rcan_fit_inside " << object->name_ << " " << box->name_ << " " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t)" << std::endl;
			}
		}
		
		for (std::vector<const Location*>::const_iterator ci = locations.begin(); ci != locations.end(); ++ci)
		{
			const Location* location = *ci;
			myfile << "\t\t(when (or (robot_at " << location->name_ << " " << state->state_name_ << ") (not (m " << state->state_name_ << ")))" << std::endl;
			myfile << "\t\t\t(Rrobot_at " << location->name_ << " " << state->state_name_ << ")" << std::endl;
			myfile << "\t\t)" << std::endl;
			
			myfile << "\t\t(when (and (not (robot_at " << location->name_ << " " << state->state_name_ << ")) (m " << state->state_name_ << "))" << std::endl;
			myfile << "\t\t\t(not (Rrobot_at " << location->name_ << " " << state->state_name_ << "))" << std::endl;
			myfile << "\t\t)" << std::endl;
			
			myfile << "\t\t(when (or (object_at " << location->name_ << " " << state->state_name_ << ") (not (m " << state->state_name_ << ")))" << std::endl;
			myfile << "\t\t\t(Robject_at " << location->name_ << " " << state->state_name_ << ")" << std::endl;
			myfile << "\t\t)" << std::endl;
			
			myfile << "\t\t(when (and (not (object_at " << location->name_ << " " << state->state_name_ << ")) (m " << state->state_name_ << "))" << std::endl;
			myfile << "\t\t\t(not (Robject_at " << location->name_ << " " << state->state_name_ << "))" << std::endl;
			myfile << "\t\t)" << std::endl;
			
			myfile << "\t\t(when (or (box_at " << location->name_ << " " << state->state_name_ << ") (not (m " << state->state_name_ << ")))" << std::endl;
			myfile << "\t\t\t(Rbox_at " << location->name_ << " " << state->state_name_ << ")" << std::endl;
			myfile << "\t\t)" << std::endl;
			
			myfile << "\t\t(when (and (not (box_at " << location->name_ << " " << state->state_name_ << ")) (m " << state->state_name_ << "))" << std::endl;
			myfile << "\t\t\t(not (Rbox_at " << location->name_ << " " << state->state_name_ << "))" << std::endl;
			myfile << "\t\t)" << std::endl;
		}
		
		for (std::vector<const Type*>::const_iterator ci = types.begin(); ci != types.end(); ++ci)
		{
			const Type* type = *ci;
			myfile << "\t\t(when (or (can_pickup robot " << type->name_ << " " << state->state_name_ << ") (not (m " << state->state_name_ << ")))" << std::endl;
			myfile << "\t\t\t(Rcan_pickup robot " << type->name_ << " " << state->state_name_ << ")" << std::endl;
			myfile << "\t\t)" << std::endl;
			
			myfile << "\t\t(when (and (not (can_pickup robot " << type->name_ << " " << state->state_name_ << ")) (m " << state->state_name_ << "))" << std::endl;
			myfile << "\t\t\t(not (Rcan_pickup robot " << type->name_ << " " << state->state_name_ << "))" << std::endl;
			myfile << "\t\t)" << std::endl;
			
			myfile << "\t\t(when (or (can_push robot " << type->name_ << " " << state->state_name_ << ") (not (m " << state->state_name_ << ")))" << std::endl;
			myfile << "\t\t\t(Rcan_push robot " << type->name_ << " " << state->state_name_ << ")" << std::endl;
			myfile << "\t\t)" << std::endl;
			
			myfile << "\t\t(when (and (not (can_push robot " << type->name_ << " " << state->state_name_ << ")) (m " << state->state_name_ << "))" << std::endl;
			myfile << "\t\t\t(not (Rcan_push robot " << type->name_ << " " << state->state_name_ << "))" << std::endl;
			myfile << "\t\t)" << std::endl;
		}
	}
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	
	myfile << ";; Move 'down' into the knowledge base." << std::endl;
	myfile << "(:action assume_knowledge" << std::endl;
	myfile << "\t:parameters (?old_kb ?new_kb - knowledgebase)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	myfile << "\t\t(current_kb ?old_kb)" << std::endl;
	myfile << "\t\t(parent ?old_kb ?new_kb)" << std::endl;
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t(not (current_kb ?old_kb))" << std::endl;
	myfile << "\t\t(current_kb ?new_kb)" << std::endl;
	myfile << "\t\t(resolve-axioms)" << std::endl;
	myfile << std::endl;
	
	myfile << "\t\t;; Now we need to delete all knowledge from the old_kb and insert it to" << std::endl;
	myfile << "\t\t;; the new_kb level." << std::endl;

	myfile << "\t\t;; For every state ?s, ?s2" << std::endl;
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		const State* state = *ci;
		
		// Get rid of all states that are not part of this knowledge base.
		myfile << "\t\t(when (and (m " << state->state_name_ << ") (not (part-of " << state->state_name_ << " ?new_kb)))" << std::endl;
		myfile << "\t\t\t(not (m " << state->state_name_ << "))" << std::endl;
		myfile << "\t\t)" << std::endl;
		
		// Enable the states that are encapsulated in this knowledge base.
		myfile << "\t\t(when (part-of " << state->state_name_ << " ?new_kb)" << std::endl;
		myfile << "\t\t\t(and (m " << state->state_name_ << "))" << std::endl;
		myfile << "\t\t)" << std::endl;
		
		// Copy all knowledge that is part of ?old_kb to all the new states.
		for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
		{
			const State* state2 = *ci;
			
			myfile << "\t\t(when (and (part-of " << state->state_name_ << " ?old_kb) (gripper_empty robot " << state->state_name_ << ") (part-of " << state2->state_name_ << " ?new_kb))" << std::endl;
			myfile << "\t\t\t(and " << std::endl;
			myfile << "\t\t\t\t(not (Rgripper_empty robot " << state->state_name_ << "))" << std::endl;
			myfile << "\t\t\t\t(not (gripper_empty robot " << state->state_name_ << "))" << std::endl;
			myfile << "\t\t\t\t(Rgripper_empty robot " << state2->state_name_ << ")" << std::endl;
			myfile << "\t\t\t\t(gripper_empty robot " << state2->state_name_ << ")" << std::endl;
			myfile << "\t\t\t)" << std::endl;
			myfile << "\t\t)" << std::endl;
			
			for (std::vector<const Object*>::const_iterator ci = objects.begin(); ci != objects.end(); ++ci)
			{
				const Object* object = *ci;
				myfile << "\t\t(when (and (part-of " << state->state_name_ << " ?old_kb) (holding robot " << object->name_ << " " << state->state_name_ << ") (part-of " << state2->state_name_ << " ?new_kb))" << std::endl;
				myfile << "\t\t\t(and " << std::endl;
				myfile << "\t\t\t\t(not (Rholding robot " << object->name_ << " " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t\t\t(not (holding robot " << object->name_ << " " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t\t\t(Rholding robot " << object->name_ << " " << state2->state_name_ << ")" << std::endl;
				myfile << "\t\t\t\t(holding robot " << object->name_ << " " << state2->state_name_ << ")" << std::endl;
				myfile << "\t\t\t)" << std::endl;
				myfile << "\t\t)" << std::endl;
				
				myfile << "\t\t(when (and (part-of " << state->state_name_ << " ?old_kb) (clear " << object->name_ << " " << state->state_name_ << ") (part-of " << state2->state_name_ << " ?new_kb))" << std::endl;
				myfile << "\t\t\t(and " << std::endl;
				myfile << "\t\t\t\t(not (Rclear " << object->name_ << " " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t\t\t(not (clear " << object->name_ << " " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t\t\t(Rclear " << object->name_ << " " << state2->state_name_ << ")" << std::endl;
				myfile << "\t\t\t\t(clear " << object->name_ << " " << state2->state_name_ << ")" << std::endl;
				myfile << "\t\t\t)" << std::endl;
				myfile << "\t\t)" << std::endl;
				
				myfile << "\t\t(when (and (part-of " << state->state_name_ << " ?old_kb) (tidy " << object->name_ << " " << state->state_name_ << ") (part-of " << state2->state_name_ << " ?new_kb))" << std::endl;
				myfile << "\t\t\t(and " << std::endl;
				myfile << "\t\t\t\t(not (Rtidy " << object->name_ << " " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t\t\t(not (tidy " << object->name_ << " " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t\t\t(Rtidy " << object->name_ << " " << state2->state_name_ << ")" << std::endl;
				myfile << "\t\t\t\t(tidy " << object->name_ << " " << state2->state_name_ << ")" << std::endl;
				myfile << "\t\t\t)" << std::endl;
				myfile << "\t\t)" << std::endl;
				
				for (std::vector<const Object*>::const_iterator ci = objects.begin(); ci != objects.end(); ++ci)
				{
					const Object* other_object = *ci;
					myfile << "\t\t(when (and (part-of " << state->state_name_ << " ?old_kb) (on " << object->name_ << " " << other_object->name_ << " " << state->state_name_ << ") (part-of " << state2->state_name_ << " ?new_kb))" << std::endl;
					myfile << "\t\t\t(and " << std::endl;
					myfile << "\t\t\t\t(not (Ron " << object->name_ << " " << other_object->name_ << " " << state->state_name_ << "))" << std::endl;
					myfile << "\t\t\t\t(not (on " << object->name_ << " " << other_object->name_ << " " << state->state_name_ << "))" << std::endl;
					myfile << "\t\t\t\t(Ron " << object->name_ << " " << other_object->name_ << " " << state2->state_name_ << ")" << std::endl;
					myfile << "\t\t\t\t(on " << object->name_ << " " << other_object->name_ << " " << state2->state_name_ << ")" << std::endl;
					myfile << "\t\t\t)" << std::endl;
					myfile << "\t\t)" << std::endl;
					
					myfile << "\t\t(when (and (part-of " << state->state_name_ << " ?old_kb) (can_stack_on " << object->name_ << " " << other_object->name_ << " " << state->state_name_ << ") (part-of " << state2->state_name_ << " ?new_kb))" << std::endl;
					myfile << "\t\t\t(and " << std::endl;
					myfile << "\t\t\t\t(not (Rcan_stack_on " << object->name_ << " " << other_object->name_ << " " << state->state_name_ << "))" << std::endl;
					myfile << "\t\t\t\t(not (can_stack_on " << object->name_ << " " << other_object->name_ << " " << state->state_name_ << "))" << std::endl;
					myfile << "\t\t\t\t(Rcan_stack_on " << object->name_ << " " << other_object->name_ << " " << state2->state_name_ << ")" << std::endl;
					myfile << "\t\t\t\t(can_stack_on " << object->name_ << " " << other_object->name_ << " " << state2->state_name_ << ")" << std::endl;
					myfile << "\t\t\t)" << std::endl;
					myfile << "\t\t)" << std::endl;
				}
				
				for (std::vector<const Box*>::const_iterator ci = boxes.begin(); ci != boxes.end(); ++ci)
				{
					const Box* box = *ci;
					myfile << "\t\t(when (and (part-of " << state->state_name_ << " ?old_kb) (inside " << object->name_ << " " << box->name_ << " " << state->state_name_ << ") (part-of " << state2->state_name_ << " ?new_kb))" << std::endl;
					myfile << "\t\t\t(and " << std::endl;
					myfile << "\t\t\t\t(not (Rinside " << object->name_ << " " << box->name_ << " " << state->state_name_ << "))" << std::endl;
					myfile << "\t\t\t\t(not (inside " << object->name_ << " " << box->name_ << " " << state->state_name_ << "))" << std::endl;
					myfile << "\t\t\t\t(Rinside " << object->name_ << " " << box->name_ << " " << state2->state_name_ << ")" << std::endl;
					myfile << "\t\t\t\t(inside " << object->name_ << " " << box->name_ << " " << state2->state_name_ << ")" << std::endl;
					myfile << "\t\t\t)" << std::endl;
					myfile << "\t\t)" << std::endl;
					
					myfile << "\t\t(when (and (part-of " << state->state_name_ << " ?old_kb) (can_fit_inside " << object->name_ << " " << box->name_ << " " << state->state_name_ << ") (part-of " << state2->state_name_ << " ?new_kb))" << std::endl;
					myfile << "\t\t\t(and " << std::endl;
					myfile << "\t\t\t\t(not (Rcan_fit_inside " << object->name_ << " " << box->name_ << " " << state->state_name_ << "))" << std::endl;
					myfile << "\t\t\t\t(not (can_fit_inside " << object->name_ << " " << box->name_ << " " << state->state_name_ << "))" << std::endl;
					myfile << "\t\t\t\t(Rcan_fit_inside " << object->name_ << " " << box->name_ << " " << state2->state_name_ << ")" << std::endl;
					myfile << "\t\t\t\t(can_fit_inside " << object->name_ << " " << box->name_ << " " << state2->state_name_ << ")" << std::endl;
					myfile << "\t\t\t)" << std::endl;
					myfile << "\t\t)" << std::endl;
				}
			}
			
			for (std::vector<const Location*>::const_iterator ci = locations.begin(); ci != locations.end(); ++ci)
			{
				const Location* location = *ci;
				
				myfile << "\t\t(when (and (part-of " << state->state_name_ << " ?old_kb) (robot_at robot " << location->name_ << " " << state->state_name_ << ") (part-of " << state2->state_name_ << " ?new_kb))" << std::endl;
				myfile << "\t\t\t(and " << std::endl;
				myfile << "\t\t\t\t(not (Rrobot_at robot " << location->name_ << " " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t\t\t(not (robot_at robot " << location->name_ << " " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t\t\t(Rrobot_at robot " << location->name_ << " " << state2->state_name_ << ")" << std::endl;
				myfile << "\t\t\t\t(robot_at robot " << location->name_ << " " << state2->state_name_ << ")" << std::endl;
				myfile << "\t\t\t)" << std::endl;
				myfile << "\t\t)" << std::endl;
				
				myfile << "\t\t(when (and (part-of " << state->state_name_ << " ?old_kb) (object_at robot " << location->name_ << " " << state->state_name_ << ") (part-of " << state2->state_name_ << " ?new_kb))" << std::endl;
				myfile << "\t\t\t(and " << std::endl;
				myfile << "\t\t\t\t(not (Robject_at robot " << location->name_ << " " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t\t\t(not (object_at robot " << location->name_ << " " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t\t\t(Robject_at robot " << location->name_ << " " << state2->state_name_ << ")" << std::endl;
				myfile << "\t\t\t\t(object_at robot " << location->name_ << " " << state2->state_name_ << ")" << std::endl;
				myfile << "\t\t\t)" << std::endl;
				myfile << "\t\t)" << std::endl;
				
				myfile << "\t\t(when (and (part-of " << state->state_name_ << " ?old_kb) (box_at robot " << location->name_ << " " << state->state_name_ << ") (part-of " << state2->state_name_ << " ?new_kb))" << std::endl;
				myfile << "\t\t\t(and " << std::endl;
				myfile << "\t\t\t\t(not (Rbox_at robot " << location->name_ << " " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t\t\t(not (box_at robot " << location->name_ << " " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t\t\t(Rbox_at robot " << location->name_ << " " << state2->state_name_ << ")" << std::endl;
				myfile << "\t\t\t\t(box_at robot " << location->name_ << " " << state2->state_name_ << ")" << std::endl;
				myfile << "\t\t\t)" << std::endl;
				myfile << "\t\t)" << std::endl;
				
				for (std::vector<const Object*>::const_iterator ci = objects.begin(); ci != objects.end(); ++ci)
				{
					const Object* object = *ci;
					myfile << "\t\t(when (and (part-of " << state->state_name_ << " ?old_kb) (tidy_location " << object->name_ << " " << location->name_ << " " << state->state_name_ << ") (part-of " << state2->state_name_ << " ?new_kb))" << std::endl;
					myfile << "\t\t\t(and " << std::endl;
					myfile << "\t\t\t\t(not (Rtidy_location " << object->name_ << " " << location->name_ << " " << state->state_name_ << "))" << std::endl;
					myfile << "\t\t\t\t(not (tidy_location " << object->name_ << " " << location->name_ << " " << state->state_name_ << "))" << std::endl;
					myfile << "\t\t\t\t(Rtidy_location " << object->name_ << " " << location->name_ << " " << state2->state_name_ << ")" << std::endl;
					myfile << "\t\t\t\t(tidy_location " << object->name_ << " " << location->name_ << " " << state2->state_name_ << ")" << std::endl;
					myfile << "\t\t\t)" << std::endl;
					myfile << "\t\t)" << std::endl;
					
					myfile << "\t\t(when (and (part-of " << state->state_name_ << " ?old_kb) (push_location " << object->name_ << " " << location->name_ << " " << state->state_name_ << ") (part-of " << state2->state_name_ << " ?new_kb))" << std::endl;
					myfile << "\t\t\t(and " << std::endl;
					myfile << "\t\t\t\t(not (Rpush_location " << object->name_ << " " << location->name_ << " " << state->state_name_ << "))" << std::endl;
					myfile << "\t\t\t\t(not (push_location " << object->name_ << " " << location->name_ << " " << state->state_name_ << "))" << std::endl;
					myfile << "\t\t\t\t(Rpush_location " << object->name_ << " " << location->name_ << " " << state2->state_name_ << ")" << std::endl;
					myfile << "\t\t\t\t(push_location " << object->name_ << " " << location->name_ << " " << state2->state_name_ << ")" << std::endl;
					myfile << "\t\t\t)" << std::endl;
					myfile << "\t\t)" << std::endl;
				}
			}
			
			for (std::vector<const Type*>::const_iterator ci = types.begin(); ci != types.end(); ++ci)
			{
				const Type* type = *ci;
				myfile << "\t\t(when (and (part-of " << state->state_name_ << " ?old_kb) (can_pickup robot " << type->name_ << " " << state->state_name_ << ") (part-of " << state2->state_name_ << " ?new_kb))" << std::endl;
				myfile << "\t\t\t(and " << std::endl;
				myfile << "\t\t\t\t(not (Rcan_pickup robot " << type->name_ << " " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t\t\t(not (can_pickup robot " << type->name_ << " " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t\t\t(Rcan_pickup robot " << type->name_ << " " << state2->state_name_ << ")" << std::endl;
				myfile << "\t\t\t\t(can_pickup robot " << type->name_ << " " << state2->state_name_ << ")" << std::endl;
				myfile << "\t\t\t)" << std::endl;
				myfile << "\t\t)" << std::endl;
				
				myfile << "\t\t(when (and (part-of " << state->state_name_ << " ?old_kb) (can_push robot " << type->name_ << " " << state->state_name_ << ") (part-of " << state2->state_name_ << " ?new_kb))" << std::endl;
				myfile << "\t\t\t(and " << std::endl;
				myfile << "\t\t\t\t(not (Rcan_push robot " << type->name_ << " " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t\t\t(not (can_push robot " << type->name_ << " " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t\t\t(Rcan_push robot " << type->name_ << " " << state2->state_name_ << ")" << std::endl;
				myfile << "\t\t\t\t(can_push robot " << type->name_ << " " << state2->state_name_ << ")" << std::endl;
				myfile << "\t\t\t)" << std::endl;
				myfile << "\t\t)" << std::endl;
			}
		}
	}
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;

	myfile << ";; Move 'up' into the knowledge base." << std::endl;;
	myfile << "(:action shed_knowledge" << std::endl;
	myfile << "\t:parameters (?old_kb ?new_kb - knowledgebase)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	myfile << "\t\t(current_kb ?old_kb)" << std::endl;
	myfile << "\t\t(parent ?new_kb ?old_kb)" << std::endl;
	
	// We can only move back up the knowledge base if there are not states that belong to this knowledge base.
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(or " << std::endl;
		myfile << "\t\t\t(not (part-of " << (*ci)->state_name_ << " ?old_kb))" << std::endl;
		myfile << "\t\t\t(not (exists (?l - level ) (stack " << (*ci)->state_name_ << " ?l)))" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	
	// Make sure the robot is in the same location.
	myfile << "\t\t(or";
	for (std::vector<const Location*>::const_iterator ci = locations.begin(); ci != locations.end(); ++ci)
	{
		const Location* location = *ci;
		myfile << "\t\t\t(and";
		for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
		{
			const State* state = *ci;
			// Make sure the state of the toilets are the same.
			myfile << "\t\t\t\t(or " << std::endl;
			myfile << "\t\t\t\t\t(not (part-of " << (*ci)->state_name_ << " ?old_kb))" << std::endl;
			myfile << "\t\t\t\t\t(robot_at " << location->name_ << " " << state->state_name_ << ")" << std::endl;
			myfile << "\t\t\t\t)" << std::endl;
		}
		myfile << "\t\t\t)" << std::endl;
	}
	myfile << "\t\t)" << std::endl;
	
	// Make sure the location of the objects is identical.
	for (std::vector<const Object*>::const_iterator ci = objects.begin(); ci != objects.end(); ++ci)
	{
		const Object* object = *ci;
		myfile << "\t\t(or";
		for (std::vector<const Location*>::const_iterator ci = locations.begin(); ci != locations.end(); ++ci)
		{
			const Location* location = *ci;
			myfile << "\t\t\t(and";
			for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
			{
				const State* state = *ci;
				// Make sure the state of the toilets are the same.
				myfile << "\t\t\t\t(or " << std::endl;
				myfile << "\t\t\t\t\t(not (part-of " << (*ci)->state_name_ << " ?old_kb))" << std::endl;
				myfile << "\t\t\t\t\t(object_at " << object->name_ << " " <<  location->name_ << " " << state->state_name_ << ")" << std::endl;
				myfile << "\t\t\t\t)" << std::endl;
			}
			myfile << "\t\t\t)" << std::endl;
		}
		
		myfile << "\t\t\t(and";
		for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
		{
			const State* state = *ci;
			// Make sure the state of the toilets are the same.
			myfile << "\t\t\t\t(or " << std::endl;
			myfile << "\t\t\t\t\t(not (part-of " << (*ci)->state_name_ << " ?old_kb))" << std::endl;
			myfile << "\t\t\t\t\t(holding robot " << object->name_ << " " << state->state_name_ << ")" << std::endl;
			myfile << "\t\t\t\t)" << std::endl;
		}
		myfile << "\t\t\t)" << std::endl;
	}
	myfile << "\t\t)" << std::endl;
	
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t(not (current_kb ?old_kb))" << std::endl;
	myfile << "\t\t(current_kb ?new_kb)" << std::endl;
	myfile << "\t\t(resolve-axioms)" << std::endl;
	myfile << std::endl;
	
	myfile << "\t\t;; Now we need to push all knowledge that is true for all states part of " << std::endl;
	myfile << "\t\t;; kb_old up to kb_new." << std::endl;
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		const State* state = *ci;
		
		// Make the states that were held in suspection active again.
		myfile << "\t\t(when (part-of " << state->state_name_ << " ?new_kb)" << std::endl;
		myfile << "\t\t\t(and (m " << state->state_name_ << "))" << std::endl;
		myfile << "\t\t)" << std::endl;
		
		// And those that were active, inactive.
		myfile << "\t\t(when (not (part-of " << state->state_name_ << " ?new_kb))" << std::endl;
		myfile << "\t\t\t(and (not (m " << state->state_name_ << ")))" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		const State* state = *ci;
		myfile << "\t\t(when (and " << std::endl;
		myfile << "\t\t\t\t;; For every state ?s, ?s2" << std::endl;
	
		for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
		{
			const State* state2 = *ci;
			myfile << "\t\t\t\t\t(or " << std::endl;
			myfile << "\t\t\t\t\t\t(not (part-of " << state2->state_name_ << " ?old_kb))" << std::endl;
			myfile << "\t\t\t\t\t\t(gripper_empty robot " << state2->state_name_ << ")" << std::endl;
			myfile << "\t\t\t\t\t)" << std::endl;
			
		}
		myfile << "\t\t\t\t\t(part-of " << state->state_name_ << " ?new_kb)" << std::endl;
		myfile << "\t\t\t)" << std::endl;

		myfile << "\t\t\t;; Conditional effects" << std::endl;
		myfile << "\t\t\t(and " << std::endl;
		for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
		{
			const State* state2 = *ci;
			myfile << "\t\t\t\t(not (gripper_empty robot " << state2->state_name_ << "))" << std::endl;
		}
		myfile << "\t\t\t\t(gripper_empty robot " << state->state_name_ << ")" << std::endl;
		myfile << "\t\t\t)" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	
	for (std::vector<const Object*>::const_iterator ci = objects.begin(); ci != objects.end(); ++ci)
	{
		const Object* object = *ci;
		for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
		{
			// Deal with the location of the robot.
			const State* state = *ci;
			myfile << "\t\t(when (and " << std::endl;
			myfile << "\t\t\t\t;; For every state ?s, ?s2" << std::endl;
		
			// Holding.
			for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
			{
				const State* state2 = *ci;
				myfile << "\t\t\t\t\t(or " << std::endl;
				myfile << "\t\t\t\t\t\t(not (part-of " << state2->state_name_ << " ?old_kb))" << std::endl;
				myfile << "\t\t\t\t\t\t(holding robot " << object->name_ << " " << state2->state_name_ << ")" << std::endl;
				myfile << "\t\t\t\t\t)" << std::endl;
				
			}
			myfile << "\t\t\t\t\t(part-of " << state->state_name_ << " ?new_kb)" << std::endl;
			myfile << "\t\t\t)" << std::endl;

			myfile << "\t\t\t;; Conditional effects" << std::endl;
			myfile << "\t\t\t(and " << std::endl;
			for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
			{
				const State* state2 = *ci;
				myfile << "\t\t\t\t(not (holding robot " << object->name_ << " " << state2->state_name_ << "))" << std::endl;
			}
			myfile << "\t\t\t\t(holding robot " << object->name_ << " " << state->state_name_ << ")" << std::endl;
			myfile << "\t\t\t)" << std::endl;
			myfile << "\t\t)" << std::endl;
			
			// Tidy.
			myfile << "\t\t(when (and " << std::endl;
			myfile << "\t\t\t\t;; For every state ?s, ?s2" << std::endl;
		
			for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
			{
				const State* state2 = *ci;
				myfile << "\t\t\t\t\t(or " << std::endl;
				myfile << "\t\t\t\t\t\t(not (part-of " << state2->state_name_ << " ?old_kb))" << std::endl;
				myfile << "\t\t\t\t\t\t(tidy " << object->name_ << " " << state2->state_name_ << ")" << std::endl;
				myfile << "\t\t\t\t\t)" << std::endl;
				
			}
			myfile << "\t\t\t\t\t(part-of " << state->state_name_ << " ?new_kb)" << std::endl;
			myfile << "\t\t\t)" << std::endl;

			myfile << "\t\t\t;; Conditional effects" << std::endl;
			myfile << "\t\t\t(and " << std::endl;
			for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
			{
				const State* state2 = *ci;
				myfile << "\t\t\t\t(not (tidy " << object->name_ << " " << state2->state_name_ << "))" << std::endl;
			}
			myfile << "\t\t\t\t(tidy " << object->name_ << " " << state->state_name_ << ")" << std::endl;
			myfile << "\t\t\t)" << std::endl;
			myfile << "\t\t)" << std::endl;
		}
		
		// Objects in boxes.
		for (std::vector<const Box*>::const_iterator ci = boxes.begin(); ci != boxes.end(); ++ci)
		{
			const Box* box = *ci;
			for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
			{
				// Deal with the location of the robot.
				const State* state = *ci;
				myfile << "\t\t(when (and " << std::endl;
				myfile << "\t\t\t\t;; For every state ?s, ?s2" << std::endl;
			
				// Holding.
				for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
				{
					const State* state2 = *ci;
					myfile << "\t\t\t\t\t(or " << std::endl;
					myfile << "\t\t\t\t\t\t(not (part-of " << state2->state_name_ << " ?old_kb))" << std::endl;
					myfile << "\t\t\t\t\t\t(inside " << object->name_ << " " << box->name_ << " " << state2->state_name_ << ")" << std::endl;
					myfile << "\t\t\t\t\t)" << std::endl;
					
				}
				myfile << "\t\t\t\t\t(part-of " << state->state_name_ << " ?new_kb)" << std::endl;
				myfile << "\t\t\t)" << std::endl;

				myfile << "\t\t\t;; Conditional effects" << std::endl;
				myfile << "\t\t\t(and " << std::endl;
				for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
				{
					const State* state2 = *ci;
					myfile << "\t\t\t\t(not (inside " << object->name_ << " " << box->name_ << " " << state2->state_name_ << "))" << std::endl;
				}
				myfile << "\t\t\t\t(inside " << object->name_ << " " << box->name_ << " " << state->state_name_ << ")" << std::endl;
				myfile << "\t\t\t)" << std::endl;
				myfile << "\t\t)" << std::endl;
			}
		}
	}
	
	// Location of the agent.
	for (std::vector<const Location*>::const_iterator ci = locations.begin(); ci != locations.end(); ++ci)
	{
		const Location* location = *ci;
		for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
		{
			// Deal with the location of the robot.
			const State* state = *ci;
			myfile << "\t\t(when (and " << std::endl;
			myfile << "\t\t\t\t;; For every state ?s, ?s2" << std::endl;
		
			for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
			{
				const State* state2 = *ci;
				myfile << "\t\t\t\t(and " << std::endl;
				myfile << "\t\t\t\t\t(or " << std::endl;
				myfile << "\t\t\t\t\t\t(not (part-of " << state2->state_name_ << " ?old_kb))" << std::endl;
				myfile << "\t\t\t\t\t\t(robot_at " << location->name_ << " " << state2->state_name_ << ")" << std::endl;
				myfile << "\t\t\t\t\t)" << std::endl;
				myfile << "\t\t\t\t)" << std::endl;
			}
			myfile << "\t\t\t\t\t(part-of " << state->state_name_ << " ?new_kb)" << std::endl;
			myfile << "\t\t\t)" << std::endl;

			myfile << "\t\t\t;; Conditional effects" << std::endl;
			myfile << "\t\t\t(and " << std::endl;
			
			for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
			{
				const State* state2 = *ci;
				myfile << "\t\t\t\t(not (robot_at " << location->name_ << " " << state2->state_name_ << "))" << std::endl;
			}
			myfile << "\t\t\t\t(robot_at " << location->name_ << " " << state->state_name_ << ")" << std::endl;
			
			myfile << "\t\t\t)" << std::endl;
			myfile << "\t\t)" << std::endl;
			
			// The locations of the objects.
			for (std::vector<const Object*>::const_iterator ci = objects.begin(); ci != objects.end(); ++ci)
			{
				const Object* object = *ci;
				
				// Deal with the location of the robot.
				myfile << "\t\t(when (and " << std::endl;
				myfile << "\t\t\t\t;; For every state ?s, ?s2" << std::endl;
			
				for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
				{
					const State* state2 = *ci;
					myfile << "\t\t\t\t(and " << std::endl;
					myfile << "\t\t\t\t\t(or " << std::endl;
					myfile << "\t\t\t\t\t\t(not (part-of " << state2->state_name_ << " ?old_kb))" << std::endl;
					myfile << "\t\t\t\t\t\t(object_at " << object->name_ << " " << location->name_ << " " << state2->state_name_ << ")" << std::endl;
					myfile << "\t\t\t\t\t)" << std::endl;
					myfile << "\t\t\t\t)" << std::endl;
				}
				myfile << "\t\t\t\t\t(part-of " << state->state_name_ << " ?new_kb)" << std::endl;
				myfile << "\t\t\t)" << std::endl;

				myfile << "\t\t\t;; Conditional effects" << std::endl;
				myfile << "\t\t\t(and " << std::endl;
				
				for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
				{
					const State* state2 = *ci;
					myfile << "\t\t\t\t(not (object_at " << object->name_ << " " << location->name_ << " " << state2->state_name_ << "))" << std::endl;
				}
				myfile << "\t\t\t\t(object_at " << object->name_ << " " << location->name_ << " " << state->state_name_ << ")" << std::endl;
				
				myfile << "\t\t\t)" << std::endl;
				myfile << "\t\t)" << std::endl;
			}
		}
		
		myfile << "\t)" << std::endl;
		myfile << ")" << std::endl;
	}
	myfile << ")" << std::endl;
	myfile.close();
}

int main(int argc, char **argv)
{
	std::vector<const Location*> locations;
	std::vector<const Box*> boxes;
	std::vector<const Object*> objects;
	std::vector<const Type*> types;
	
	if (argc < 3)
	{
		std::cout << "Usage: <number of locations> <number of objects> <number of types>" << std::endl;
		return -1;
	}
	
	unsigned int nr_locations = ::atoi(argv[1]);
	unsigned int nr_objects = ::atoi(argv[2]);
	unsigned int nr_types = ::atoi(argv[3]);
	
	
	std::stringstream ss;
	
	std::vector<const Location*> connected_locations;
	for (unsigned int i = 0; i < nr_locations; ++i)
	{
		ss.str(std::string());
		ss << "loc_" << i;
		Location* l = new Location(ss.str(), connected_locations);
		locations.push_back(l);
	}
	
	for (unsigned int i = 0; i < nr_objects; ++i)
	{
		ss.str(std::string());
		ss << "object_" << i;
		Object* b = new Object(ss.str());
		objects.push_back(b);
	}
	
	for (unsigned int i = 0; i < nr_types; ++i)
	{
		ss.str(std::string());
		ss << "type_" << i;
		Type* c = new Type(ss.str());
		types.push_back(c);
	}
	
	std::cout << "Create an instance with " << nr_locations << " locations, " << nr_objects << " objects, and " << nr_types << " types." << std::endl;
	
	std::cout << "Creating all possible states..." << std::endl;

	std::vector<const KnowledgeBase*> knowledge_bases;
	std::map<const Object*, const Location*> empty_object_location_mapping;
	std::map<const Object*, const Object*> empty_stacked_objects_mapping;
	std::map<const Object*, const Type*> empty_type_mapping;
	std::vector<const Object*> empty_pushable_objects;
	std::vector<const Object*> empty_pickupable_objects;
	State basic_state("basic", empty_object_location_mapping, empty_stacked_objects_mapping, empty_type_mapping, empty_pushable_objects, empty_pickupable_objects);
	
	KnowledgeBase basis_kb("basis_kb");
	basis_kb.addState(basic_state);
	knowledge_bases.push_back(&basis_kb);
	
	// Create a new knowledge base for each object.
	for (std::vector<const Object*>::const_iterator ci = objects.begin(); ci != objects.end(); ++ci)
	{
		const Object* object = *ci;
		ss.str(std::string());
		ss << "kb_location_" << object->name_;
		
		KnowledgeBase* kb_location = new KnowledgeBase(ss.str());
		basis_kb.addChild(*kb_location);
		knowledge_bases.push_back(kb_location);
		
		std::map<const Object*, const Location*> object_location_mapping;
		std::map<const Object*, const Object*> stackable_mapping;
		std::map<const Object*, const Type*> type_mapping;
		std::vector<const Object*> pushable_objects;
		std::vector<const Object*> pickupable_objects;
		
		for (std::vector<const Location*>::const_iterator ci = locations.begin(); ci != locations.end(); ++ci)
		{
			const Location* location = *ci;
			for (std::vector<const Type*>::const_iterator ci = types.begin(); ci != types.end(); ++ci)
			{
				const Type* type = *ci;
				object_location_mapping[object] = location;
				type_mapping[object] = type;
				ss.str(std::string());
				ss << "skb_location_" << object->name_ << "_" << location->name_ << "_" << type->name_ << "_pickupable";
				pickupable_objects.push_back(object);
				State* state_pickupable = new State(ss.str(), object_location_mapping, stackable_mapping, type_mapping, pushable_objects, pickupable_objects);
				kb_location->addState(*state_pickupable);
				pickupable_objects.clear();
				
				ss.str(std::string());
				ss << "skb_location_" << object->name_ << "_" << location->name_ << "_" << type->name_ << "_pushable";
				pickupable_objects.clear();
				pushable_objects.push_back(object);
				State* state_pushable = new State(ss.str(), object_location_mapping, stackable_mapping, type_mapping, pushable_objects, pickupable_objects);
				kb_location->addState(*state_pushable);
				pushable_objects.clear();
			}
		}
	}
	
	std::cout << "Generate domain..." << std::endl;
	generateDomain("test_domain.pddl", basis_kb, knowledge_bases, locations, objects, boxes, types);
	std::cout << "Generate problem..." << std::endl;
	generateProblem("test_problem.pddl", basis_kb, knowledge_bases, locations, objects, boxes, types);
	
	return 0;
}
