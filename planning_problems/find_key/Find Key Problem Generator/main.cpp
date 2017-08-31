#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <stdlib.h>

struct Waypoint
{
	Waypoint(const std::string& wp_name)
		: name_(wp_name)
	{
		
	}
	
	std::string name_;
};

struct Key
{
	Key(const std::string& key_name)
		: name_(key_name)
	{
		
	}
	
	std::string name_;
};

struct State
{
	State(const std::string& state_name, const Key* key, const Waypoint* waypoint)
		: state_name_(state_name), key_(key), waypoint_(waypoint)
	{
		
	}
	
	std::string state_name_;
	const Key* key_;
	const Waypoint* waypoint_;
};

struct KnowledgeBase
{
	KnowledgeBase(const std::string& name, const std::vector<const State*>& states)
		: name_(name), states_(&states)
	{
		
	}
	
	void addChild(const KnowledgeBase& knowledge_base)
	{
		children_.push_back(&knowledge_base);
	}
	
	std::string name_;
	const std::vector<const State*>* states_;
	
	std::vector<const KnowledgeBase*> children_;
};

void generateProblem(const std::string& file_name, const std::vector<const Waypoint*>& waypoints, const std::vector<const Key*>& keys, const std::vector<const KnowledgeBase*>& knowledge_base, const Waypoint& robot_location, const KnowledgeBase& current_knowledge_base, bool factorise, unsigned int nr_locations)
{
	std::vector<const State*> states;
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_base.begin(); ci != knowledge_base.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_->begin(); ci != knowledge_base->states_->end(); ++ci)
		{
			const State* state = *ci;
			states.push_back(state);
		}
	}
	
	std::cout << "Create the problem file: " << file_name << std::endl;
	std::cout << " *** WAYPOINT *** " << std::endl;
	for (std::vector<const Waypoint*>::const_iterator ci = waypoints.begin(); ci != waypoints.end(); ++ci)
	{
		const Waypoint* waypoint = *ci;
		std::cout << "* " << waypoint->name_ << std::endl;
	}
	
	std::cout << " *** KEYS *** " << std::endl;
	for (std::vector<const Key*>::const_iterator ci = keys.begin(); ci != keys.end(); ++ci)
	{
		const Key* key = *ci;
		std::cout << "* " << key->name_ << std::endl;
	}
	
	std::cout << " *** Knowledge Bases *** " << std::endl;
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_base.begin(); ci != knowledge_base.end(); ++ci)
	{
		const KnowledgeBase* kb = *ci;
		std::cout << kb->name_ << " -> ";
		
		for (std::vector<const State*>::const_iterator ci = kb->states_->begin(); ci != kb->states_->end(); ++ci)
		{
			std::cout << (*ci)->state_name_ << ", ";
		}
		std::cout << std::endl;
	}
	
	
	std::ofstream myfile;
	myfile.open(file_name.c_str());
	myfile << "(define (problem Keys-0)" << std::endl;
	myfile << "(:domain find_key)" << std::endl;
	myfile << "(:objects" << std::endl;
	
	if (factorise)
	{
		myfile << "\tl0 - LEVEL" << std::endl;
		myfile << "\tl1 - LEVEL" << std::endl;
	}
	else
	{
		for (unsigned int key_nr = 0; key_nr < keys.size(); ++key_nr)
		{
			myfile << "\tl" << key_nr << " - LEVEL" << std::endl;
		}
	}
	myfile << ")" << std::endl;
	myfile << std::endl;

	myfile << "(:init" << std::endl;
	myfile << "\t(resolve-axioms)" << std::endl;
	myfile << "\t(lev l0)" << std::endl;
	
	if (factorise)
	{
		myfile << "\t(next l0 l1)" << std::endl;
	}
	else if (keys.size() > 1)
	{
		for (unsigned int key_nr = 1; key_nr < keys.size(); ++key_nr)
		{
			myfile << "\t(next l" << (key_nr - 1) << " l" << key_nr << ")" << std::endl;
		}
	}
	
	
	if (factorise)
	{
		myfile << "\t(current_kb " << current_knowledge_base.name_ << ")" << std::endl;
	}
	if (factorise)
	{
		for (std::vector<const State*>::const_iterator ci = current_knowledge_base.states_->begin(); ci != current_knowledge_base.states_->end(); ++ci)
		{
			myfile << "\t(part-of " << (*ci)->state_name_ << " " << current_knowledge_base.name_ << ")" << std::endl;
			myfile << "\t(m " << (*ci)->state_name_ << ")" << std::endl;
			myfile << "\t(at robot " << robot_location.name_ << " " << (*ci)->state_name_ << ")" << std::endl;
		}
	}
	else
	{
		for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_base.begin(); ci != knowledge_base.end(); ++ci)
		{
			const KnowledgeBase* kb = *ci;
			for (std::vector<const State*>::const_iterator ci = kb->states_->begin(); ci != kb->states_->end(); ++ci)
			{
				const State* state = *ci;
				myfile << "\t(m " << state->state_name_ << ")" << std::endl;
				myfile << "\t(at " << state->key_->name_ << " " << state->waypoint_->name_ << " " << state->state_name_ << ")" << std::endl;
				myfile << "\t(at robot " << robot_location.name_ << " " << state->state_name_ << ")" << std::endl;
			}
		}
	}
	
	if (factorise)
	{
		for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_base.begin(); ci != knowledge_base.end(); ++ci)
		{
			const KnowledgeBase* knowledge_base = *ci;
			
			for (std::vector<const State*>::const_iterator ci = knowledge_base->states_->begin(); ci != knowledge_base->states_->end(); ++ci)
			{
				const State* state = *ci;
				
				myfile << "\t(part-of " << state->state_name_ << " " << knowledge_base->name_ << ")" << std::endl;
			
				if (state->key_ != NULL)
				{
					myfile << "\t(at " << state->key_->name_ << " " << state->waypoint_->name_ << " " << state->state_name_ << ")" << std::endl;
				}
			}
			
			for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_base->children_.begin(); ci != knowledge_base->children_.end(); ++ci)
			{
				myfile << "\t(parent " << knowledge_base->name_ << " " << (*ci)->name_ << ")" << std::endl;
			}
		}
	}
	else
	{
		std::vector<unsigned int> key_indexes(keys.size(), 0);
		unsigned int current_state_id = 0;
		
		for (unsigned int i = 0; i < keys.size() * nr_locations; ++i)
		{
			unsigned int key_index = 0;
			for (std::vector<unsigned int>::const_iterator ci = key_indexes.begin(); ci != key_indexes.end(); ++ci, ++key_index)
			{
				myfile << "\t(at " << keys[key_index]->name_ << " wp" << *ci << "_for_" << keys[key_index]->name_ << " s" << current_state_id << ")" << std::endl;
			}
			
			// Update the indices.
			for (unsigned int j = 0; j < keys.size() * nr_locations; ++j)
			{
				if (key_indexes[j] + 1 == nr_locations)
				{
					key_indexes[j] = 0;
					continue;
				}
				key_indexes[j] = key_indexes[j] + 1;
				break;
			}
			
			++current_state_id;
		}
	}
	myfile << ")" << std::endl;
	myfile << "(:goal (and" << std::endl;
	if (factorise)
	{
		for (std::vector<const Key*>::const_iterator ci = keys.begin(); ci != keys.end(); ++ci)
		{
			const Key* key = *ci;
			for (std::vector<const State*>::const_iterator ci = current_knowledge_base.states_->begin(); ci != current_knowledge_base.states_->end(); ++ci)
			{
				myfile << "\t(has " << key->name_ << " " << (*ci)->state_name_ << ")" << std::endl;
			}
		}
	}
	else
	{
		unsigned int current_state_id = 0;
		for (unsigned int i = 0; i < keys.size() * nr_locations; ++i)
		{
			for (std::vector<const Key*>::const_iterator ci = keys.begin(); ci != keys.end(); ++ci)
			{
				myfile << "\t(has " << (*ci)->name_ << " s" << i << ")" << std::endl;
			}
		}
	}
	myfile << ")" << std::endl;
	myfile << ")" << std::endl;
	myfile << ")" << std::endl;
	myfile.close();
}

