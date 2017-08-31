#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <stdlib.h>
#include <cmath>
#include <boost/config/posix_features.hpp>

struct Cell
{
	enum CELL_STATUS { CLOSED, OPEN, UNKNOWN };
	Cell(const std::string& name, CELL_STATUS status, int x, int y)
		: name_(name), status_(status), x_(x), y_(y)
	{
		
	}
	
	std::string name_;
	CELL_STATUS status_;
	std::vector<Cell*> adjacent_cells_;
	int x_, y_;
};

struct State
{
	State(const std::string& state_name, std::vector<Cell*>& open_cell)
		: state_name_(state_name), cells_(open_cell)
	{
		
	}
	
	std::string state_name_;
	std::vector<Cell*> cells_;
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

void generateProblem(const std::string& file_name, KnowledgeBase& current_knowledge_base,  const std::vector<const KnowledgeBase*>& knowledge_base, const std::vector<Cell*>& grid, bool factorise)
{
	std::ofstream myfile;
	myfile.open(file_name.c_str());
	myfile << "(define (problem Keys-0)" << std::endl;
	myfile << "(:domain doors)" << std::endl;
	myfile << "(:objects" << std::endl;
	
	if (factorise)
	{
		myfile << "\tl0 - LEVEL" << std::endl;
		myfile << "\tl1 - LEVEL" << std::endl;
	}
	else
	{
		for (unsigned int i = 0; i < sqrt(grid.size()); ++i)
		{
			myfile << "\tl" << i << " - LEVEL" << std::endl;
		}
	}
	myfile << ")" << std::endl;
	myfile << std::endl;

	std::cout << "Start encoding..." << std::endl;
	myfile << "(:init" << std::endl;
	myfile << "\t(resolve-axioms)" << std::endl;
	myfile << "\t(lev l0)" << std::endl;
	
	if (factorise)
	{
		myfile << "\t(next l0 l1)" << std::endl;
	}
	else if (sqrt(grid.size()) > 1)
	{
		for (unsigned int i = 1; i < sqrt(grid.size()); ++i)
		{
			myfile << "\t(next l" << (i - 1) << " l" << i << ")" << std::endl;
		}
	}
	
	
	std::cout << "Encoding current knowledge base (" << current_knowledge_base.name_ << ")..." << std::endl;
	if (factorise)
	{
		myfile << "\t(current_kb " << current_knowledge_base.name_ << ")" << std::endl;
	}
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_base.begin(); ci != knowledge_base.end(); ++ci)
	{
		const KnowledgeBase* kb = *ci;
		std::cout << "Process knowledge base: " << kb->name_ << "(" << kb->states_->size() << ")" << std::endl;
		for (std::vector<const State*>::const_iterator ci = kb->states_->begin(); ci != kb->states_->end(); ++ci)
		{
			const State* state = *ci;
			std::cout << "Process state: " << state->state_name_ << std::endl;
			
			if (factorise)
			{
				myfile << "\t(part-of " << state->state_name_ << " " << kb->name_ << ")" << std::endl;
			}
			
			if (kb == &current_knowledge_base)
			{
				myfile << "\t(m " << state->state_name_ << ")" << std::endl;
				myfile << "\t(at " << grid[0]->name_ << " " << state->state_name_ << ")" << std::endl;
				
				for (std::vector<Cell*>::const_iterator ci = grid.begin(); ci != grid.end(); ++ci)
				{
					Cell* cell = *ci;
					if (cell->y_ % 2 == 0)
					{
						myfile << "\t(opened " << cell->name_ << " " << state->state_name_ << ")" << std::endl;
					}
				}
			}
			std::cout << "Check cell..." << std::endl;
			for (std::vector<Cell*>::const_iterator ci = state->cells_.begin(); ci != state->cells_.end(); ++ci)
			{
				myfile << "\t(opened " << (*ci)->name_ << " " << state->state_name_ << ")" << std::endl;
			}
			std::cout << "Process next state..." << std::endl;
		}
		
		for (std::vector<const KnowledgeBase*>::const_iterator ci = kb->children_.begin(); ci != kb->children_.end(); ++ci)
		{
			myfile << "\t(parent " << kb->name_ << " " << (*ci)->name_ << ")" << std::endl;
		}
	}
	
