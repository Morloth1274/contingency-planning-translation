#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <stdlib.h>
#include <map>
#include <boost/concept_check.hpp>

struct Package
{
	Package(const std::string& wp_name)
		: name_(wp_name)
	{
		
	}
	
	std::string name_;
};

struct Bomb
{
	Bomb(const std::string& key_name)
		: name_(key_name)
	{
		
	}
	
	std::string name_;
};

struct State
{
	State(const std::string& state_name, const std::map<const Bomb*, const Package*>& mapping)
		: state_name_(state_name), mapping_(mapping)
	{
		
	}
	
	std::string state_name_;
	std::map<const Bomb*, const Package*> mapping_;
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
		//state.kb_ = this;
		states_.push_back(&state);
	}
	
	std::string name_;
	std::vector<const State*> states_;
	
	std::vector<const KnowledgeBase*> children_;
};

void generateProblem(const std::string& file_name, const KnowledgeBase& current_knowledge_base, const std::vector<const KnowledgeBase*>& knowledge_base, const std::vector<const Package*>& packages, const std::vector<const Bomb*>& bombs, bool factorise)
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
		for (unsigned int key_nr = 0; key_nr < bombs.size() + 1; ++key_nr)
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
		for (unsigned int key_nr = 1; key_nr < bombs.size() + 1; ++key_nr)
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
		
			for (std::map<const Bomb*, const Package*>::const_iterator ci = state->mapping_.begin(); ci != state->mapping_.end(); ++ci)
			{
				myfile << "\t(in " << (*ci).second->name_ << " " << (*ci).first->name_ << " " << state->state_name_ << ")" << std::endl;
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
		for (std::vector<const Bomb*>::const_iterator ci = bombs.begin(); ci != bombs.end(); ++ci)
		{
			const Bomb* bomb = *ci;
			for (std::vector<const State*>::const_iterator ci = current_knowledge_base.states_.begin(); ci != current_knowledge_base.states_.end(); ++ci)
			{
				myfile << "\t(defused " << bomb->name_ << " " << (*ci)->state_name_ << ")" << std::endl;
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
				for (std::vector<const Bomb*>::const_iterator ci = bombs.begin(); ci != bombs.end(); ++ci)
				{
					const Bomb* bomb = *ci;
					myfile << "\t(defused " << bomb->name_ << " " << state->state_name_ << ")" << std::endl;
				}
			}
		}
	}
	myfile << ")" << std::endl;
	myfile << ")" << std::endl;
	myfile << ")" << std::endl;
	myfile.close();
}