void generateDomain(const std::string& file_name, const std::vector<const Waypoint*>& waypoints, const std::vector<const Key*>& keys, const std::vector<const KnowledgeBase*>& knowledge_bases, const Waypoint& robot_location, bool factorise)
{
	std::vector<const State*> states;
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_->begin(); ci != knowledge_base->states_->end(); ++ci)
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
	myfile << "\twaypoint" << std::endl;
	myfile << "\trobot key - locatable" << std::endl;
	myfile << "\tlevel" << std::endl;
	myfile << "\tstate" << std::endl;
	myfile << "\tknowledgebase" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	myfile << "(:predicates" << std::endl;
	myfile << "\t(at ?l - locatable ?wp - waypoint ?s - state)" << std::endl;
	myfile << "\t(Rat ?l - locatable ?wp - waypoint ?s - state)" << std::endl;
	myfile << "\t(has ?k - key ?s - state)" << std::endl;
	myfile << "\t(Rhas ?k - key ?s - state)" << std::endl;
	
	if (factorise)
	{
		myfile << "\t(part-of ?s - state ?kb - knowledgebase)" << std::endl;
		myfile << "\t(current_kb ?kb - knowledgebase)" << std::endl;
		myfile << "\t(parent ?kb ?kb2 - knowledgebase)" << std::endl;
	}
	
	myfile << std::endl;
	myfile << "\t;; Bookkeeping predicates." << std::endl;
	myfile << "\t(next ?l ?l2 - level)" << std::endl;
	myfile << "\t(lev ?l - LEVEL)" << std::endl;
	myfile << "\t(m ?s - STATE)" << std::endl;
	myfile << "\t(stack ?s - STATE ?l - LEVEL)" << std::endl;
	
	myfile << "\t(resolve-axioms)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	myfile << "(:constants" << std::endl;
	myfile << "\t; All the waypoints." << std::endl;
	
	for (std::vector<const Waypoint*>::const_iterator ci = waypoints.begin(); ci != waypoints.end(); ++ci)
	{
		myfile << "\t" << (*ci)->name_ << " - waypoint" << std::endl;
	}

	myfile << "\t; The keys." << std::endl;
	for (std::vector<const Key*>::const_iterator ci = keys.begin(); ci != keys.end(); ++ci)
	{
		
		myfile << "\t" << (*ci)->name_ << " - key" << std::endl;
	}

	myfile << "\t; The robot." << std::endl;
	myfile << "\trobot - robot" << std::endl;

	myfile << "\t; All the states." << std::endl;
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_->begin(); ci != knowledge_base->states_->end(); ++ci)
		{
			myfile << "\t" << (*ci)->state_name_ << " - state" << std::endl;
		}
	}

	if (factorise)
	{
		myfile << "\t; The knowledge bases" << std::endl;
		for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
		{
			const KnowledgeBase* knowledge_base = *ci;
			myfile << "\t" << knowledge_base->name_ << " - knowledgebase" << std::endl;
		}
	}

	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * NAVIGATE ACTION.
	 */
	myfile << ";; Action to move the Robot." << std::endl;
	myfile << "(:action navigate" << std::endl;
	myfile << "\t:parameters (?r - robot ?from ?to - waypoint)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_->begin(); ci != knowledge_base->states_->end(); ++ci)
		{
			myfile << "\t\t(Rat ?r ?from " << (*ci)->state_name_ << ")" << std::endl;
		}
	}
	
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(when (m " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t(and" << std::endl;
		myfile << "\t\t\t\t(not (at ?r ?from " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(at ?r ?to " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t\t(not (Rat ?r ?from " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(Rat ?r ?to " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t)" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;

	/**
	 * PICKUP action.
	 */
	myfile << ";; Pick up a key." << std::endl;
	myfile << "(:action pickup" << std::endl;
	myfile << "\t:parameters (?k - key ?r - robot ?wp - waypoint)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t;; For every state ?s" << std::endl;
		myfile << "\t\t(Rat ?r ?wp " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t(Rat ?k ?wp " << (*ci)->state_name_ << ")" << std::endl;
	}

	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t;; For every state " << (*ci)->state_name_ << "" << std::endl;
		myfile << "\t\t(when (m " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t(and" << std::endl;
		myfile << "\t\t\t\t(not (at ?k ?wp " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(has ?k " << (*ci)->state_name_ << ")" << std::endl;
		myfile << std::endl;
		myfile << "\t\t\t\t;; This optimises the search as it does not require a raminificate action to discover these facts." << std::endl;
		myfile << "\t\t\t\t(not (Rat ?k ?wp " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(Rhas ?k " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t)" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * SENSE action.
	 */
	myfile << ";; Sense for the key." << std::endl;
	myfile << "(:action sense" << std::endl;

	if (factorise)
	{
		myfile << "\t:parameters (?k - key ?r - robot ?wp - waypoint ?l ?l2 - level ?kb - knowledgebase)" << std::endl;
	}
	else
	{
		myfile << "\t:parameters (?k - key ?r - robot ?wp - waypoint ?l ?l2 - level)" << std::endl;
	}

	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	myfile << "\t\t(next ?l ?l2)" << std::endl;
	myfile << "\t\t(lev ?l)" << std::endl;
	
	if (factorise)
	{
		myfile << "\t\t(current_kb ?kb)" << std::endl;
	}
	myfile << std::endl;

	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t;; For every state ?s" << std::endl;
		myfile << "\t\t(Rat ?r ?wp " << (*ci)->state_name_ << ")" << std::endl;
		myfile << std::endl;
	}
	
	myfile << "\t\t;; This action is only applicable if there are world states where the outcome can be different." << std::endl;
	
	if (factorise)
	{
		myfile << "\t\t(exists (?s - state) (and (m ?s) (Rat ?k ?wp ?s) (part-of ?s ?kb)))" << std::endl;
		myfile << "\t\t(exists (?s - state) (and (m ?s) (not (Rat ?k ?wp ?s)) (part-of ?s ?kb)))" << std::endl;;
	}
	else
	{
		myfile << "\t\t(exists (?s - state) (and (m ?s) (Rat ?k ?wp ?s)))" << std::endl;
		myfile << "\t\t(exists (?s - state) (and (m ?s) (not (Rat ?k ?wp ?s))))" << std::endl;;
	}
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t(not (lev ?l))" << std::endl;
	myfile << "\t\t(lev ?l2)" << std::endl;
	myfile << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(when (and (m " << (*ci)->state_name_ << ") (not (Rat ?k ?wp " << (*ci)->state_name_ << ")))" << std::endl;
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
		
		// The location of the robot.
		for (std::vector<const Waypoint*>::const_iterator ci = waypoints.begin(); ci != waypoints.end(); ++ci)
		{
			myfile << "\t\t(when (or (at robot " << (*ci)->name_ << " " << state->state_name_ << ") (not (m " << state->state_name_ << ")))" << std::endl;
			myfile << "\t\t\t(Rat robot " << (*ci)->name_ << " " << state->state_name_ << ")" << std::endl;
			myfile << "\t\t)" << std::endl;
			
			myfile << "\t\t(when (and (not (at robot " << (*ci)->name_ << " " << state->state_name_ << ")) (m " << state->state_name_ << "))" << std::endl;
			myfile << "\t\t\t(not (Rat robot " << (*ci)->name_ << " " << state->state_name_ << "))" << std::endl;
			myfile << "\t\t)" << std::endl;
		}
		
		// Location of the keys.
		for (std::vector<const Key*>::const_iterator ci = keys.begin(); ci != keys.end(); ++ci)
		{
			const Key* key = *ci;
			myfile << "\t\t(when (or (has " << key->name_ << " " << state->state_name_ << ") (not (m " << state->state_name_ << ")))" << std::endl;
			myfile << "\t\t\t(Rhas " << key->name_ << " " << state->state_name_ << ")" << std::endl;
			myfile << "\t\t)" << std::endl;
			
			myfile << "\t\t(when (and (not (has " << key->name_ << " " << state->state_name_ << ")) (m " << state->state_name_ << "))" << std::endl;
			myfile << "\t\t\t(not (Rhas " << key->name_ << " " << state->state_name_ << "))" << std::endl;
			myfile << "\t\t)" << std::endl;
			
			for (std::vector<const Waypoint*>::const_iterator ci = waypoints.begin(); ci != waypoints.end(); ++ci)
			{
				const Waypoint* waypoint = *ci;
				myfile << "\t\t(when (or (at " << key->name_ << " " << waypoint->name_<< " " << state->state_name_ << ") (not (m " << state->state_name_ << ")))" << std::endl;
				myfile << "\t\t\t(Rat " << key->name_ << " " << waypoint->name_ << " " << state->state_name_ << ")" << std::endl;
				myfile << "\t\t)" << std::endl;
				
				myfile << "\t\t(when (and (not (at " << key->name_ << " " << waypoint->name_<< " " << state->state_name_ << ")) (m " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t\t(not (Rat " << key->name_ << " " << waypoint->name_ << " " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t)" << std::endl;
			}
		}
	}
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	if (factorise)
	{
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
				for (std::vector<const Waypoint*>::const_iterator ci = waypoints.begin(); ci != waypoints.end(); ++ci)
				{
					const Waypoint* waypoint = *ci;
					// Copy the states of the robot.
					myfile << "\t\t(when (and (part-of " << state->state_name_ << " ?old_kb) (Rat robot " << waypoint->name_ << " " << state->state_name_ << ") (part-of " << state2->state_name_ << " ?new_kb))" << std::endl;
					myfile << "\t\t\t(and " << std::endl;
					myfile << "\t\t\t\t(not (Rat robot " << waypoint->name_ << " " << state->state_name_ << "))" << std::endl;
					myfile << "\t\t\t\t(not (at robot " << waypoint->name_ << " " << state->state_name_ << "))" << std::endl;
					myfile << "\t\t\t\t(Rat robot " << waypoint->name_ << " " << state2->state_name_ << ")" << std::endl;
					myfile << "\t\t\t\t(at robot " << waypoint->name_ << " " << state2->state_name_ << ")" << std::endl;
					myfile << "\t\t\t)" << std::endl;
					myfile << "\t\t)" << std::endl;
					
					// Copy the states of the keys.
					for (std::vector<const Key*>::const_iterator ci = keys.begin(); ci != keys.end(); ++ci)
					{
						myfile << "\t\t(when (and (part-of " << state->state_name_ << " ?old_kb) (Rat " << (*ci)->name_ << " " << waypoint->name_ << " " << state->state_name_ << ") (part-of " << state2->state_name_ << " ?new_kb))" << std::endl;
						myfile << "\t\t\t(and " << std::endl;
						myfile << "\t\t\t\t(not (Rat " << (*ci)->name_ << " " << waypoint->name_ << " " << state->state_name_ << "))" << std::endl;
						myfile << "\t\t\t\t(not (at " << (*ci)->name_ << " " << waypoint->name_ << " " << state->state_name_ << "))" << std::endl;
						myfile << "\t\t\t\t(Rat " << (*ci)->name_ << " " << waypoint->name_ << " " << state2->state_name_ << ")" << std::endl;
						myfile << "\t\t\t\t(at " << (*ci)->name_ << " " << waypoint->name_ << " " << state2->state_name_ << ")" << std::endl;
						myfile << "\t\t\t)" << std::endl;
						myfile << "\t\t)" << std::endl;
					}
				}
				
				for (std::vector<const Key*>::const_iterator ci = keys.begin(); ci != keys.end(); ++ci)
				{
					myfile << "\t\t(when (and (part-of " << state->state_name_ << " ?old_kb) (Rhas " << (*ci)->name_ << " " << state->state_name_ << ") (part-of " << state2->state_name_ << " ?new_kb))" << std::endl;
					myfile << "\t\t\t(and " << std::endl;
					myfile << "\t\t\t\t(not (Rhas " << (*ci)->name_ << " " << " " << state->state_name_ << "))" << std::endl;
					myfile << "\t\t\t\t(not (has " << (*ci)->name_ << " " << state->state_name_ << "))" << std::endl;
					myfile << "\t\t\t\t(Rhas " << (*ci)->name_ << " " << state2->state_name_ << ")" << std::endl;
					myfile << "\t\t\t\t(has " << (*ci)->name_ << " " << state2->state_name_ << ")" << std::endl;
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
			
			// Hack, force the robot to be at the starting location for every state.
			myfile << "\t\t(or " << std::endl;
			myfile << "\t\t\t(not (part-of " << (*ci)->state_name_ << " ?old_kb))" << std::endl;
			myfile << "\t\t\t(at robot " << robot_location.name_ << " " << (*ci)->state_name_ << ")" << std::endl;
			myfile << "\t\t)" << std::endl;
			
			// Also force that all keys have been found before moving on.
			if ((*ci)->key_ != NULL)
			{
				myfile << "\t\t(or " << std::endl;
				myfile << "\t\t\t(not (part-of " << (*ci)->state_name_ << " ?old_kb))" << std::endl;
				myfile << "\t\t\t(has " << (*ci)->key_->name_ << " " << (*ci)->state_name_ << ")" << std::endl;
				myfile << "\t\t)" << std::endl;
			}
		}
		
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
			
		for (std::vector<const Waypoint*>::const_iterator ci = waypoints.begin(); ci != waypoints.end(); ++ci)
		{
			const Waypoint* waypoint = *ci;
			for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
			{
				// Deal with the location of the robot.
				const State* state = *ci;
				myfile << "\t\t(when (and " << std::endl;
				myfile << "\t\t\t\t;; For every state ?s, ?s2" << std::endl;
			
				for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
				{
					const State* state2 = *ci;
	//				myfile << "\t\t\t\t(and " << std::endl;
					myfile << "\t\t\t\t\t(or " << std::endl;
					myfile << "\t\t\t\t\t\t(not (part-of " << state2->state_name_ << " ?old_kb))" << std::endl;
					myfile << "\t\t\t\t\t\t(at robot " << waypoint->name_ << " " << state2->state_name_ << ")" << std::endl;
					myfile << "\t\t\t\t\t)" << std::endl;
	//				myfile << "\t\t\t\t)" << std::endl;
					
				}
				myfile << "\t\t\t\t\t(part-of " << state->state_name_ << " ?new_kb)" << std::endl;
				myfile << "\t\t\t)" << std::endl;

				myfile << "\t\t\t;; Conditional effects" << std::endl;
				myfile << "\t\t\t(and " << std::endl;
				for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
				{
					const State* state2 = *ci;
					myfile << "\t\t\t\t(not (at robot " << waypoint->name_ << " " << state2->state_name_ << "))" << std::endl;
				}
				myfile << "\t\t\t\t(at robot " << waypoint->name_ << " " << state->state_name_ << ")" << std::endl;
				myfile << "\t\t\t)" << std::endl;
				myfile << "\t\t)" << std::endl;
			}
			
			// Deal with the location of the key.
			for (std::vector<const Key*>::const_iterator ci = keys.begin(); ci != keys.end(); ++ci)
			{
				const Key* key = *ci;
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
						myfile << "\t\t\t\t\t\t(at " << key->name_ << " " << waypoint->name_ << " " << state2->state_name_ << ")" << std::endl;
						myfile << "\t\t\t\t\t)" << std::endl;
						myfile << "\t\t\t\t)" << std::endl;
					}

					myfile << "\t\t\t\t(part-of " << state->state_name_ << " ?new_kb)" << std::endl;
					myfile << "\t\t\t)" << std::endl;

					myfile << "\t\t\t;; Conditional effects" << std::endl;
					myfile << "\t\t\t(and " << std::endl;
					for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
					{
						const State* state2 = *ci;
						myfile << "\t\t\t\t(not (at " << key->name_ << " " << waypoint->name_ << " " << state2->state_name_ << "))" << std::endl;
					}
					myfile << "\t\t\t\t(at " << key->name_ << " " << waypoint->name_ << " " << state->state_name_ << ")" << std::endl;
					myfile << "\t\t\t)" << std::endl;
					myfile << "\t\t)" << std::endl;
				}
			}
			
			// Check whether we have the key or not.
			for (std::vector<const Key*>::const_iterator ci = keys.begin(); ci != keys.end(); ++ci)
			{
				const Key* key = *ci;
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
						myfile << "\t\t\t\t\t\t(has " << key->name_ << " " << state2->state_name_ << ")" << std::endl;
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
						myfile << "\t\t\t\t(not (has " << key->name_ << " " << state2->state_name_ << "))" << std::endl;
					}
					myfile << "\t\t\t\t(has " << key->name_ << " " << state->state_name_ << ")" << std::endl;
					
					myfile << "\t\t\t)" << std::endl;
					myfile << "\t\t)" << std::endl;
				}
		}
		}
		myfile << "\t)" << std::endl;
		myfile << ")" << std::endl;
	}
	myfile << ")" << std::endl;
	myfile.close();
}


void generateProblem(const std::string& file_name, const std::vector<const Location*>& locations, const std::vector<const Ball*>& balls, const std::vector<const Colour*>& colours, const std::vector<const Garbage*>& garbage_places)
{
	std::ofstream myfile;
	myfile.open(file_name.c_str());
	myfile << "(define (problem Keys-0)" << std::endl;
	myfile << "(:domain find_key)" << std::endl;
	myfile << "(:objects" << std::endl;
	
	for (std::vector<const Ball*>::const_iterator ci = balls.begin(); ci != balls.end(); ++ci)
	{
		myfile << "\t" << (*ci)->name_ << " - obj" << std::endl;
	}

	myfile << "\t; The locations." << std::endl;
	for (std::vector<const Location*>::const_iterator ci = locations.begin(); ci != locations.end(); ++ci)
	{
		myfile << "\t" << (*ci)->name_ << " - pos" << std::endl;
	}
	
	myfile << "\t; The colours." << std::endl;
	for (std::vector<const Colour*>::const_iterator ci = colours.begin(); ci != colours.end(); ++ci)
	{
		myfile << "\t" << (*ci)->name_ << " - col" << std::endl;
	}
	
	myfile << "\t; The garbage places." << std::endl;
	for (std::vector<const Garbage*>::const_iterator ci = garbage_places.begin(); ci != garbage_places.end(); ++ci)
	{
		myfile << "\t" << (*ci)->name_ << " - gar" << std::endl;
	}

	
	myfile << ")" << std::endl;
	myfile << "(:init" << std::endl;
	myfile << "\t(at " << locations[0]->name_ << ")" << std::endl;
	
	// Garbage constants.
	for (std::vector<const Garbage*>::const_iterator ci = garbage_places.begin(); ci != garbage_places.end(); ++ci)
	{
		myfile << "\t(garbage-at " << (*ci)->name_ << " " << (*ci)->location_.name_ << ")" << std::endl;
		myfile << "\t(garbage-color " << (*ci)->name_ << " " << (*ci)->colour_.name_ << ")" << std::endl;
	}
	
	// Location constants.
	for (std::vector<const Location*>::const_iterator ci = locations.begin(); ci != locations.end(); ++ci)
	{
		const Location* location = *ci;
		for (std::vector<const Location*>::const_iterator ci = locations.begin(); ci != locations.end(); ++ci)
		{
			const Location* location2 = *ci;
			if (location == location2) continue;
			myfile << "\t(adj " << location->name_ << " " << location2->name_ << ")" << std::endl;
			myfile << "\t(adj " << location2->name_ << " " << location->name_ << ")" << std::endl;
		}	
	}
	
	for (std::vector<const Ball*>::const_iterator ci = balls.begin(); ci != balls.end(); ++ci)
	{
		const Ball* ball = *ci;
		
		if (locations.size() > 1)
		{
			myfile << "(oneof " << std::endl;
		}
		for (std::vector<const Location*>::const_iterator ci = locations.begin(); ci != locations.end(); ++ci)
		{
			const Location* location = *ci;
			myfile << "\t(obj-at " << ball->name_ << " " << location->name_ << ")" << std::endl;
		}
		if (locations.size() > 1)
		{
			myfile << ")" << std::endl;
		}
	}
	
	for (std::vector<const Ball*>::const_iterator ci = balls.begin(); ci != balls.end(); ++ci)
	{
		const Ball* ball = *ci;
		if (colours.size() > 1)
		{
			myfile << "(oneof " << std::endl;
		}
		for (std::vector<const Colour*>::const_iterator ci = colours.begin(); ci != colours.end(); ++ci)
		{
			const Colour* colour = *ci;
			myfile << "\t(color " << ball->name_ << " " << colour->name_ << ")" << std::endl;
		}
		if (colours.size() > 1)
		{
			myfile << ")" << std::endl;
		}
	}
	
	myfile << ")" << std::endl;
	myfile << "(:goal (and" << std::endl;
	for (std::vector<const Ball*>::const_iterator ci = balls.begin(); ci != balls.end(); ++ci)
	{
		const Ball* ball = *ci;
		myfile << "\t(trashed " << ball->name_ << ")" << std::endl;
	}
	myfile << ")" << std::endl;
	myfile << ")" << std::endl;
	myfile << ")" << std::endl;
	myfile.close();
}

void generateDomain(const std::string& file_name, const std::vector<const Waypoint*>& waypoints, const std::vector<const Key*>& keys, const Waypoint& robot_location)
{
	std::ofstream myfile;
	myfile.open (file_name.c_str());
	myfile << "(define (domain find_key)" << std::endl;
	myfile << "(:requirements :typing)" << std::endl;
	myfile << std::endl;
	myfile << "(:types" << std::endl;
	myfile << "\trobot key - locatable" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	myfile << "(:predicates" << std::endl;
	myfile << "\t(at ?l - locatable ?wp - waypoint)" << std::endl;
	myfile << "\t(has ?k - key)" << std::endl;
	myfile << ")" << std::endl;
	
	/**
	 * NAVIGATE ACTION.
	 */
	myfile << ";; Action to move the Robot." << std::endl;
	myfile << "(:action navigate" << std::endl;
	myfile << "\t:parameters (?r - robot ?from ?to - waypoint)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(at ?r ?from)" << std::endl;
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t(not (at ?r ?from))" << std::endl;
	myfile << "\t\t(at ?r ?to)" << std::endl;
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;

	/**
	 * PICKUP action.
	 */
	myfile << ";; Pick up a key." << std::endl;
	myfile << "(:action pickup" << std::endl;
	myfile << "\t:parameters (?k - key ?r - robot ?wp - waypoint)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(at ?r ?wp)" << std::endl;
	myfile << "\t\t(at ?k ?wp)" << std::endl;
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t(not (at ?k ?wp))" << std::endl;
	myfile << "\t\t(has ?k)" << std::endl;
	myfile << "\t\)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * SENSE action.
	 */
	myfile << ";; Sense for the key." << std::endl;
	myfile << "(:action sense" << std::endl;
	myfile << "\t:parameters (?k - key ?r - robot ?wp - waypoint)" << std::endl;

	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(at ?r ?wp)" << std::endl;
	myfile << "\t)" << std::endl;
	myfile << "\t:observe (at ?k ?wp)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	
	myfile << ")" << std::endl;
	myfile.close();
}

int main(int argc, char **argv)
{
	std::vector<const Waypoint*> waypoints;
	std::vector<const Key*> keys;
	
	if (argc < 3)
	{
		std::cout << "Usage: <number of keys> <number of locations> {-f}" << std::endl;
		return -1;
	}
	
	unsigned int nr_keys = ::atoi(argv[1]);
	unsigned int nr_locations = ::atoi(argv[2]);
	
	bool factorise = argc == 4;
	
	std::vector<const KnowledgeBase*> knowledge_bases;
	
	State basic_state("basic", NULL, NULL);
	
	std::vector<const State*> basis_kb_states;
	basis_kb_states.push_back(&basic_state);
	
	KnowledgeBase basis_kb("basis_kb", basis_kb_states);
	
	if (factorise)
	{
		knowledge_bases.push_back(&basis_kb);
	}
	
	std::cout << "Create an instance with " << nr_keys << " keys, the number of locations " << nr_locations << "." << std::endl;
	
	unsigned int nr_states = 0;
	for (unsigned int i = 0; i < nr_keys; ++i)
	{
		std::vector<const Waypoint*> local_waypoints;
		for (unsigned int j = 0; j < nr_locations; ++j)
		{
			std::stringstream ss;
			ss << "wp" << j << "_for_key" << i;
			Waypoint* wp = new Waypoint(ss.str());
			waypoints.push_back(wp);
			local_waypoints.push_back(wp);
		}
		
		std::stringstream ss;
		ss << "key" << i;
		Key* key = new Key(ss.str());
		keys.push_back(key);
		
		ss.str(std::string());
		ss << "kb_key" << i;
		std::vector<const State*>* kb_states = new std::vector<const State*>();
		KnowledgeBase* kb = new KnowledgeBase(ss.str(), *kb_states);
		basis_kb.addChild(*kb);
		knowledge_bases.push_back(kb);
		
		// Create the states.
		for (std::vector<const Waypoint*>::const_iterator ci = local_waypoints.begin(); ci != local_waypoints.end(); ++ci)
		{
			std::cout << "State..." << std::endl;
			const Waypoint* waypoint = *ci;
			std::stringstream ss;
			ss << "s" << nr_states;
			State* state = new State(ss.str(), key, waypoint);
			kb_states->push_back(state);
			++nr_states;
		}
	}
	
	
	Waypoint wp_start("start_wp");
	waypoints.push_back(&wp_start);
	
	generateDomain("test_domain.pddl", waypoints, keys, knowledge_bases, wp_start, factorise);
	generateProblem("test_problem.pddl", waypoints, keys, knowledge_bases, wp_start, basis_kb, factorise, nr_locations);
	
	return 0;
}
