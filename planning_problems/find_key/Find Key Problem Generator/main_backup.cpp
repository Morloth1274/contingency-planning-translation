#include <iostream>
#include <fstream>
#include <string>
#include <vector>

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
	State(const std::string& state_name, const Key& key, Waypoint& waypoint)
		: state_name_(state_name), key_(&key), waypoint_(&waypoint)
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
	
	std::string name_;
	const std::vector<const State*>* states_;
};

void generateProblem(const std::string& file_name, const std::vector<const Waypoint*>& waypoints, const std::vector<const Key*>& keys, const std::vector<const KnowledgeBase*>& knowledge_base, const Waypoint& robot_location, bool factorise)
{
	std::ofstream myfile;
	myfile.open(file_name.c_str());
	myfile << "(define (problem Keys-0)" << std::endl;
	myfile << "(:domain find-key)" << std::endl;
	myfile << "(:objects" << std::endl;
	myfile << "\tl0 - LEVEL" << std::endl;
	myfile << "\tl1 - LEVEL" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;

	myfile << "(:init" << std::endl;
	myfile << "\t(resolve-axioms)" << std::endl;
	myfile << "\t(lev l0)" << std::endl;
	myfile << "\t(next l0 l1)" << std::endl;
	
	if (factorise)
	{
		myfile << "\t(current_kb kb_root)" << std::endl;
		myfile << "\t(is-part base_state kb_root)" << std::endl;
	}
	
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_base.begin(); ci != knowledge_base.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		if (factorise)
		{
			myfile << "\t(parent kb_root " << knowledge_base->name_ << ")" << std::endl;
		}
		
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_->begin(); ci != knowledge_base->states_->end(); ++ci)
		{
			const State* state = *ci;
			myfile << "\t(m " << state->state_name_ << ")" << std::endl;
			
			if (factorise)
			{
				myfile << "\t(part-of " << state->state_name_ << " " << knowledge_base->name_ << ")" << std::endl;
			}
			
			myfile << "\t(at " << state->key_->name_ << " " << state->waypoint_->name_ << " " << state->state_name_ << ")" << std::endl;
			myfile << "\t(at robot " << robot_location.name_ << " " << state->state_name_ << ")" << std::endl;
		}
	}
	myfile << ")" << std::endl;
	myfile << "(:goal (and" << std::endl;
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_base.begin(); ci != knowledge_base.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_->begin(); ci != knowledge_base->states_->end(); ++ci)
		{
			const State* state = *ci;
			myfile << "\t(has " << state->key_->name_ << " " << state->state_name_ << ")" << std::endl;
		}
	}
	myfile << ")" << std::endl;
	myfile << ")" << std::endl;
	myfile << ")" << std::endl;
	myfile.close();
}

