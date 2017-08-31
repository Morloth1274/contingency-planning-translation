#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <stdlib.h>
#include <map>
#include <boost/concept_check.hpp>

struct Colour;

struct Palette
{
	Palette(const std::string& name, const std::vector<Colour*>& palette)
		: name_(name), palette_(palette)
	{
		
	}
	
	std::string name_;
	std::vector<Colour*> palette_;
};

struct Colour
{
	Colour(const std::string& name)
		: name_(name)
	{
		
	}
	
	void isPartOf(Palette& palette)
	{
		is_part_of_.push_back(&palette);
	}
	
	std::string name_;
	std::vector<Palette*> is_part_of_;
};

struct Toy
{
	Toy(const std::string& name)
		: name_(name)
	{
		
	}
	
	std::string name_;
};


struct Box
{
	Box(const std::string& name, const Palette& palette)
		: name_(name), palette_(palette)
	{
		
	}
	
	std::string name_;
	Palette palette_;
};

struct State
{
	State(const std::string& state_name, std::map<const Toy*, const Palette*>& colour_mapping)
		: state_name_(state_name), colour_mapping_(colour_mapping)
	{
		
	}
	
	std::string state_name_;
	std::map<const Toy*, const Palette*> colour_mapping_;
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

void generateProblem(const std::string& file_name, const KnowledgeBase& current_knowledge_base, const std::vector<const KnowledgeBase*>& knowledge_base, const std::vector<const Toy*>& toys, const std::vector<const Palette*>& palettes, const std::vector<const Box*>& box_places, bool factorise)
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
	
	if (factorise)
	{
		myfile << "\tl0 - LEVEL" << std::endl;
		myfile << "\tl1 - LEVEL" << std::endl;
	}
	else
	{
		for (unsigned int key_nr = 0; key_nr < balls.size() + colours.size() + 1; ++key_nr)
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
	else
	{
		for (unsigned int key_nr = 1; key_nr < balls.size() + colours.size() + 1; ++key_nr)
		{
			myfile << "\t(next l" << (key_nr - 1) << " l" << key_nr << ")" << std::endl;
		}
	}
	
	if (factorise)
	{
		myfile << "\t(current_kb " << current_knowledge_base.name_ << ")" << std::endl;
		for (std::vector<const State*>::const_iterator ci = current_knowledge_base.states_.begin(); ci != current_knowledge_base.states_.end(); ++ci)
		{
			myfile << "\t(part-of " << (*ci)->state_name_ << " " << current_knowledge_base.name_ << ")" << std::endl;
			myfile << "\t(m " << (*ci)->state_name_ << ")" << std::endl;
			myfile << "\t(at " << locations[0]->name_ << " " << (*ci)->state_name_ << ")" << std::endl;
		}
	}
	else
	{
		for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_base.begin(); ci != knowledge_base.end(); ++ci)
		{
			const KnowledgeBase* kb = *ci;
			for (std::vector<const State*>::const_iterator ci = kb->states_.begin(); ci != kb->states_.end(); ++ci)
			{
				const State* state = *ci;
				myfile << "\t(m " << state->state_name_ << ")" << std::endl;
			}
		}
	}
	
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
	
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_base.begin(); ci != knowledge_base.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_.begin(); ci != knowledge_base->states_.end(); ++ci)
		{
			const State* state = *ci;
			if (factorise)
			{
				myfile << "\t(part-of " << state->state_name_ << " " << knowledge_base->name_ << ")" << std::endl;
			}
			
			for (std::map<const Ball*, const Colour*>::const_iterator ci = state->colour_mapping_.begin(); ci != state->colour_mapping_.end(); ++ci)
			{
				myfile << "\t(color " << (*ci).first->name_ << " " << (*ci).second->name_ << " " << state->state_name_ << ")" << std::endl;
			}
			for (std::map<const Ball*, const Location*>::const_iterator ci = state->location_mapping_.begin(); ci != state->location_mapping_.end(); ++ci)
			{
				myfile << "\t(obj-at " << (*ci).first->name_ << " " << (*ci).second->name_ << " " << state->state_name_ << ")" << std::endl;
			}
			
			if (!factorise)
			{
				myfile << "\t(at " << locations[0]->name_ << " " << state->state_name_ << ")" << std::endl;
			}
		}
		
		for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_base->children_.begin(); ci != knowledge_base->children_.end(); ++ci)
		{
			myfile << "\t(parent " << knowledge_base->name_ << " " << (*ci)->name_ << ")" << std::endl;
		}
	}
	myfile << ")" << std::endl;
	myfile << "(:goal (and" << std::endl;
	if (factorise)
	{
		for (std::vector<const Ball*>::const_iterator ci = balls.begin(); ci != balls.end(); ++ci)
		{
			const Ball* ball = *ci;
			for (std::vector<const State*>::const_iterator ci = current_knowledge_base.states_.begin(); ci != current_knowledge_base.states_.end(); ++ci)
			{
				myfile << "\t(trashed " << ball->name_ << " " << (*ci)->state_name_ << ")" << std::endl;
			}
		}
	}
	else
	{
		for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_base.begin(); ci != knowledge_base.end(); ++ci)
		{
			const KnowledgeBase* knowledge_base = *ci;
			
			for (std::vector<const State*>::const_iterator ci = knowledge_base->states_.begin(); ci != knowledge_base->states_.end(); ++ci)
			{
				const State* state = *ci;
				for (std::vector<const Ball*>::const_iterator ci = balls.begin(); ci != balls.end(); ++ci)
				{
					const Ball* ball = *ci;
					myfile << "\t(trashed " << ball->name_ << " " << state->state_name_ << ")" << std::endl;
				}
			}
		}
	}
	myfile << ")" << std::endl;
	myfile << ")" << std::endl;
	myfile << ")" << std::endl;
	myfile.close();
}