	for (std::vector<Cell*>::const_iterator ci = grid.begin(); ci != grid.end(); ++ci)
	{
		Cell* cell = *ci;
		for (std::vector<Cell*>::const_iterator ci = cell->adjacent_cells_.begin(); ci != cell->adjacent_cells_.end(); ++ci)
		{
			Cell* other_cell = *ci;
			if (other_cell->y_ < cell->y_)
			{
				continue;
			}
			myfile << "\t(adj " << cell->name_ << " " << (*ci)->name_ << ")" << std::endl;
		}
	}
	
	std::cout << "Encoding goals..." << std::endl;
	myfile << ")" << std::endl;
	myfile << "(:goal (and" << std::endl;
	if (factorise)
	{
		for (std::vector<const State*>::const_iterator ci = current_knowledge_base.states_->begin(); ci != current_knowledge_base.states_->end(); ++ci)
		{
			myfile << "\t(at " << grid[grid.size() - 1]->name_ << " " << (*ci)->state_name_ << ")" << std::endl;
		}
	}
	else
	{
		for (std::vector<const State*>::const_iterator ci = current_knowledge_base.states_->begin(); ci != current_knowledge_base.states_->end(); ++ci)
		{
			myfile << "\t(at " << grid[grid.size() - 1]->name_ << " " << (*ci)->state_name_ << ")" << std::endl;
		}
	}
	myfile << ")" << std::endl;
	myfile << ")" << std::endl;
	myfile << ")" << std::endl;
	myfile.close();
}