void generateDomain(const std::string& file_name, const std::vector<const Waypoint*>& waypoints, const std::vector<const Key*>& keys, const std::vector<const KnowledgeBase*>& knowledge_base, const Waypoint& robot_location, bool factorise)
{
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
	if (factorise)
	{
		myfile << "\tknowledgebase" << std::endl;
	}
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
	if (factorise)
	{
		myfile << "\t(stack_kb ?s - STATE ?kb - knowledgebase)" << std::endl;
	}
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
	if (factorise)
	{
		myfile << "\t(base_state - state)" << std::endl;
	}
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_base.begin(); ci != knowledge_base.end(); ++ci)
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
		myfile << "\tkb_root - knowledgebase" << std::endl;
		for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_base.begin(); ci != knowledge_base.end(); ++ci)
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
	
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_base.begin(); ci != knowledge_base.end(); ++ci)
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
	
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_base.begin(); ci != knowledge_base.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_->begin(); ci != knowledge_base->states_->end(); ++ci)
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
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_base.begin(); ci != knowledge_base.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_->begin(); ci != knowledge_base->states_->end(); ++ci)
		{
			myfile << "\t\t;; For every state ?s" << std::endl;
			myfile << "\t\t(Rat ?r ?wp " << (*ci)->state_name_ << ")" << std::endl;
			myfile << "\t\t(Rat ?k ?wp " << (*ci)->state_name_ << ")" << std::endl;
		}
	}
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_base.begin(); ci != knowledge_base.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_->begin(); ci != knowledge_base->states_->end(); ++ci)
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
		myfile << std::endl;
	}
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_base.begin(); ci != knowledge_base.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_->begin(); ci != knowledge_base->states_->end(); ++ci)
		{
			myfile << "\t\t;; For every state ?s" << std::endl;
			myfile << "\t\t(Rat ?r ?wp " << (*ci)->state_name_ << ")" << std::endl;
			myfile << std::endl;
		}
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
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_base.begin(); ci != knowledge_base.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_->begin(); ci != knowledge_base->states_->end(); ++ci)
		{
			myfile << "\t\t(when (and (m " << (*ci)->state_name_ << ") (not (Rat ?k ?wp " << (*ci)->state_name_ << ")))" << std::endl;
			myfile << "\t\t\t(and (stack " << (*ci)->state_name_ << " ?l) (not (m " << (*ci)->state_name_ << ")))" << std::endl;
			myfile << "\t\t)" << std::endl;
			/*
			myfile << "\t\t(when (and (m " << (*ci)->state_name_ << ") (not (Rat ?k ?wp " << (*ci)->state_name_ << ")) (not (part-of " << (*ci)->state_name_ << " ?kb2)))" << std::endl;
			myfile << "\t\t\t(not (m " << (*ci)->state_name_ << "))" << std::endl;
			myfile << "\t\t)" << std::endl;
			*/
		}
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
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_base.begin(); ci != knowledge_base.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_->begin(); ci != knowledge_base->states_->end(); ++ci)
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
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_base.begin(); ci != knowledge_base.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_->begin(); ci != knowledge_base->states_->end(); ++ci)
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
				myfile << "\t\t\t(Rhas key0 " << state->state_name_ << ")" << std::endl;
				myfile << "\t\t)" << std::endl;
				
				myfile << "\t\t(when (and (not (has " << key->name_ << " " << state->state_name_ << ")) (m " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t\t(not (Rhas key0 " << state->state_name_ << "))" << std::endl;
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
		myfile << std::endl;
		
		myfile << "\t\t;; Now we need to delete all knowledge from the old_kb and insert it to" << std::endl;
		myfile << "\t\t;; the new_kb level." << std::endl;

		myfile << "\t\t;; For every state ?s, ?s2" << std::endl;
		for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_base.begin(); ci != knowledge_base.end(); ++ci)
		{
			const KnowledgeBase* knowledge_base = *ci;
			for (std::vector<const State*>::const_iterator ci = knowledge_base->states_->begin(); ci != knowledge_base->states_->end(); ++ci)
			{
				const State* state = *ci;
				
				myfile << "\t\t(when (and (m " << state->state_name_ << ") (not (part-of " << state->state_name_ << " ?new_kb)))" << std::endl;
				myfile << "\t\t\t(and (stack_kb " << state->state_name_ << " ?new_kb) (not (m " << state->state_name_ << ")))" << std::endl;
				myfile << "\t\t)" << std::endl;
				
				for (std::vector<const Waypoint*>::const_iterator ci = waypoints.begin(); ci != waypoints.end(); ++ci)
				{
					const Waypoint* waypoint = *ci;
					for (std::vector<const State*>::const_iterator ci = knowledge_base->states_->begin(); ci != knowledge_base->states_->end(); ++ci)
					{
						const State* state2 = *ci;
						myfile << "\t\t(when (and (part-of " << state->state_name_ << " ?old_kb) (Rat robot " << waypoint->name_ << " " << state->state_name_ << ") (part-of " << state2->state_name_ << " ?new_kb))" << std::endl;
						myfile << "\t\t\t(and " << std::endl;
						myfile << "\t\t\t\t(not (Rat robot " << waypoint->name_ << " " << state->state_name_ << "))" << std::endl;
						myfile << "\t\t\t\t(not (at robot " << waypoint->name_ << " " << state->state_name_ << "))" << std::endl;
						myfile << "\t\t\t\t(Rat robot " << waypoint->name_ << " " << state2->state_name_ << ")" << std::endl;
						myfile << "\t\t\t\t(at robot " << waypoint->name_ << " " << state2->state_name_ << ")" << std::endl;
						myfile << "\t\t\t)" << std::endl;
						myfile << "\t\t)" << std::endl;
					}
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
		myfile << "\t)" << std::endl;
		myfile << "\t:effect (and" << std::endl;
		myfile << "\t\t(not (current_kb ?old_kb))" << std::endl;
		myfile << "\t\t(current_kb ?new_kb)" << std::endl;
		
		myfile << "\t\t;; Now we need to push all knowledge that is true for all states part of " << std::endl;
		myfile << "\t\t;; kb_old up to kb_new." << std::endl;
		for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_base.begin(); ci != knowledge_base.end(); ++ci)
		{
			const KnowledgeBase* knowledge_base = *ci;
			for (std::vector<const State*>::const_iterator ci = knowledge_base->states_->begin(); ci != knowledge_base->states_->end(); ++ci)
			{
				const State* state = *ci;
				
				myfile << "\t\t(when (stack_kb " << state->state_name_ << " ?new_kb)" << std::endl;
				myfile << "\t\t\t(and (not (stack_kb " << state->state_name_ << " ?new_kb)) (m " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t)" << std::endl;
				
				for (std::vector<const Waypoint*>::const_iterator ci = waypoints.begin(); ci != waypoints.end(); ++ci)
				{
					const Waypoint* waypoint = *ci;
					for (std::vector<const State*>::const_iterator ci = knowledge_base->states_->begin(); ci != knowledge_base->states_->end(); ++ci)
					{
						const State* state2 = *ci;	
						myfile << "\t\t(when (and " << std::endl;
						myfile << "\t\t\t\t;; For every state ?s, ?s2" << std::endl;
						myfile << "\t\t\t\t(or " << std::endl;
						myfile << "\t\t\t\t\t(not (part-of " << state->state_name_ << " ?old_kb))" << std::endl;
						myfile << "\t\t\t\t\t(Rat robot " << waypoint->name_ << " " << state->state_name_ << ")" << std::endl;
						myfile << "\t\t\t\t)" << std::endl;
						myfile << "\t\t\t\t(part-of " << state2->state_name_ << " ?new_kb)" << std::endl;
						myfile << "\t\t\t)" << std::endl;

						myfile << "\t\t\t;; Conditional effects" << std::endl;
						myfile << "\t\t\t(and " << std::endl;
						myfile << "\t\t\t\t(not (Rat robot " << waypoint->name_ << " " << state->state_name_ << "))" << std::endl;
						myfile << "\t\t\t\t(not (at robot " << waypoint->name_ << " " << state->state_name_ << "))" << std::endl;
						myfile << "\t\t\t\t(Rat robot " << waypoint->name_ << " " << state2->state_name_ << ")" << std::endl;
						myfile << "\t\t\t\t(at robot " << waypoint->name_ << " " << state2->state_name_ << ")" << std::endl;
						myfile << "\t\t\t)" << std::endl;
						myfile << "\t\t)" << std::endl;
					}
				}
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
	bool factorise = false;
	if (argc > 1 && std::string(argv[1]) == "-f")
	{
		std::cout << "Factorise that shit!" << std::endl;
		factorise = true;
	}
	std::vector<const Waypoint*> waypoints;
	std::vector<const Key*> keys;
	
	// Lets setup the problem.
	Waypoint wp0("wp0"), wp1("wp1"), wp2("wp2"), wp3("wp3"), wp4("wp4");
	
	waypoints.push_back(&wp0);
	waypoints.push_back(&wp1);
	waypoints.push_back(&wp2);
	waypoints.push_back(&wp3);
	waypoints.push_back(&wp4);
	
	Key key0("key0");
	Key key1("key1");
	keys.push_back(&key0);
	keys.push_back(&key1);
	
	State key0_at_wp1("s0", key0, wp1);
	State key0_at_wp2("s1", key0, wp2);
	State key1_at_wp3("s2", key1, wp3);
	State key1_at_wp4("s3", key1, wp4);
	
	std::vector<const State*> key0_kb_states;
	key0_kb_states.push_back(&key0_at_wp1);
	key0_kb_states.push_back(&key0_at_wp2);
	
	std::vector<const State*> key1_kb_states;
	key1_kb_states.push_back(&key1_at_wp3);
	key1_kb_states.push_back(&key1_at_wp4);
	
	KnowledgeBase key0_kb("kb_key0", key0_kb_states);
	KnowledgeBase key1_kb("kb_key1", key1_kb_states);
	
	std::vector<const KnowledgeBase*> knowledge_bases;
	knowledge_bases.push_back(&key0_kb);
	knowledge_bases.push_back(&key1_kb);
	
	generateDomain("test_domain.pddl", waypoints, keys, knowledge_bases, wp0, factorise);
	generateProblem("test_problem.pddl", waypoints, keys, knowledge_bases, wp0, factorise);
	
	return 0;
}