void generateDomain(const std::string& file_name, const KnowledgeBase& current_knowledge_base, const std::vector<const KnowledgeBase*>& knowledge_bases, const std::vector<const Location*>& locations, const std::vector<const Ball*>& balls, const std::vector<const Colour*>& colours, const std::vector<const Garbage*>& garbage_places, bool factorise)
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
	myfile << "\tpos obj col gar" << std::endl;
	myfile << "\tlevel" << std::endl;
	myfile << "\tstate" << std::endl;
	myfile << "\tknowledgebase" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	myfile << "(:predicates" << std::endl;
	myfile << "\t(color ?o - obj ?c - col ?s - state)" << std::endl;
	myfile << "\t(Rcolor ?o - obj ?c - col ?s - state)" << std::endl;
	myfile << "\t(trashed ?o - obj ?s - state)" << std::endl;
	myfile << "\t(Rtrashed ?o - obj ?s - state)" << std::endl;
	myfile << "\t(at ?p - pos ?s - state)" << std::endl;
	myfile << "\t(Rat ?p - pos ?s - state)" << std::endl;
	myfile << "\t(holding ?o - obj ?s - state)" << std::endl;
	myfile << "\t(Rholding ?o - obj ?s - state)" << std::endl;
	myfile << "\t(obj-at ?o - obj ?p - pos ?s - state)" << std::endl;
	myfile << "\t(Robj-at ?o - obj ?p - pos ?s - state)" << std::endl;
	
	myfile << "\t(garbage-color ?g - gar ?c - col)" << std::endl;
	myfile << "\t(garbage-at ?g - gar ?l - pos)" << std::endl;
	myfile << "\t(adj ?i ?j - pos)" << std::endl;
	
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
	myfile << "\t; All the balls." << std::endl;
	
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

	myfile << "\t; All the states." << std::endl;
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_.begin(); ci != knowledge_base->states_.end(); ++ci)
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
	myfile << ";; Action to dunk the package." << std::endl;
	myfile << "(:action move" << std::endl;
	myfile << "\t:parameters (?i ?j - pos)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	myfile << "\t\t(adj ?i ?j)" << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_.begin(); ci != knowledge_base->states_.end(); ++ci)
		{
			myfile << "\t\t(Rat ?i " << (*ci)->state_name_ << ")" << std::endl;
		}
	}
	
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(when (m " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t(and" << std::endl;
		myfile << "\t\t\t\t(not (at ?i " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(not (Rat ?i " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(at ?j " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t\t(Rat ?j " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t)" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;

	/**
	 * PICKUP action.
	 */
	myfile << ";; Pick a ball up." << std::endl;
	myfile << "(:action pickup" << std::endl;
	myfile << "\t:parameters (?o - obj ?i - pos)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_.begin(); ci != knowledge_base->states_.end(); ++ci)
		{
			myfile << "\t\t(Rat ?i " << (*ci)->state_name_ << ")" << std::endl;
			myfile << "\t\t(Robj-at ?o ?i " << (*ci)->state_name_ << ")" << std::endl;
		}
	}
	
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t;; For every state " << (*ci)->state_name_ << "" << std::endl;
		myfile << "\t\t(when (m " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t(and" << std::endl;
		myfile << "\t\t\t\t(holding ?o " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t\t(Rholding ?o " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t\t(not (obj-at ?o ?i " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(not (Robj-at ?o ?i " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t)" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * TRASH action.
	 */
	myfile << ";; Trash a ball." << std::endl;
	myfile << "(:action trash" << std::endl;
	myfile << "\t:parameters (?o - obj ?c - col ?t -gar ?p - pos)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_.begin(); ci != knowledge_base->states_.end(); ++ci)
		{
			myfile << "\t\t(Rcolor ?o ?c " << (*ci)->state_name_ << ")" << std::endl;
			myfile << "\t\t(Rholding ?o " << (*ci)->state_name_ << ")" << std::endl;
			myfile << "\t\t(Rat ?p " << (*ci)->state_name_ << ")" << std::endl;
			myfile << "\t\t(garbage-at ?t ?p)" << std::endl;
			myfile << "\t\t(garbage-color ?t ?c)" << std::endl;
		}
	}
	
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t;; For every state " << (*ci)->state_name_ << "" << std::endl;
		myfile << "\t\t(when (m " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t(and" << std::endl;
		myfile << "\t\t\t\t(trashed ?o " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t\t(Rtrashed ?o " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t\t(not (holding ?o " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(not (Rholding ?o " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t)" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * SENSE action.
	 */
	myfile << ";; Sense the color." << std::endl;
	myfile << "(:action observe-color" << std::endl;

	if (factorise)
	{
		myfile << "\t:parameters (?c - col ?o - obj ?l ?l2 - level ?kb - knowledgebase)" << std::endl;
	}
	else
	{
		myfile << "\t:parameters (?c - col ?o - obj ?l ?l2 - level)" << std::endl;
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
	
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_.begin(); ci != knowledge_base->states_.end(); ++ci)
		{
			myfile << "\t\t(Rholding ?o " << (*ci)->state_name_ << ")" << std::endl;
		}
	}

	myfile << "\t\t;; This action is only applicable if there are world states where the outcome can be different." << std::endl;
	
	if (factorise)
	{
		myfile << "\t\t(exists (?s - state) (and (m ?s) (color ?o ?c ?s) (part-of ?s ?kb)))" << std::endl;
		myfile << "\t\t(exists (?s - state) (and (m ?s) (not (color ?o ?c ?s)) (part-of ?s ?kb)))" << std::endl;;
	}
	else
	{
		myfile << "\t\t(exists (?s - state) (and (m ?s) (color ?o ?c ?s)))" << std::endl;
		myfile << "\t\t(exists (?s - state) (and (m ?s) (not (color ?o ?c ?s))))" << std::endl;;
	}
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t(not (lev ?l))" << std::endl;
	myfile << "\t\t(lev ?l2)" << std::endl;
	myfile << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(when (and (m " << (*ci)->state_name_ << ") (not (color ?o ?c " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t(and (stack " << (*ci)->state_name_ << " ?l) (not (m " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	myfile << "\t\t(resolve-axioms)" << std::endl;
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	
	
	myfile << ";; Sense the ball." << std::endl;
	myfile << "(:action observe-ball" << std::endl;

	if (factorise)
	{
		myfile << "\t:parameters (?pos - pos ?o - obj ?l ?l2 - level ?kb - knowledgebase)" << std::endl;
	}
	else
	{
		myfile << "\t:parameters (?pos - pos ?o - obj ?l ?l2 - level)" << std::endl;
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
	
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_.begin(); ci != knowledge_base->states_.end(); ++ci)
		{
			myfile << "\t\t(Rat ?pos " << (*ci)->state_name_ << ")" << std::endl;
		}
	}

	myfile << "\t\t;; This action is only applicable if there are world states where the outcome can be different." << std::endl;
	
	if (factorise)
	{
		myfile << "\t\t(exists (?s - state) (and (m ?s) (obj-at ?o ?pos ?s) (part-of ?s ?kb)))" << std::endl;
		myfile << "\t\t(exists (?s - state) (and (m ?s) (not (obj-at ?o ?pos ?s)) (part-of ?s ?kb)))" << std::endl;;
	}
	else
	{
		myfile << "\t\t(exists (?s - state) (and (m ?s) (obj-at ?o ?pos ?s)))" << std::endl;
		myfile << "\t\t(exists (?s - state) (and (m ?s) (not (obj-at ?o ?pos ?s))))" << std::endl;;
	}
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t(not (lev ?l))" << std::endl;
	myfile << "\t\t(lev ?l2)" << std::endl;
	myfile << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(when (and (m " << (*ci)->state_name_ << ") (not (obj-at ?o ?pos " << (*ci)->state_name_ << ")))" << std::endl;
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
		
		for (std::vector<const Ball*>::const_iterator ci = balls.begin(); ci != balls.end(); ++ci)
		{
			const Ball* ball = *ci;
			myfile << "\t\t(when (or (holding " << ball->name_ << " " << state->state_name_ << ") (not (m " << state->state_name_ << ")))" << std::endl;
			myfile << "\t\t\t(Rholding " << ball->name_ << " " << state->state_name_ << ")" << std::endl;
			myfile << "\t\t)" << std::endl;
			
			myfile << "\t\t(when (and (not (holding " << ball->name_ << " " << state->state_name_ << ")) (m " << state->state_name_ << "))" << std::endl;
			myfile << "\t\t\t(not (Rholding " << ball->name_ << " " << state->state_name_ << "))" << std::endl;
			myfile << "\t\t)" << std::endl;
			
			myfile << "\t\t(when (or (trashed " << ball->name_ << " " << state->state_name_ << ") (not (m " << state->state_name_ << ")))" << std::endl;
			myfile << "\t\t\t(Rtrashed " << ball->name_ << " " << state->state_name_ << ")" << std::endl;
			myfile << "\t\t)" << std::endl;
			
			myfile << "\t\t(when (and (not (trashed " << ball->name_ << " " << state->state_name_ << ")) (m " << state->state_name_ << "))" << std::endl;
			myfile << "\t\t\t(not (Rtrashed " << ball->name_ << " " << state->state_name_ << "))" << std::endl;
			myfile << "\t\t)" << std::endl;
			
			for (std::vector<const Location*>::const_iterator ci = locations.begin(); ci != locations.end(); ++ci)
			{
				const Location* location = *ci;
				myfile << "\t\t(when (or (obj-at " << ball->name_ << " " << location->name_<< " " << state->state_name_ << ") (not (m " << state->state_name_ << ")))" << std::endl;
				myfile << "\t\t\t(Robj-at " << ball->name_ << " " << location->name_<< " " << state->state_name_ << ")" << std::endl;
				myfile << "\t\t)" << std::endl;
				
				myfile << "\t\t(when (and (not (obj-at " << ball->name_ << " " << location->name_<< " " << state->state_name_ << ")) (m " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t\t(not (Robj-at " << ball->name_ << " " << location->name_<< " " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t)" << std::endl;
			}
			
			for (std::vector<const Colour*>::const_iterator ci = colours.begin(); ci != colours.end(); ++ci)
			{
				const Colour* colour = *ci;
				myfile << "\t\t(when (or (color " << ball->name_ << " " << colour->name_<< " " << state->state_name_ << ") (not (m " << state->state_name_ << ")))" << std::endl;
				myfile << "\t\t\t(Rcolor " << ball->name_ << " " << colour->name_<< " " << state->state_name_ << ")" << std::endl;
				myfile << "\t\t)" << std::endl;
				
				myfile << "\t\t(when (and (not (color " << ball->name_ << " " << colour->name_<< " " << state->state_name_ << ")) (m " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t\t(not (Rcolor " << ball->name_ << " " << colour->name_<< " " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t)" << std::endl;
			}
		}
		
		for (std::vector<const Location*>::const_iterator ci = locations.begin(); ci != locations.end(); ++ci)
		{
			const Location* location = *ci;
			myfile << "\t\t(when (or (at " << location->name_<< " " << state->state_name_ << ") (not (m " << state->state_name_ << ")))" << std::endl;
			myfile << "\t\t\t(Rat " << location->name_<< " " << state->state_name_ << ")" << std::endl;
			myfile << "\t\t)" << std::endl;
			
			myfile << "\t\t(when (and (not (at " << location->name_<< " " << state->state_name_ << ")) (m " << state->state_name_ << "))" << std::endl;
			myfile << "\t\t\t(not (Rat " << location->name_<< " " << state->state_name_ << "))" << std::endl;
			myfile << "\t\t)" << std::endl;
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
				for (std::vector<const Ball*>::const_iterator ci = balls.begin(); ci != balls.end(); ++ci)
				{
					const Ball* ball = *ci;
					
					myfile << "\t\t(when (and (part-of " << state->state_name_ << " ?old_kb) (holding " << ball->name_ << " " << state->state_name_ << ") (part-of " << state2->state_name_ << " ?new_kb))" << std::endl;
					myfile << "\t\t\t(and " << std::endl;
					myfile << "\t\t\t\t(not (Rholding " << ball->name_ << " " << state->state_name_ << "))" << std::endl;
					myfile << "\t\t\t\t(not (holding " << ball->name_ << " " << state->state_name_ << "))" << std::endl;
					myfile << "\t\t\t\t(Rholding " << ball->name_ << " " << state2->state_name_ << ")" << std::endl;
					myfile << "\t\t\t\t(holding " << ball->name_ << " " << state2->state_name_ << ")" << std::endl;
					myfile << "\t\t\t)" << std::endl;
					myfile << "\t\t)" << std::endl;
					
					myfile << "\t\t(when (and (part-of " << state->state_name_ << " ?old_kb) (trashed " << ball->name_ << " " << state->state_name_ << ") (part-of " << state2->state_name_ << " ?new_kb))" << std::endl;
					myfile << "\t\t\t(and " << std::endl;
					myfile << "\t\t\t\t(not (Rtrashed " << ball->name_ << " " << state->state_name_ << "))" << std::endl;
					myfile << "\t\t\t\t(not (trashed " << ball->name_ << " " << state->state_name_ << "))" << std::endl;
					myfile << "\t\t\t\t(Rtrashed " << ball->name_ << " " << state2->state_name_ << ")" << std::endl;
					myfile << "\t\t\t\t(trashed " << ball->name_ << " " << state2->state_name_ << ")" << std::endl;
					myfile << "\t\t\t)" << std::endl;
					myfile << "\t\t)" << std::endl;
					
					for (std::vector<const Location*>::const_iterator ci = locations.begin(); ci != locations.end(); ++ci)
					{
						const Location* location = *ci;
						myfile << "\t\t(when (and (part-of " << state->state_name_ << " ?old_kb) (obj-at " << ball->name_ << " " << location->name_ << " " << state->state_name_ << ") (part-of " << state2->state_name_ << " ?new_kb))" << std::endl;
						myfile << "\t\t\t(and " << std::endl;
						myfile << "\t\t\t\t(not (Robj-at " << ball->name_ << " " << location->name_ << " " << state->state_name_ << "))" << std::endl;
						myfile << "\t\t\t\t(not (obj-at " << ball->name_ << " " << location->name_ << " " << state->state_name_ << "))" << std::endl;
						myfile << "\t\t\t\t(Robj-at " << ball->name_ << " " << location->name_ << " " << state2->state_name_ << ")" << std::endl;
						myfile << "\t\t\t\t(obj-at " << ball->name_ << " " << location->name_ << " " << state2->state_name_ << ")" << std::endl;
						myfile << "\t\t\t)" << std::endl;
						myfile << "\t\t)" << std::endl;
					}
					
					for (std::vector<const Colour*>::const_iterator ci = colours.begin(); ci != colours.end(); ++ci)
					{
						const Colour* colour = *ci;
						myfile << "\t\t(when (and (part-of " << state->state_name_ << " ?old_kb) (color " << ball->name_ << " " << colour->name_ << " " << state->state_name_ << ") (part-of " << state2->state_name_ << " ?new_kb))" << std::endl;
						myfile << "\t\t\t(and " << std::endl;
						myfile << "\t\t\t\t(not (Rcolor " << ball->name_ << " " << colour->name_ << " " << state->state_name_ << "))" << std::endl;
						myfile << "\t\t\t\t(not (color " << ball->name_ << " " << colour->name_ << " " << state->state_name_ << "))" << std::endl;
						myfile << "\t\t\t\t(Rcolor " << ball->name_ << " " << colour->name_ << " " << state2->state_name_ << ")" << std::endl;
						myfile << "\t\t\t\t(color " << ball->name_ << " " << colour->name_ << " " << state2->state_name_ << ")" << std::endl;
						myfile << "\t\t\t)" << std::endl;
						myfile << "\t\t)" << std::endl;
					}
				}
				
				for (std::vector<const Location*>::const_iterator ci = locations.begin(); ci != locations.end(); ++ci)
				{
					const Location* location = *ci;
					myfile << "\t\t(when (and (part-of " << state->state_name_ << " ?old_kb) (at " << location->name_ << " " << state->state_name_ << ") (part-of " << state2->state_name_ << " ?new_kb))" << std::endl;
					myfile << "\t\t\t(and " << std::endl;
					myfile << "\t\t\t\t(not (Rat " << location->name_ << " " << state->state_name_ << "))" << std::endl;
					myfile << "\t\t\t\t(not (at " << location->name_ << " " << state->state_name_ << "))" << std::endl;
					myfile << "\t\t\t\t(Rat " << location->name_ << " " << state2->state_name_ << ")" << std::endl;
					myfile << "\t\t\t\t(at " << location->name_ << " " << state2->state_name_ << ")" << std::endl;
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
		
		// Make sure the agent is in the same location.
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
				myfile << "\t\t\t\t\t(at " << location->name_ << " " << state->state_name_ << ")" << std::endl;
				myfile << "\t\t\t\t)" << std::endl;
			}
			myfile << "\t\t\t)" << std::endl;
		}
		myfile << "\t\t)" << std::endl;
		
		// Make sure the balls are in the same state.
		myfile << "\t\t(or";
		for (std::vector<const Ball*>::const_iterator ci = balls.begin(); ci != balls.end(); ++ci)
		{
			const Ball* ball = *ci;
			/**
			 * The observable fact of the location of the ball will NEVER be the same.
			myfile << "\t\t\t(and";
			for (std::vector<const Location*>::const_iterator ci = locations.begin(); ci != locations.end(); ++ci)
			{
				const Location* location = *ci;
				for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
				{
					const State* state = *ci;
					// Make sure the state of the toilets are the same.
					myfile << "\t\t\t\t(or " << std::endl;
					myfile << "\t\t\t\t\t(not (part-of " << (*ci)->state_name_ << " ?old_kb))" << std::endl;
					myfile << "\t\t\t\t\t(obj-at " << ball->name_ << " " << location->name_ << " " << state->state_name_ << ")" << std::endl;
					myfile << "\t\t\t\t)" << std::endl;
				}
			}
			myfile << "\t\t\t)" << std::endl;
			*/
			
			myfile << "\t\t\t(and";
			for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
			{
				const State* state = *ci;
				// Make sure the state of the toilets are the same.
				myfile << "\t\t\t\t(or " << std::endl;
				myfile << "\t\t\t\t\t(not (part-of " << (*ci)->state_name_ << " ?old_kb))" << std::endl;
				myfile << "\t\t\t\t\t(holding " << ball->name_ << " " << state->state_name_ << ")" << std::endl;
				myfile << "\t\t\t\t)" << std::endl;
			}
			myfile << "\t\t\t)" << std::endl;
			
			myfile << "\t\t\t(and";
			for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
			{
				const State* state = *ci;
				// Make sure the state of the toilets are the same.
				myfile << "\t\t\t\t(or " << std::endl;
				myfile << "\t\t\t\t\t(not (part-of " << (*ci)->state_name_ << " ?old_kb))" << std::endl;
				myfile << "\t\t\t\t\t(trashed " << ball->name_ << " " << state->state_name_ << ")" << std::endl;
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
			
		for (std::vector<const Ball*>::const_iterator ci = balls.begin(); ci != balls.end(); ++ci)
		{
			const Ball* ball = *ci;
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
					myfile << "\t\t\t\t\t\t(holding " << ball->name_ << " " << state2->state_name_ << ")" << std::endl;
					myfile << "\t\t\t\t\t)" << std::endl;
					
				}
				myfile << "\t\t\t\t\t(part-of " << state->state_name_ << " ?new_kb)" << std::endl;
				myfile << "\t\t\t)" << std::endl;

				myfile << "\t\t\t;; Conditional effects" << std::endl;
				myfile << "\t\t\t(and " << std::endl;
				for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
				{
					const State* state2 = *ci;
					myfile << "\t\t\t\t(not (holding " << ball->name_ << " " << state2->state_name_ << "))" << std::endl;
				}
				myfile << "\t\t\t\t(holding " << ball->name_ << " " << state->state_name_ << ")" << std::endl;
				myfile << "\t\t\t)" << std::endl;
				myfile << "\t\t)" << std::endl;
				
				// Trashed.
				myfile << "\t\t(when (and " << std::endl;
				myfile << "\t\t\t\t;; For every state ?s, ?s2" << std::endl;
			
				for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
				{
					const State* state2 = *ci;
					myfile << "\t\t\t\t\t(or " << std::endl;
					myfile << "\t\t\t\t\t\t(not (part-of " << state2->state_name_ << " ?old_kb))" << std::endl;
					myfile << "\t\t\t\t\t\t(trashed " << ball->name_ << " " << state2->state_name_ << ")" << std::endl;
					myfile << "\t\t\t\t\t)" << std::endl;
					
				}
				myfile << "\t\t\t\t\t(part-of " << state->state_name_ << " ?new_kb)" << std::endl;
				myfile << "\t\t\t)" << std::endl;

				myfile << "\t\t\t;; Conditional effects" << std::endl;
				myfile << "\t\t\t(and " << std::endl;
				for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
				{
					const State* state2 = *ci;
					myfile << "\t\t\t\t(not (trashed " << ball->name_ << " " << state2->state_name_ << "))" << std::endl;
				}
				myfile << "\t\t\t\t(trashed " << ball->name_ << " " << state->state_name_ << ")" << std::endl;
				myfile << "\t\t\t)" << std::endl;
				myfile << "\t\t)" << std::endl;
			}
			
			/**
			 * The observable facts that are the location and coloour of a ball will never by pushed to the 
			 * global knowledge base.
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
							
						myfile << "\t\t\t\t\t(or " << std::endl;
						myfile << "\t\t\t\t\t\t(not (part-of " << state2->state_name_ << " ?old_kb))" << std::endl;
						myfile << "\t\t\t\t\t\t(obj-at " << ball->name_ << " " << location->name_ << " " << state2->state_name_ << ")" << std::endl;
						myfile << "\t\t\t\t\t)" << std::endl;
					}

					myfile << "\t\t\t\t(part-of " << state->state_name_ << " ?new_kb)" << std::endl;
					myfile << "\t\t\t)" << std::endl;

					myfile << "\t\t\t;; Conditional effects" << std::endl;
					myfile << "\t\t\t(and " << std::endl;
					for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
					{
						const State* state2 = *ci;
						myfile << "\t\t\t\t(not (obj-at " << ball->name_ << " " << location->name_ << " " << state2->state_name_ << "))" << std::endl;
					}
					myfile << "\t\t\t\t(obj-at " << ball->name_ << " " << location->name_ << " " << state->state_name_ << ")" << std::endl;
					myfile << "\t\t\t)" << std::endl;
					myfile << "\t\t)" << std::endl;
				}
			}
			*/
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
					myfile << "\t\t\t\t\t\t(at " << location->name_ << " " << state2->state_name_ << ")" << std::endl;
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
					myfile << "\t\t\t\t(not (at " << location->name_ << " " << state2->state_name_ << "))" << std::endl;
				}
				myfile << "\t\t\t\t(at " << location->name_ << " " << state->state_name_ << ")" << std::endl;
				
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

void generateDomain(const std::string& file_name, const std::vector<const Location*>& locations, const std::vector<const Ball*>& balls, const std::vector<const Colour*>& colours, const std::vector<const Garbage*>& garbage_places)
{
	std::ofstream myfile;
	myfile.open (file_name.c_str());
	myfile << "(define (domain find_key)" << std::endl;
	myfile << "(:requirements :typing)" << std::endl;
	myfile << std::endl;
	myfile << "(:types" << std::endl;
	myfile << "\tpos obj col gar" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	myfile << "(:predicates" << std::endl;
	myfile << "\t(color ?o - obj ?c - col)" << std::endl;
	myfile << "\t(trashed ?o - obj)" << std::endl;
	myfile << "\t(at ?p - pos)" << std::endl;
	myfile << "\t(holding ?o - obj)" << std::endl;
	myfile << "\t(obj-at ?o - obj ?p - pos)" << std::endl;
	
	myfile << "\t(garbage-color ?g - gar ?c - col)" << std::endl;
	myfile << "\t(garbage-at ?g - gar ?l - pos)" << std::endl;
	myfile << "\t(adj ?i ?j - pos)" << std::endl;
	myfile << ")" << std::endl;
	
	/**
	 * NAVIGATE ACTION.
	 */
	myfile << ";; Action to dunk the package." << std::endl;
	myfile << "(:action move" << std::endl;
	myfile << "\t:parameters (?i ?j - pos)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(adj ?i ?j)" << std::endl;
	myfile << "\t\t(at ?i)" << std::endl;
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t(not (at ?i))" << std::endl;
	myfile << "\t\t(at ?j)" << std::endl;
	
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;

	/**
	 * PICKUP action.
	 */
	myfile << ";; Pick a ball up." << std::endl;
	myfile << "(:action pickup" << std::endl;
	myfile << "\t:parameters (?o - obj ?i - pos)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(at ?i)" << std::endl;
	myfile << "\t\t(obj-at ?o ?i)" << std::endl;
	
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t(holding ?o)" << std::endl;
	myfile << "\t\t(not (obj-at ?o ?i))" << std::endl;
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * TRASH action.
	 */
	myfile << ";; Trash a ball." << std::endl;
	myfile << "(:action trash" << std::endl;
	myfile << "\t:parameters (?o - obj ?c - col ?t -gar ?p - pos)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(color ?o ?c)" << std::endl;
	myfile << "\t\t(holding ?o)" << std::endl;
	myfile << "\t\t(at ?p)" << std::endl;
	myfile << "\t\t(garbage-at ?t ?p)" << std::endl;
	myfile << "\t\t(garbage-color ?t ?c)" << std::endl;
	
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t(trashed ?o)" << std::endl;
	myfile << "\t\t(not (holding ?o))" << std::endl;
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * SENSE action.
	 */
	myfile << ";; Sense the color." << std::endl;
	myfile << "(:action observe-color" << std::endl;
	myfile << "\t:parameters (?c - col ?o - obj)" << std::endl;

	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(holding ?o)" << std::endl;
	myfile << "\t)" << std::endl;
	myfile << "\t:observe (color ?o ?c)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	myfile << ";; Sense the ball." << std::endl;
	myfile << "(:action observe-ball" << std::endl;
	myfile << "\t:parameters (?pos - pos ?o - obj)" << std::endl;

	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(at ?pos)" << std::endl;
	myfile << "\t)" << std::endl;
	myfile << "\t:observe (obj-at ?o ?pos)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	myfile << ")" << std::endl;
	myfile.close();
}

int main(int argc, char **argv)
{
	std::vector<const Toy*> toys;
	std::vector<const Colour*> colours;
	std::vector<const Palette*> palettes;
	std::vector<const Box*> boxes;
	
	if (argc < 4)
	{
		std::cout << "Usage: <number of toys> <number of colours> {-f}" << std::endl;
		return -1;
	}
	
	unsigned int nr_toys = ::atoi(argv[1]);
	unsigned int nr_colours = ::atoi(argv[2]);
	
	enum MODE { ORIGINAL, FACTORISED, PRP};
	MODE mode = ORIGINAL;
	
	if (argc == 4)
	{
		mode = ORIGINAL;
	}
	else if (std::string(argv[4]) == "-f")
	{
		mode = FACTORISED;
	}
	else if (std::string(argv[4]) == "-p")
	{
		mode = PRP;
	}
	else
	{
		std::cerr << "Unknown options: " << argv[4] << std::endl;
		exit(-1);
	}
	std::stringstream ss;
	
	for (unsigned int i = 0; i < nr_toys; ++i)
	{
		ss.str(std::string());
		ss << "toy_" << i;
		Toy* t = new Toy(ss.str());
		toys.push_back(t);
	}
	
	for (unsigned int i = 0; i < nr_colours; ++i)
	{
		ss.str(std::string());
		ss << "colour_" << i;
		Colour* c = new Colour(ss.str());
		colours.push_back(c);
	}
	
	for (Colour* c : colours)
	{
		std::vector<Colour*> single_colour_palette;
		single_colour_palette.push_back(c);
		ss.str(std::string());
		ss << "palette_" << c->name_;
		Palette* p = new Palette(ss.str(), single_colour_palette);
		palettes.push_back(p);
		
		ss.str(std::string());
		ss << "box_" << c->name_;
		Box* b = new Box(ss.str(), *p);
		boxes.push_back(b);
		for (Colour* c2 : colours)
		{
			if (c == c2) continue;
			std::vector<Colour*> dual_colour_palette;
			dual_colour_palette.push_back(c);
			dual_colour_palette.push_back(c2);
			ss.str(std::string());
			ss << "palette_" << c->name_ << "_and_" << c2->name_;
			Palette* p2 = new Palette(ss.str(), dual_colour_palette);
			palettes.push_back(p2);
			
			ss.str(std::string());
			ss << "box_" << c->name_ << "_and_" << c2->name_;
			Box* b2 = new Box(ss.str(), *p2);
			boxes.push_back(b2);
		}
	}
	
	std::cout << "Create an instance with " << nr_locations << " locations, " << nr_balls << " balls, and " << nr_colours << " colours." << std::endl;
	
	std::cout << "Creating all possible states..." << std::endl;
	if (mode == FACTORISED)
	{
		std::vector<const KnowledgeBase*> knowledge_bases;
		std::map<const Ball*, const Location*> empty_ball_to_location_mapping;
		std::map<const Ball*, const Colour*> empty_ball_to_colour_mapping;
		State basic_state("basic", empty_ball_to_location_mapping, empty_ball_to_colour_mapping);
		
		KnowledgeBase basis_kb("basis_kb");
		basis_kb.addState(basic_state);
		knowledge_bases.push_back(&basis_kb);
		
		// Create a new knowledge base for each ball.
		for (std::vector<const Ball*>::const_iterator ci = balls.begin(); ci != balls.end(); ++ci)
		{
			const Ball* ball = *ci;
			ss.str(std::string());
			ss << "kb_location_" << ball->name_;
			
			KnowledgeBase* kb_location = new KnowledgeBase(ss.str());
			basis_kb.addChild(*kb_location);
			knowledge_bases.push_back(kb_location);
			
			for (std::vector<const Location*>::const_iterator ci = locations.begin(); ci != locations.end(); ++ci)
			{
				const Location* location = *ci;
				std::map<const Ball*, const Location*> ball_location_mapping;
				std::map<const Ball*, const Colour*> ball_colour_mapping;
				ball_location_mapping[ball] = location;
				ss.str(std::string());
				ss << "skb_location_" << ball->name_ << "_" << location->name_;
				State* state = new State(ss.str(), ball_location_mapping, ball_colour_mapping);
				kb_location->addState(*state);
			}
			
			ss.str(std::string());
			ss << "kb_colour_" << ball->name_;
			
			KnowledgeBase* kb_colour = new KnowledgeBase(ss.str());
			basis_kb.addChild(*kb_colour);
			knowledge_bases.push_back(kb_colour);
			
			for (std::vector<const Colour*>::const_iterator ci = colours.begin(); ci != colours.end(); ++ci)
			{
				const Colour* colour = *ci;
				std::map<const Ball*, const Location*> ball_location_mapping;
				std::map<const Ball*, const Colour*> ball_colour_mapping;
				ball_colour_mapping[ball] = colour;
				ss.str(std::string());
				ss << "skb_colour_" << ball->name_ << "_" << colour->name_;
				State* state = new State(ss.str(), ball_location_mapping, ball_colour_mapping);
				kb_colour->addState(*state);
			}
		}
		
		std::cout << "Generate domain..." << std::endl;
		generateDomain("test_domain.pddl", basis_kb, knowledge_bases, locations, balls, colours, garbage_places, true);
		std::cout << "Generate problem..." << std::endl;
		generateProblem("test_problem.pddl", basis_kb, knowledge_bases, locations, balls, colours, garbage_places, true);
	}
	else if (mode == ORIGINAL)
	{
		// Enumerate all the possible states.
		std::vector<int> ball_locations(balls.size(), 0);
		std::vector<int> ball_colours(balls.size(), 0);
		
		bool done = false;
		unsigned int state_id = 0;
		std::stringstream ss;
		
		KnowledgeBase basis_kb("basis_kb");
		std::vector<const KnowledgeBase*> knowledge_bases;
		knowledge_bases.push_back(&basis_kb);
		
		while (!done)
		{
			std::map<const Ball*, const Location*> ball_location_mapping;
			std::map<const Ball*, const Colour*> ball_colour_mapping;
			for (unsigned int ball_id = 0; ball_id < balls.size(); ++ball_id)
			{
				const Ball* ball= balls[ball_id];
				ball_location_mapping[ball] = locations[ball_locations[ball_id]];
				ball_colour_mapping[ball] = colours[ball_colours[ball_id]];
			}
			
			ss.str(std::string());
			ss << "s" << state_id;
			State* state = new State(ss.str(), ball_location_mapping, ball_colour_mapping);
			basis_kb.addState(*state);
			++state_id;
			
			done = true;
			for (unsigned int i = 0; i < balls.size(); ++i)
			{
				if (ball_locations[i] + 1 != locations.size())
				{
					ball_locations[i] = ball_locations[i] + 1;
					done = false;
					break;
				}
				ball_locations[i] = 0;
			}
			
			if (done)
			{
				for (unsigned int i = 0; i < balls.size(); ++i)
				{
					if (ball_colours[i] + 1 != colours.size())
					{
						ball_colours[i] = ball_colours[i] + 1;
						done = false;
						break;
					}
					ball_colours[i] = 0;
				}
			}
		}
		generateDomain("test_domain.pddl", basis_kb, knowledge_bases, locations, balls, colours, garbage_places, false);
		std::cout << "Domain generated!" << std::endl;
		generateProblem("test_problem.pddl", basis_kb, knowledge_bases, locations, balls, colours, garbage_places, false);
		std::cout << "Problem generated!" << std::endl;
	}
	else
	{
		generateDomain("test_domain.pddl", locations, balls, colours, garbage_places);
		std::cout << "Domain generated!" << std::endl;
		generateProblem("test_problem.pddl", locations, balls, colours, garbage_places);
		std::cout << "Problem generated!" << std::endl;
	}
	
	return 0;
}