void generateDomain(const std::string& file_name, const KnowledgeBase& current_knowledge_base, const std::vector<const KnowledgeBase*>& knowledge_bases, const std::vector<Cell*>& grid, bool factorise)
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
	myfile << "(define (domain doors)" << std::endl;
	myfile << "(:requirements :typing :conditional-effects :negative-preconditions :disjunctive-preconditions)" << std::endl;
	myfile << std::endl;
	myfile << "(:types" << std::endl;
	myfile << "\tcell" << std::endl;
	myfile << "\tlevel" << std::endl;
	myfile << "\tstate" << std::endl;
	if (factorise)
	{
		myfile << "\tknowledgebase" << std::endl;
	}
	myfile << ")" << std::endl;
	myfile << std::endl;
	myfile << "(:predicates" << std::endl;
	myfile << "\t(at ?c - cell ?s - state)" << std::endl;
	myfile << "\t(Rat ?c - cell ?s - state)" << std::endl;
	myfile << "\t(opened ?c - cell ?s - state)" << std::endl;
	myfile << "\t(Ropened ?c - cell ?s - state)" << std::endl;
	myfile << "\t(adj ?c ?c2 - cell)" << std::endl;
	
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
	myfile << "\t; All the cells." << std::endl;
	
	for (std::vector<Cell*>::const_iterator ci = grid.begin(); ci != grid.end(); ++ci)
	{
		myfile << "\t" << (*ci)->name_ << " - cell" << std::endl;
	}

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
	myfile << ";; Action to move to a different cell." << std::endl;
	myfile << "(:action navigate" << std::endl;
	myfile << "\t:parameters (?from ?to - cell)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	myfile << "\t\t(adj ?from ?to)" << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_->begin(); ci != knowledge_base->states_->end(); ++ci)
		{
			myfile << "\t\t(Rat ?from " << (*ci)->state_name_ << ")" << std::endl;
			myfile << "\t\t(Ropened ?to " << (*ci)->state_name_ << ")" << std::endl;
		}
	}
	
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(when (m " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t(and" << std::endl;
		myfile << "\t\t\t\t(not (at ?from " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(at ?to " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t\t(not (Rat ?from " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(Rat ?to " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t)" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;

	/**
	 * SENSE action.
	 * FF is bugged and produces wrong plans...
	 */
	if (factorise)
	{
		for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
		{
			const KnowledgeBase* kb = *ci;
			if (kb == &current_knowledge_base) 
			{
				continue;
			}
			
			myfile << ";; Sense if a cell is open." << std::endl;
			myfile << "(:action sense_" << kb->name_ << std::endl;

			if (factorise)
			{
				//myfile << "\t:parameters (?c ?c2 - cell ?l ?l2 - level ?kb - knowledgebase)" << std::endl;
				myfile << "\t:parameters (?c ?c2 - cell ?l ?l2 - level)" << std::endl;
			}
			else
			{
				myfile << "\t:parameters (?c ?c2 - cell ?l ?l2 - level)" << std::endl;
			}

			myfile << "\t:precondition (and" << std::endl;
			myfile << "\t\t(not (resolve-axioms))" << std::endl;
			myfile << "\t\t(next ?l ?l2)" << std::endl;
			myfile << "\t\t(lev ?l)" << std::endl;
			myfile << "\t\t(adj ?c2 ?c)" << std::endl;
			//myfile << "\t\t(not (current_kb kb_row_1)" << std::endl;
			
			if (factorise)
			{
				//myfile << "\t\t(current_kb ?kb)" << std::endl;
				myfile << "\t\t(current_kb " << kb->name_ << ")" << std::endl;
			}
			myfile << std::endl;

			myfile << "\t\t;; For every state ?s" << std::endl;
			for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
			{
				myfile << "\t\t(Rat ?c2 " << (*ci)->state_name_ << ")" << std::endl;
				myfile << std::endl;
			}
			
			myfile << "\t\t;; This action is only applicable if there are world states where the outcome can be different." << std::endl;
			
			if (factorise)
			{
				//myfile << "\t\t(exists (?s - state) (and (m ?s) (opened ?c ?s) (part-of ?s ?kb)))" << std::endl;
				//myfile << "\t\t(exists (?s - state) (and (m ?s) (not (opened ?c ?s)) (part-of ?s ?kb)))" << std::endl;;
				myfile << "\t\t(exists (?s - state) (and (m ?s) (opened ?c ?s) (part-of ?s " << kb->name_ << ")))" << std::endl;
				myfile << "\t\t(exists (?s - state) (and (m ?s) (not (opened ?c ?s)) (part-of ?s " << kb->name_ << ")))" << std::endl;;
			}
			else
			{
				myfile << "\t\t(exists (?s - state) (and (m ?s) (opened ?c ?s)))" << std::endl;
				myfile << "\t\t(exists (?s - state) (and (m ?s) (not (opened ?c ?s))))" << std::endl;;
			}
			myfile << "\t)" << std::endl;
			myfile << "\t:effect (and" << std::endl;
			myfile << "\t\t(not (lev ?l))" << std::endl;
			myfile << "\t\t(lev ?l2)" << std::endl;
			myfile << std::endl;
			myfile << "\t\t;; For every state ?s" << std::endl;
			for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
			{
				myfile << "\t\t(when (and (m " << (*ci)->state_name_ << ") (not (opened ?c " << (*ci)->state_name_ << ")))" << std::endl;
				myfile << "\t\t\t(and (stack " << (*ci)->state_name_ << " ?l) (not (m " << (*ci)->state_name_ << ")))" << std::endl;
				myfile << "\t\t)" << std::endl;
			}
			myfile << "\t\t(resolve-axioms)" << std::endl;
			myfile << "\t)" << std::endl;
			myfile << ")" << std::endl;
			myfile << std::endl;
		}
	}
	else
	{
		myfile << ";; Sense if a cell is open." << std::endl;
		myfile << "(:action sense" << std::endl;
		myfile << "\t:parameters (?c ?c2 - cell ?l ?l2 - level)" << std::endl;

		myfile << "\t:precondition (and" << std::endl;
		myfile << "\t\t(not (resolve-axioms))" << std::endl;
		myfile << "\t\t(next ?l ?l2)" << std::endl;
		myfile << "\t\t(lev ?l)" << std::endl;
		myfile << "\t\t(adj ?c2 ?c)" << std::endl;
		myfile << std::endl;

		myfile << "\t\t;; For every state ?s" << std::endl;
		for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
		{
			myfile << "\t\t(Rat ?c2 " << (*ci)->state_name_ << ")" << std::endl;
			myfile << std::endl;
		}
		
		myfile << "\t\t;; This action is only applicable if there are world states where the outcome can be different." << std::endl;
		myfile << "\t\t(exists (?s - state) (and (m ?s) (opened ?c ?s)))" << std::endl;
		myfile << "\t\t(exists (?s - state) (and (m ?s) (not (opened ?c ?s))))" << std::endl;;

		myfile << "\t)" << std::endl;
		myfile << "\t:effect (and" << std::endl;
		myfile << "\t\t(not (lev ?l))" << std::endl;
		myfile << "\t\t(lev ?l2)" << std::endl;
		myfile << std::endl;
		myfile << "\t\t;; For every state ?s" << std::endl;
		for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
		{
			myfile << "\t\t(when (and (m " << (*ci)->state_name_ << ") (not (opened ?c " << (*ci)->state_name_ << ")))" << std::endl;
			myfile << "\t\t\t(and (stack " << (*ci)->state_name_ << " ?l) (not (m " << (*ci)->state_name_ << ")))" << std::endl;
			myfile << "\t\t)" << std::endl;
		}
		myfile << "\t\t(resolve-axioms)" << std::endl;
		myfile << "\t)" << std::endl;
		myfile << ")" << std::endl;
		myfile << std::endl;
	}
	
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
		
		// The location and whether cells are open.
		for (std::vector<Cell*>::const_iterator ci = grid.begin(); ci != grid.end(); ++ci)
		{
			// Locality
			myfile << "\t\t(when (or (at " << (*ci)->name_ << " " << state->state_name_ << ") (not (m " << state->state_name_ << ")))" << std::endl;
			myfile << "\t\t\t(Rat " << (*ci)->name_ << " " << state->state_name_ << ")" << std::endl;
			myfile << "\t\t)" << std::endl;
			
			myfile << "\t\t(when (and (not (at " << (*ci)->name_ << " " << state->state_name_ << ")) (m " << state->state_name_ << "))" << std::endl;
			myfile << "\t\t\t(not (Rat " << (*ci)->name_ << " " << state->state_name_ << "))" << std::endl;
			myfile << "\t\t)" << std::endl;
			
			// Openess.
			myfile << "\t\t(when (or (opened " << (*ci)->name_ << " " << state->state_name_ << ") (not (m " << state->state_name_ << ")))" << std::endl;
			myfile << "\t\t\t(Ropened " << (*ci)->name_ << " " << state->state_name_ << ")" << std::endl;
			myfile << "\t\t)" << std::endl;
			
			myfile << "\t\t(when (and (not (opened " << (*ci)->name_ << " " << state->state_name_ << ")) (m " << state->state_name_ << "))" << std::endl;
			myfile << "\t\t\t(not (Ropened " << (*ci)->name_ << " " << state->state_name_ << "))" << std::endl;
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
				for (std::vector<Cell*>::const_iterator ci = grid.begin(); ci != grid.end(); ++ci)
				{
					const Cell* cell = *ci;
					// Copy the states of where we are.
					myfile << "\t\t(when (and (part-of " << state->state_name_ << " ?old_kb) (at " << cell->name_ << " " << state->state_name_ << ") (part-of " << state2->state_name_ << " ?new_kb))" << std::endl;
					myfile << "\t\t\t(and " << std::endl;
					myfile << "\t\t\t\t(not (Rat " << cell->name_ << " " << state->state_name_ << "))" << std::endl;
					myfile << "\t\t\t\t(not (at " << cell->name_ << " " << state->state_name_ << "))" << std::endl;
					myfile << "\t\t\t\t(Rat " << cell->name_ << " " << state2->state_name_ << ")" << std::endl;
					myfile << "\t\t\t\t(at " << cell->name_ << " " << state2->state_name_ << ")" << std::endl;
					myfile << "\t\t\t)" << std::endl;
					myfile << "\t\t)" << std::endl;
					
					// We dont have to copy the openess of the cells as they are distinct.
					myfile << "\t\t(when (and (part-of " << state->state_name_ << " ?old_kb) (opened " << cell->name_ << " " << state->state_name_ << ") (part-of " << state2->state_name_ << " ?new_kb))" << std::endl;
					myfile << "\t\t\t(and " << std::endl;
					myfile << "\t\t\t\t(not (Ropened " << cell->name_ << " " << state->state_name_ << "))" << std::endl;
					myfile << "\t\t\t\t(not (opened " << cell->name_ << " " << state->state_name_ << "))" << std::endl;
					myfile << "\t\t\t\t(Ropened " << cell->name_ << " " << state2->state_name_ << ")" << std::endl;
					myfile << "\t\t\t\t(opened " << cell->name_ << " " << state2->state_name_ << ")" << std::endl;
					myfile << "\t\t\t)" << std::endl;
					myfile << "\t\t)" << std::endl;
				}
			}
		}
		myfile << "\t)" << std::endl;
		myfile << ")" << std::endl;

		myfile << ";; Move 'up' into the knowledge base." << std::endl;;
		myfile << "(:action shed_knowledge" << std::endl;
		myfile << "\t:parameters (?old_kb ?new_kb - knowledgebase ?c - cell)" << std::endl;
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
			
			// Force the planner to have each active state at the same location.
			myfile << "\t\t(or " << std::endl;
			myfile << "\t\t\t(not (part-of " << (*ci)->state_name_ << " ?old_kb))" << std::endl;
			myfile << "\t\t\t(at ?c " << (*ci)->state_name_ << ")" << std::endl;
			myfile << "\t\t)" << std::endl;
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
			
		for (std::vector<Cell*>::const_iterator ci = grid.begin(); ci != grid.end(); ++ci)
		{
			const Cell* cell = *ci;
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
					myfile << "\t\t\t\t\t\t(at " << cell->name_ << " " << state2->state_name_ << ")" << std::endl;
					myfile << "\t\t\t\t\t)" << std::endl;
				}
				myfile << "\t\t\t\t\t(part-of " << state->state_name_ << " ?new_kb)" << std::endl;
				myfile << "\t\t\t)" << std::endl;

				myfile << "\t\t\t;; Conditional effects" << std::endl;
				myfile << "\t\t\t(and " << std::endl;
				for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
				{
					const State* state2 = *ci;
					myfile << "\t\t\t\t(not (at " << cell->name_ << " " << state2->state_name_ << "))" << std::endl;
				}
				myfile << "\t\t\t\t(at " << cell->name_ << " " << state->state_name_ << ")" << std::endl;
				myfile << "\t\t\t)" << std::endl;
				myfile << "\t\t)" << std::endl;
				
				// Deal with the openess of the cells.
				myfile << "\t\t(when (and " << std::endl;
				myfile << "\t\t\t\t;; For every state ?s, ?s2" << std::endl;
			
				for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
				{
					const State* state2 = *ci;
					myfile << "\t\t\t\t\t(or " << std::endl;
					myfile << "\t\t\t\t\t\t(not (part-of " << state2->state_name_ << " ?old_kb))" << std::endl;
					myfile << "\t\t\t\t\t\t(opened " << cell->name_ << " " << state2->state_name_ << ")" << std::endl;
					myfile << "\t\t\t\t\t)" << std::endl;
				}
				myfile << "\t\t\t\t\t(part-of " << state->state_name_ << " ?new_kb)" << std::endl;
				myfile << "\t\t\t)" << std::endl;

				myfile << "\t\t\t;; Conditional effects" << std::endl;
				myfile << "\t\t\t(and " << std::endl;
				for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
				{
					const State* state2 = *ci;
					myfile << "\t\t\t\t(not (opened " << cell->name_ << " " << state2->state_name_ << "))" << std::endl;
				}
				myfile << "\t\t\t\t(opened " << cell->name_ << " " << state->state_name_ << ")" << std::endl;
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
	if (argc < 2)
	{
		std::cout << "Usage: <number rows/colums> {-f}" << std::endl;
		return -1;
	}
	
	unsigned int nr_locations = ::atoi(argv[1]);
	
	bool factorise = argc == 3;
	std::cout << "Creating a slding door problem with " << nr_locations << " cells.";
	if (factorise) std::cout << "[FACTORISE]";
	std::cout << std::endl;
	
	std::vector<Cell*> cells;
	
	// Create the grid.
	std::vector<Cell*> grid;
	std::stringstream ss;
	for (unsigned int y = 0; y < nr_locations; ++y)
	{
		for (unsigned int x = 0; x < nr_locations; ++x)
		{
			ss.str(std::string());
			ss << "cell_" << x << "_" << y;
			bool is_open = y % 2 == 0;
			Cell* new_cell = new Cell(ss.str(), is_open ? Cell::OPEN : Cell::UNKNOWN, x, y);
			grid.push_back(new_cell);
			
			// Link this cell up with its neighbours.
			if (x > 0)
			{
				Cell* other_cell = grid[(x - 1) + y * nr_locations];
				new_cell->adjacent_cells_.push_back(other_cell);
				other_cell->adjacent_cells_.push_back(new_cell);
			}
			if (y > 0)
			{
				Cell* other_cell = grid[x + (y - 1) * nr_locations];
				new_cell->adjacent_cells_.push_back(other_cell);
				other_cell->adjacent_cells_.push_back(new_cell);
			}
		}
	}
	std::cout << "Grid created (" << grid.size() << ")." << std::endl;
	
	if (factorise)
	{
		std::vector<const KnowledgeBase*> knowledge_bases;
		State basic_state("basic", cells);
		
		std::vector<const State*> basis_kb_states;
		basis_kb_states.push_back(&basic_state);
		
		KnowledgeBase basis_kb("basis_kb", basis_kb_states);
		
		if (factorise)
		{
			knowledge_bases.push_back(&basis_kb);
		}
		
		for (unsigned int row = 1; row < nr_locations; row += 2)
		{
			ss.str(std::string());
			ss << "kb_row_" << row;
			std::vector<const State*>* kb_states = new std::vector<const State*>();
			KnowledgeBase* kb = new KnowledgeBase(ss.str(), *kb_states);
			basis_kb.addChild(*kb);
			knowledge_bases.push_back(kb);
			
			for (unsigned int x = 0; x < nr_locations; ++x)
			{
				Cell* cell = grid[row * nr_locations + x];
				std::vector<Cell*> open_cells;
				open_cells.push_back(cell);
				ss.str(std::string());
				ss << "s" << cell->name_;
				
				State* state = new State(ss.str(), open_cells);
				kb_states->push_back(state);
			}
		}
		
				
		std::cout << "Generate domain..." << std::endl;
		generateDomain("test_domain.pddl", basis_kb, knowledge_bases, grid, factorise);
		std::cout << "Generate problem..." << std::endl;
		generateProblem("test_problem.pddl", basis_kb, knowledge_bases, grid, factorise);
	}
	else
	{
		std::vector<const State*> basis_kb_states;
		
		std::vector<int> open_cell_x(nr_locations / 2, 0);
		bool done = false;
		unsigned int state_id = 0;
		std::stringstream ss;
		while (!done)
		{
			std::vector<Cell*> open_cells;
			for (unsigned int row = 0; row < nr_locations / 2; ++row)
			{
				Cell* cell = grid[(row * 2 + 1) * nr_locations + open_cell_x[row]];
				open_cells.push_back(cell);
			}
			ss.str(std::string());
			ss << "s" << state_id;
			State* state = new State(ss.str(), open_cells);
			basis_kb_states.push_back(state);
			++state_id;
			
			done = true;
			for (unsigned int i = 0; i < open_cell_x.size(); ++i)
			{
				if (open_cell_x[i] + 1 != nr_locations)
				{
					open_cell_x[i] = open_cell_x[i] + 1;
					done = false;
					break;
				}
				open_cell_x[i] = 0;
			}
		}
		
		std::cout << "Created " << basis_kb_states.size() << " states." << std::endl;
		
		KnowledgeBase basis_kb("basis_kb", basis_kb_states);
		std::vector<const KnowledgeBase*> knowledge_bases;
		knowledge_bases.push_back(&basis_kb);
		
		std::cout << "Generate domain..." << std::endl;
		generateDomain("test_domain.pddl", basis_kb, knowledge_bases, grid, factorise);
		std::cout << "Generate problem..." << std::endl;
		generateProblem("test_problem.pddl", basis_kb, knowledge_bases, grid, factorise);
	}

	
	return 0;
}