void generateDomain(const std::string& file_name, const KnowledgeBase& current_knowledge_base, const std::vector<const KnowledgeBase*>& knowledge_bases, const std::vector<const Package*>& packages, const std::vector<const Bomb*>& bombs, bool factorise)
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
	myfile << "\tpackage bomb toilet" << std::endl;
	myfile << "\tlevel" << std::endl;
	myfile << "\tstate" << std::endl;
	myfile << "\tknowledgebase" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	myfile << "(:predicates" << std::endl;
	myfile << "\t(in ?p - package ?b - bomb ?s - state)" << std::endl;
	myfile << "\t(Rin ?p - package ?b - bomb ?s - state)" << std::endl;
	myfile << "\t(defused ?b - bomb ?s - state)" << std::endl;
	myfile << "\t(Rdefused ?b - bomb ?s - state)" << std::endl;
	myfile << "\t(nclog ?t - toilet ?s - state)" << std::endl;
	myfile << "\t(Rnclog ?t - toilet ?s - state)" << std::endl;
	
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
	myfile << "\t; All the packages." << std::endl;
	
	for (std::vector<const Package*>::const_iterator ci = packages.begin(); ci != packages.end(); ++ci)
	{
		myfile << "\t" << (*ci)->name_ << " - package" << std::endl;
	}

	myfile << "\t; The bombs." << std::endl;
	for (std::vector<const Bomb*>::const_iterator ci = bombs.begin(); ci != bombs.end(); ++ci)
	{
		
		myfile << "\t" << (*ci)->name_ << " - bomb" << std::endl;
	}

	myfile << "\t; The toilet." << std::endl;
	myfile << "\ttoilet - toilet" << std::endl;

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
	myfile << "(:action dunk" << std::endl;
	myfile << "\t:parameters (?p - package ?b - bomb ?t - toilet)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_.begin(); ci != knowledge_base->states_.end(); ++ci)
		{
			myfile << "\t\t(Rnclog ?t " << (*ci)->state_name_ << ")" << std::endl;
			myfile << "\t\t(Rin ?p ?b " << (*ci)->state_name_ << ")" << std::endl;
		}
	}
	
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(when (m " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t(and" << std::endl;
		myfile << "\t\t\t\t(not (nclog ?t " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(not (Rnclog ?t " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(defused ?b " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t\t(Rdefused ?b " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t)" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;

	/**
	 * PICKUP action.
	 */
	myfile << ";; Flush a toilet." << std::endl;
	myfile << "(:action flush" << std::endl;
	myfile << "\t:parameters (?t - toilet)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t;; For every state " << (*ci)->state_name_ << "" << std::endl;
		myfile << "\t\t(when (m " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t(and" << std::endl;
		myfile << "\t\t\t\t(nclog ?t " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t\t(Rnclog ?t " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t)" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * SENSE action.
	 */
	myfile << ";; Sense for the bomb." << std::endl;
	myfile << "(:action sense" << std::endl;

	if (factorise)
	{
		myfile << "\t:parameters (?p - package ?b - bomb ?l ?l2 - level ?kb - knowledgebase)" << std::endl;
	}
	else
	{
		myfile << "\t:parameters (?p - package ?b - bomb ?l ?l2 - level)" << std::endl;
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

	myfile << "\t\t;; This action is only applicable if there are world states where the outcome can be different." << std::endl;
	
	if (factorise)
	{
		myfile << "\t\t(exists (?s - state) (and (m ?s) (in ?p ?b ?s) (part-of ?s ?kb)))" << std::endl;
		myfile << "\t\t(exists (?s - state) (and (m ?s) (not (in ?p ?b ?s)) (part-of ?s ?kb)))" << std::endl;;
	}
	else
	{
		myfile << "\t\t(exists (?s - state) (and (m ?s) (in ?p ?b ?s)))" << std::endl;
		myfile << "\t\t(exists (?s - state) (and (m ?s) (not (in ?p ?b ?s))))" << std::endl;;
	}
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t(not (lev ?l))" << std::endl;
	myfile << "\t\t(lev ?l2)" << std::endl;
	myfile << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(when (and (m " << (*ci)->state_name_ << ") (not (in ?p ?b " << (*ci)->state_name_ << ")))" << std::endl;
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
		
		// Location of the bombs.
		for (std::vector<const Bomb*>::const_iterator ci = bombs.begin(); ci != bombs.end(); ++ci)
		{
			const Bomb* bomb = *ci;
			myfile << "\t\t(when (or (defused " << bomb->name_ << " " << state->state_name_ << ") (not (m " << state->state_name_ << ")))" << std::endl;
			myfile << "\t\t\t(Rdefused " << bomb->name_ << " " << state->state_name_ << ")" << std::endl;
			myfile << "\t\t)" << std::endl;
			
			myfile << "\t\t(when (and (not (defused " << bomb->name_ << " " << state->state_name_ << ")) (m " << state->state_name_ << "))" << std::endl;
			myfile << "\t\t\t(not (Rdefused " << bomb->name_ << " " << state->state_name_ << "))" << std::endl;
			myfile << "\t\t)" << std::endl;
			
			for (std::vector<const Package*>::const_iterator ci = packages.begin(); ci != packages.end(); ++ci)
			{
				const Package* package = *ci;
				myfile << "\t\t(when (or (in " << package->name_ << " " << bomb->name_<< " " << state->state_name_ << ") (not (m " << state->state_name_ << ")))" << std::endl;
				myfile << "\t\t\t(Rin " << package->name_ << " " << bomb->name_ << " " << state->state_name_ << ")" << std::endl;
				myfile << "\t\t)" << std::endl;
				
				myfile << "\t\t(when (and (not (in " << package->name_ << " " << bomb->name_<< " " << state->state_name_ << ")) (m " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t\t(not (Rin " << package->name_ << " " << bomb->name_ << " " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t)" << std::endl;
			}
		}
		
		// State of the toilet.
		myfile << "\t\t(when (or (nclog toilet " << state->state_name_ << ") (not (m " << state->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t(Rnclog toilet " << state->state_name_ << ")" << std::endl;
		myfile << "\t\t)" << std::endl;
		
		myfile << "\t\t(when (and (not (nclog toilet " << state->state_name_ << ")) (m " << state->state_name_ << "))" << std::endl;
		myfile << "\t\t\t(not (Rnclog toilet " << state->state_name_ << "))" << std::endl;
		myfile << "\t\t)" << std::endl;
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
				for (std::vector<const Bomb*>::const_iterator ci = bombs.begin(); ci != bombs.end(); ++ci)
				{
					const Bomb* bomb = *ci;
					// Copy the states of the bombs.
					myfile << "\t\t(when (and (part-of " << state->state_name_ << " ?old_kb) (defused " << bomb->name_ << " " << state->state_name_ << ") (part-of " << state2->state_name_ << " ?new_kb))" << std::endl;
					myfile << "\t\t\t(and " << std::endl;
					myfile << "\t\t\t\t(not (Rdefused " << bomb->name_ << " " << state->state_name_ << "))" << std::endl;
					myfile << "\t\t\t\t(not (defused " << bomb->name_ << " " << state->state_name_ << "))" << std::endl;
					myfile << "\t\t\t\t(Rdefused " << bomb->name_ << " " << state2->state_name_ << ")" << std::endl;
					myfile << "\t\t\t\t(defused " << bomb->name_ << " " << state2->state_name_ << ")" << std::endl;
					myfile << "\t\t\t)" << std::endl;
					myfile << "\t\t)" << std::endl;
					
					// Copy the states of the packages.
					for (std::vector<const Package*>::const_iterator ci = packages.begin(); ci != packages.end(); ++ci)
					{
						myfile << "\t\t(when (and (part-of " << state->state_name_ << " ?old_kb) (in " << (*ci)->name_ << " " << bomb->name_ << " " << state->state_name_ << ") (part-of " << state2->state_name_ << " ?new_kb))" << std::endl;
						myfile << "\t\t\t(and " << std::endl;
						myfile << "\t\t\t\t(not (Rin " << (*ci)->name_ << " " << bomb->name_ << " " << state->state_name_ << "))" << std::endl;
						myfile << "\t\t\t\t(not (in " << (*ci)->name_ << " " << bomb->name_ << " " << state->state_name_ << "))" << std::endl;
						myfile << "\t\t\t\t(Rin " << (*ci)->name_ << " " << bomb->name_ << " " << state2->state_name_ << ")" << std::endl;
						myfile << "\t\t\t\t(in " << (*ci)->name_ << " " << bomb->name_ << " " << state2->state_name_ << ")" << std::endl;
						myfile << "\t\t\t)" << std::endl;
						myfile << "\t\t)" << std::endl;
					}
				}
				
				// State of the toilet.
				myfile << "\t\t(when (and (part-of " << state->state_name_ << " ?old_kb) (nclog toilet " << state->state_name_ << ") (part-of " << state2->state_name_ << " ?new_kb))" << std::endl;
				myfile << "\t\t\t(and " << std::endl;
				myfile << "\t\t\t\t(not (Rnclog toilet " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t\t\t(not (nclog toilet " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t\t\t(Rnclog toilet " << state2->state_name_ << ")" << std::endl;
				myfile << "\t\t\t\t(nclog toilet " << state2->state_name_ << ")" << std::endl;
				myfile << "\t\t\t)" << std::endl;
				myfile << "\t\t)" << std::endl;
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
		
		// Make sure the toilets are in the same state.
		myfile << "\t\t(or";
		myfile << "\t\t\t(and";
		for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
		{
			// Make sure the state of the toilets are the same.
			myfile << "\t\t\t\t(or " << std::endl;
			myfile << "\t\t\t\t\t(not (part-of " << (*ci)->state_name_ << " ?old_kb))" << std::endl;
			myfile << "\t\t\t\t\t(nclog toilet " << (*ci)->state_name_ << ")" << std::endl;
			myfile << "\t\t\t\t)" << std::endl;
		}
		myfile << "\t\t\t)" << std::endl;
		myfile << "\t\t\t(and";
		for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
		{
			// Make sure the state of the toilets are the same.
			myfile << "\t\t\t\t(or " << std::endl;
			myfile << "\t\t\t\t\t(not (part-of " << (*ci)->state_name_ << " ?old_kb))" << std::endl;
			myfile << "\t\t\t\t\t(not (nclog toilet " << (*ci)->state_name_ << "))" << std::endl;
			myfile << "\t\t\t\t)" << std::endl;
		}
		myfile << "\t\t\t)" << std::endl;
		myfile << "\t\t)" << std::endl;
		
		// Make sure the bombs are in the same state.
		for (std::vector<const Bomb*>::const_iterator ci = bombs.begin(); ci != bombs.end(); ++ci)
		{
			const Bomb* bomb = *ci;
			myfile << "\t\t(or";
			myfile << "\t\t\t(and";
			for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
			{
				// Make sure the state of the toilets are the same.
				myfile << "\t\t\t\t(or " << std::endl;
				myfile << "\t\t\t\t\t(not (part-of " << (*ci)->state_name_ << " ?old_kb))" << std::endl;
				myfile << "\t\t\t\t\t(defused " << bomb->name_ << "  " << (*ci)->state_name_ << ")" << std::endl;
				myfile << "\t\t\t\t)" << std::endl;
			}
			myfile << "\t\t\t)" << std::endl;
			myfile << "\t\t\t(and";
			for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
			{
				// Make sure the state of the toilets are the same.
				myfile << "\t\t\t\t(or " << std::endl;
				myfile << "\t\t\t\t\t(not (part-of " << (*ci)->state_name_ << " ?old_kb))" << std::endl;
				myfile << "\t\t\t\t\t(not (defused " << bomb->name_ << "  " << (*ci)->state_name_ << "))" << std::endl;
				myfile << "\t\t\t\t)" << std::endl;
			}
			myfile << "\t\t\t)" << std::endl;
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
			
		for (std::vector<const Bomb*>::const_iterator ci = bombs.begin(); ci != bombs.end(); ++ci)
		{
			const Bomb* bomb = *ci;
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
					myfile << "\t\t\t\t\t\t(defused " << bomb->name_ << " " << state2->state_name_ << ")" << std::endl;
					myfile << "\t\t\t\t\t)" << std::endl;
					
				}
				myfile << "\t\t\t\t\t(part-of " << state->state_name_ << " ?new_kb)" << std::endl;
				myfile << "\t\t\t)" << std::endl;

				myfile << "\t\t\t;; Conditional effects" << std::endl;
				myfile << "\t\t\t(and " << std::endl;
				for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
				{
					const State* state2 = *ci;
					myfile << "\t\t\t\t(not (defused " << bomb->name_ << " " << state2->state_name_ << "))" << std::endl;
				}
				myfile << "\t\t\t\t(defused " << bomb->name_ << " " << state->state_name_ << ")" << std::endl;
				myfile << "\t\t\t)" << std::endl;
				myfile << "\t\t)" << std::endl;
			}
			
			// Deal with the location of the bomb.
			for (std::vector<const Package*>::const_iterator ci = packages.begin(); ci != packages.end(); ++ci)
			{
				const Package* package = *ci;
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
						myfile << "\t\t\t\t\t\t(in " << package->name_ << " " << bomb->name_ << " " << state2->state_name_ << ")" << std::endl;
						myfile << "\t\t\t\t\t)" << std::endl;
					}

					myfile << "\t\t\t\t(part-of " << state->state_name_ << " ?new_kb)" << std::endl;
					myfile << "\t\t\t)" << std::endl;

					myfile << "\t\t\t;; Conditional effects" << std::endl;
					myfile << "\t\t\t(and " << std::endl;
					for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
					{
						const State* state2 = *ci;
						myfile << "\t\t\t\t(not (in " << package->name_ << " " << bomb->name_ << " " << state2->state_name_ << "))" << std::endl;
					}
					myfile << "\t\t\t\t(in " << package->name_ << " " << bomb->name_ << " " << state->state_name_ << ")" << std::endl;
					myfile << "\t\t\t)" << std::endl;
					myfile << "\t\t)" << std::endl;
				}
			}
		}
		
		// Check the state of the toilet.
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
				myfile << "\t\t\t\t\t\t(nclog toilet " << state2->state_name_ << ")" << std::endl;
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
				myfile << "\t\t\t\t(not (nclog toilet " << state2->state_name_ << "))" << std::endl;
			}
			myfile << "\t\t\t\t(nclog toilet " << state->state_name_ << ")" << std::endl;
			
			myfile << "\t\t\t)" << std::endl;
			myfile << "\t\t)" << std::endl;
		}
		myfile << "\t)" << std::endl;
		myfile << ")" << std::endl;
	}
	myfile << ")" << std::endl;
	myfile.close();
}


void generateProblem(const std::string& file_name, const std::vector<const Package*>& packages, const std::vector<const Bomb*>& bombs)
{
	std::ofstream myfile;
	myfile.open(file_name.c_str());
	myfile << "(define (problem Keys-0)" << std::endl;
	myfile << "(:domain find_key)" << std::endl;
	myfile << "(:objects" << std::endl;
	for (std::vector<const Package*>::const_iterator ci = packages.begin(); ci != packages.end(); ++ci)
	{
		myfile << "\t" << (*ci)->name_ << " - package" << std::endl;
	}

	myfile << "\t; The bombs." << std::endl;
	for (std::vector<const Bomb*>::const_iterator ci = bombs.begin(); ci != bombs.end(); ++ci)
	{
		
		myfile << "\t" << (*ci)->name_ << " - bomb" << std::endl;
	}

	myfile << "\t; The toilet." << std::endl;
	myfile << "\ttoilet0 - toilet" << std::endl;

	myfile << ")" << std::endl;
	myfile << std::endl;

	myfile << "(:init" << std::endl;
	
	for (std::vector<const Bomb*>::const_iterator ci = bombs.begin(); ci != bombs.end(); ++ci)
	{
		const Bomb* bomb = *ci;
		if (packages.size() > 1)
		{
			myfile << "(oneof " << std::endl;
		}
		for (std::vector<const Package*>::const_iterator ci = packages.begin(); ci != packages.end(); ++ci)
		{
			const Package* package = *ci;
			myfile << "\t(in " << package->name_ << " " << bomb->name_ << ")" << std::endl;
		}
		if (packages.size() > 1)
		{
			myfile << ")" << std::endl;
		}
	}
	myfile << ")" << std::endl;
	myfile << "(:goal (and" << std::endl;
	for (std::vector<const Bomb*>::const_iterator ci = bombs.begin(); ci != bombs.end(); ++ci)
	{
		const Bomb* bomb = *ci;
		myfile << "\t(defused " << bomb->name_ << ")" << std::endl;
	}
	myfile << ")" << std::endl;
	myfile << ")" << std::endl;
	myfile << ")" << std::endl;
	myfile.close();
}

void generateDomain(const std::string& file_name, const std::vector<const Package*>& packages, const std::vector<const Bomb*>& bombs)
{
	std::ofstream myfile;
	myfile.open (file_name.c_str());
	myfile << "(define (domain find_key)" << std::endl;
	myfile << "(:requirements :typing)" << std::endl;
	myfile << std::endl;
	myfile << "(:types" << std::endl;
	myfile << "\tpackage bomb toilet" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	myfile << "(:predicates" << std::endl;
	myfile << "\t(in ?p - package ?b - bomb)" << std::endl;
	myfile << "\t(defused ?b - bomb)" << std::endl;
	myfile << "\t(nclog ?t - toilet)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * NAVIGATE ACTION.
	 */
	myfile << ";; Action to dunk the package." << std::endl;
	myfile << "(:action dunk" << std::endl;
	myfile << "\t:parameters (?p - package ?b - bomb ?t - toilet)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(nclog ?t)" << std::endl;
	myfile << "\t\t(in ?p ?b)" << std::endl;
	
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t(not (nclog ?t))" << std::endl;
	myfile << "\t\t(defused ?b)" << std::endl;
	
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;

	/**
	 * PICKUP action.
	 */
	myfile << ";; Flush a toilet." << std::endl;
	myfile << "(:action flush" << std::endl;
	myfile << "\t:parameters (?t - toilet)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t(nclog ?t)" << std::endl;
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * SENSE action.
	 */
	myfile << ";; Sense for the bomb." << std::endl;
	myfile << "(:action sense" << std::endl;

	myfile << "\t:parameters (?p - package ?b - bomb)" << std::endl;
	myfile << "\t:observe (in ?p ?b)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	myfile << ")" << std::endl;
	myfile.close();
}

int main(int argc, char **argv)
{
	std::vector<const Bomb*> bombs;
	std::vector<const Package*> packages;
	
	if (argc < 3)
	{
		std::cout << "Usage: <number of bombs> <number of packages> {-f}" << std::endl;
		return -1;
	}
	
	unsigned int nr_bombs = ::atoi(argv[1]);
	unsigned int nr_packages = ::atoi(argv[2]);
	
	
	enum MODE { ORIGINAL, FACTORISED, PRP};
	MODE mode = ORIGINAL;
	
	if (argc == 3)
	{
		mode = ORIGINAL;
	}
	else if (std::string(argv[3]) == "-f")
	{
		mode = FACTORISED;
	}
	else if (std::string(argv[3]) == "-p")
	{
		mode = PRP;
	}
	else
	{
		std::cerr << "Unknown option " << argv[3] << std::endl;
		exit(-1);
	}
	
	std::stringstream ss;
	
	std::vector<const KnowledgeBase*> knowledge_bases;
	std::map<const Bomb*, const Package*> empty_bomb_package_mapping;
	State basic_state("basic", empty_bomb_package_mapping);
	
	KnowledgeBase basis_kb("basis_kb");
	basis_kb.addState(basic_state);
	
	for (unsigned int i = 0; i < nr_bombs; ++i)
	{
		ss.str(std::string());
		ss << "bomb_" << i;
		Bomb* b = new Bomb(ss.str());
		bombs.push_back(b);
	}
	
	for (unsigned int i = 0; i < nr_packages; ++i)
	{
		ss.str(std::string());
		ss << "package_" << i;
		Package* p = new Package(ss.str());
		packages.push_back(p);
	}
	
	if (mode == FACTORISED)
	{
		knowledge_bases.push_back(&basis_kb);
	}
	
	std::cout << "Create an instance with " << nr_bombs << " bombs, the number of packages " << nr_packages << "." << std::endl;
	
	std::cout << "Creating all possible states..." << std::endl;
	if (mode == FACTORISED)
	{
		std::map<const Bomb*, const Package*> no_packages;
		State basic_state("basic", no_packages);
		
		std::vector<const KnowledgeBase*> knowledge_bases;
		KnowledgeBase basis_kb("basis_kb");
		basis_kb.addState(basic_state);
		knowledge_bases.push_back(&basis_kb);
		
		// Create a new knowledge base for each bomb.
		for (std::vector<const Bomb*>::const_iterator ci = bombs.begin(); ci != bombs.end(); ++ci)
		{
			const Bomb* bomb = *ci;
			ss.str(std::string());
			ss << "kb_" << bomb->name_;
			
			KnowledgeBase* kb = new KnowledgeBase(ss.str());
			basis_kb.addChild(*kb);
			knowledge_bases.push_back(kb);
			
			for (std::vector<const Package*>::const_iterator ci = packages.begin(); ci != packages.end(); ++ci)
			{
				const Package* package = *ci;
				std::map<const Bomb*, const Package*> bomb_package_mapping;
				bomb_package_mapping[bomb] = package;
				ss << "_" <<package->name_;
				State* state = new State(ss.str(), bomb_package_mapping);
				kb->addState(*state);
			}
		}
		
		std::cout << "Generate domain..." << std::endl;
		generateDomain("test_domain.pddl", basis_kb, knowledge_bases, packages, bombs, true);
		std::cout << "Generate problem..." << std::endl;
		generateProblem("test_problem.pddl", basis_kb, knowledge_bases, packages, bombs, true);
	}
	else if (mode == ORIGINAL)
	{
		// Enumerate all the possible states.
		std::vector<int> bomb_locations(packages.size(), 0);
		
		bool done = false;
		unsigned int state_id = 0;
		std::stringstream ss;
		
		KnowledgeBase basis_kb("basis_kb");
		std::vector<const KnowledgeBase*> knowledge_bases;
		knowledge_bases.push_back(&basis_kb);
		
		while (!done)
		{
			std::map<const Bomb*, const Package*> bomb_package_mapping;
			for (unsigned int bomb_id = 0; bomb_id < bombs.size(); ++bomb_id)
			{
				const Bomb* bomb = bombs[bomb_id];
				bomb_package_mapping[bomb] = packages[bomb_locations[bomb_id]];
			}
			
			ss.str(std::string());
			ss << "s" << state_id;
			State* state = new State(ss.str(), bomb_package_mapping);
			basis_kb.addState(*state);
			++state_id;
			
			done = true;
			for (unsigned int i = 0; i < bomb_locations.size(); ++i)
			{
				if (bomb_locations[i] + 1 != packages.size())
				{
					bomb_locations[i] = bomb_locations[i] + 1;
					done = false;
					break;
				}
				bomb_locations[i] = 0;
			}
		}
		generateDomain("test_domain.pddl", basis_kb, knowledge_bases, packages, bombs, false);
		generateProblem("test_problem.pddl", basis_kb, knowledge_bases, packages, bombs, false);
	}
	else if (mode == PRP)
	{
		generateDomain("test_domain.pddl", packages, bombs);
		generateProblem("test_problem.pddl",packages, bombs);
	}
	
	return 0;
}
