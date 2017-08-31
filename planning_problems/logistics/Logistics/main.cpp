#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <stdlib.h>
#include <cmath>
#include <map>

struct City;

struct NamedObject
{
	NamedObject(const std::string name)
		: name_(name)
	{
		
	}
	std::string name_;
};

struct StoragePlace : public NamedObject
{
	StoragePlace(const std::string& name, City& city)
		: NamedObject(name), city_(&city)
	{
		
	}
	
	City* city_;
};

struct Airport : public StoragePlace
{
	Airport(const std::string& name, City& city)
		: StoragePlace(name, city)
	{
		
	}
};

struct Location : public StoragePlace
{
	Location(const std::string& name, City& city)
		: StoragePlace(name, city)
	{
		
	}
};


struct Package : public NamedObject
{
	Package(const std::string& name, City& starting_city, Airport& destination)
		: NamedObject(name), starting_city_(&starting_city), destination_(&destination)
	{
		
	}
	
	City* starting_city_;
	Airport* destination_;
};

struct City : public NamedObject
{
	City(const std::string name)
		: NamedObject(name)
	{
		
	}
	
	std::vector<Airport*> airports_;
	std::vector<Location*> locations_;
};																													 

struct Truck : public NamedObject
{
	Truck(const std::string name, const StoragePlace& loc)
		: NamedObject(name), loc_(&loc)
	{
		
	}
	
	const StoragePlace* loc_;
};

struct Airplane : public NamedObject
{
	Airplane(const std::string name, const Airport& airport)
		: NamedObject(name), airport_(&airport)
	{
		
	}
	
	const Airport* airport_;
};

struct KnowledgeBase;

struct State
{
	State(const std::string& state_name, const std::map<Package*, Location*>& packages)
 		: state_name_(state_name), packages_(packages), kb_(NULL)
	{
		
	}
	
	std::string state_name_;
	std::map<Package*, Location*> packages_;
	KnowledgeBase* kb_;
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
		state.kb_ = this;
		states_.push_back(&state);
	}
	
	std::string name_;
	std::vector<const KnowledgeBase*> children_;
	std::vector<const State*> states_;
};

void generateProblem(const std::string& file_name, const std::vector<City*>& cities, const std::vector<Truck*>& trucks, const Airplane& airplane, const std::vector<Package*>& packages)
{
	std::ofstream myfile;
	myfile.open(file_name.c_str());
	myfile << "(define (problem logistics-problem)" << std::endl;
	myfile << "(:domain logistics)" << std::endl;
	myfile << "(:objects" << std::endl;
	
	for (std::vector<Package*>::const_iterator ci = packages.begin(); ci != packages.end(); ++ci)
	{
		myfile << "\t" << (*ci)->name_ << " - obj" << std::endl;
	}
	
	for (std::vector<Truck*>::const_iterator ci = trucks.begin(); ci != trucks.end(); ++ci)
	{
		myfile << "\t" << (*ci)->name_ << " - truck" << std::endl;
	}
	
	myfile << "\t" << airplane.name_ << " - airplane" << std::endl;
	
	for (std::vector<City*>::const_iterator ci = cities.begin(); ci != cities.end(); ++ci)
	{
		City* city = *ci;
		myfile << "\t" << city->name_ << " - city" << std::endl;
		for (std::vector<Location*>::const_iterator ci = city->locations_.begin(); ci != city->locations_.end(); ++ci)
		{
			Location* location = *ci;
			myfile << "\t" << location->name_ << " -location" << std::endl;
		}
		
		for (std::vector<Airport*>::const_iterator ci = city->airports_.begin(); ci != city->airports_.end(); ++ci)
		{
			Airport* airport = *ci;
			myfile << "\t" << airport->name_ << " -airport" << std::endl;
		}
	}
	myfile << ")" << std::endl;
	myfile << std::endl;

	std::cout << "Start encoding..." << std::endl;
	myfile << "(:init" << std::endl;
	
	// Construct the city.
	for (std::vector<City*>::const_iterator ci = cities.begin(); ci != cities.end(); ++ci)
	{
		const City* city = *ci;
		for (std::vector<Airport*>::const_iterator ci = city->airports_.begin(); ci != city->airports_.end(); ++ci)
		{
			Airport* airport = *ci;
			myfile << "\t(in-city-a " << airport->name_ << " " << city->name_ << ")" << std::endl;
		}
		
		for (std::vector<Location*>::const_iterator ci = city->locations_.begin(); ci != city->locations_.end(); ++ci)
		{
			Location* location = *ci;
			myfile << "\t(in-city-l " << location->name_ << " " << city->name_ << ")" << std::endl;
		}
	}
	
	// Put the trucks and airplanes in their starting locations.
	for (std::vector<Truck*>::const_iterator ci = trucks.begin(); ci != trucks.end(); ++ci)
	{
		Truck* truck = *ci;
		myfile << "\t(in-city-t " << truck->name_ << " " << truck->loc_->city_->name_ << ")" << std::endl;
		myfile << "\t(at-tl " << truck->name_ << " " << truck->loc_->name_ << ")" << std::endl;
	}
	
	myfile << "\t(at-aa " << airplane.name_ << " " << airplane.airport_->name_ << ")" << std::endl;
	
	for (std::vector<Package*>::const_iterator ci = packages.begin(); ci != packages.end(); ++ci)
	{
		const Package* package = *ci;
		if (cities.size() > 1)
		{
			myfile << "\t(oneof " << std::endl;
		}
		
		for (std::vector<City*>::const_iterator ci = cities.begin(); ci != cities.end(); ++ci)
		{
			const City* city = *ci;
			for (std::vector<Location*>::const_iterator ci = city->locations_.begin(); ci != city->locations_.end(); ++ci)
			{
				const Location* location = *ci;
				myfile << "\t(at-ol " << package->name_ << " " << location->name_ << ")" << std::endl;
			}
		}
		if (cities.size() > 1)
		{
			myfile << "\t)" << std::endl;
		}
	}
	
	std::cout << "Encoding goals..." << std::endl;
	myfile << ")" << std::endl;
	myfile << "(:goal (and" << std::endl;
	for (std::vector<Package*>::const_iterator ci = packages.begin(); ci != packages.end(); ++ci)
	{
		Package* package = *ci;
		myfile << "\t(at-oa " << package->name_ << " " << package->destination_->name_ << ")" << std::endl;
	}
	myfile << ")" << std::endl;
	myfile << ")" << std::endl;
	myfile << ")" << std::endl;
	myfile.close();
}

void generateDomain(const std::string& file_name, const std::vector<City*>& cities, const std::vector<Truck*>& trucks, const Airplane& airplane, const std::vector<Package*>& packages)
{
	std::ofstream myfile;
	myfile.open (file_name.c_str());
	myfile << "(define (domain logistics)" << std::endl;
	myfile << "(:requirements :typing)" << std::endl;
	myfile << std::endl;
	myfile << "(:types" << std::endl;
	myfile << "\tobj truck location airplane city airport" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	myfile << "(:predicates" << std::endl;
	myfile << "\t(at-tl ?obj - truck ?loc - location)" << std::endl;
	myfile << "\t(at-ta ?obj - truck ?loc - airport)" << std::endl;
	myfile << "\t(at-aa ?obj - airplane ?loc - airport)" << std::endl;
	myfile << "\t(at-ol ?obj - obj ?loc - location)" << std::endl;
	myfile << "\t(at-oa ?obj - obj ?loc - airport)" << std::endl;
	myfile << "\t(in-ot ?obj1 - obj ?obj2 - truck)" << std::endl;
	myfile << "\t(in-oa ?obj1 - obj ?obj2 - airplane)" << std::endl;
	myfile << "\t(in-city-l ?loc - location ?city - city)" << std::endl;
	myfile << "\t(in-city-a ?loc - airport ?city - city)" << std::endl;
	myfile << "\t(in-city-t ?loc - truck ?city - city)" << std::endl;
	
	myfile << ")" << std::endl;
	myfile << std::endl;

	/**
	 * Sense a package at a location with a truck.
	 */
	myfile << ";; Sense if a package is at the same location as the truck." << std::endl;
	myfile << "(:action sense-package-loc-t" << std::endl;
	myfile << "\t:parameters (?obj - obj ?loc - location ?truck - truck)" << std::endl;
	
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(at-tl ?truck ?loc)" << std::endl;
	myfile << "\t)" << std::endl;
	myfile << "\t:observe (at-ol ?obj ?loc)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * Sense a package at an airport with a truck.
	 */
	myfile << ";; Sense if a package is at the same airport as the truck." << std::endl;
	myfile << "(:action sense-package-ap-t" << std::endl;
	myfile << "\t:parameters (?obj - obj ?loc - airport ?truck - truck)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(at-ta ?truck ?loc)" << std::endl;
	myfile << "\t)" << std::endl;
	myfile << "\t:observe (at-oa ?obj ?loc)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * Sense a package at an airport with a plane.
	 */
	myfile << ";; Sense if a package is at the same airport as the plane." << std::endl;
	myfile << "(:action sense-package-ap-a" << std::endl;
	myfile << "\t:parameters (?obj - obj ?loc - airport ?airplane - airplane)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(at-aa ?airplane ?loc)" << std::endl;
	myfile << "\t)" << std::endl;
	myfile << "\t:observe (at-oa ?obj ?loc)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * Load the truck at a location.
	 */
	myfile << ";; Load the truck." << std::endl;
	myfile << "(:action load-truck-loc" << std::endl;
	myfile << "\t:parameters (?obj - obj ?truck - truck ?loc - location ?city - city)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(in-city-t ?truck ?city)" << std::endl;
	myfile << "\t\t(in-city-l ?loc ?city)" << std::endl;
	myfile << "\t\t(at-tl ?truck ?loc)" << std::endl;
	myfile << "\t\t(at-ol ?obj ?loc)" << std::endl;
	
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t\t\t(not (at-ol ?obj ?loc))" << std::endl;
	myfile << "\t\t\t\t(in-ot ?obj ?truck)" << std::endl;
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * Load the truck at the airport.
	 */
	myfile << ";; Load the truck." << std::endl;
	myfile << "(:action load-truck-ap" << std::endl;
	myfile << "\t:parameters (?obj - obj ?truck - truck ?loc - airport ?city - city)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(in-city-t ?truck ?city)" << std::endl;
	myfile << "\t\t(in-city-a ?loc ?city)" << std::endl;
	myfile << "\t\t(at-ta ?truck ?loc)" << std::endl;
	myfile << "\t\t(at-oa ?obj ?loc)" << std::endl;
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t\t\t(not (at-oa ?obj ?loc))" << std::endl;
	myfile << "\t\t\t\t(in-ot ?obj ?truck)" << std::endl;
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * Load the airplane at the airport.
	 */
	myfile << ";; Load the airplane." << std::endl;
	myfile << "(:action load-airplane" << std::endl;
	myfile << "\t:parameters (?obj - obj ?airplane - airplane ?loc - airport ?city - city)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(at-aa ?airplane ?loc)" << std::endl;
	myfile << "\t\t(at-oa ?obj ?loc)" << std::endl;
	
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t(not (at-oa ?obj ?loc))" << std::endl;
	myfile << "\t\t(in-oa ?obj ?airplane)" << std::endl;
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * Unload a truck at a location.
	 */
	myfile << ";; Unload the truck." << std::endl;
	myfile << "(:action unload-truck-loc" << std::endl;
	myfile << "\t:parameters (?obj - obj ?truck - truck ?loc - location ?city - city)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(in-city-t ?truck ?city)" << std::endl;
	myfile << "\t\t(in-city-l ?loc ?city)" << std::endl;
	myfile << "\t\t(at-tl ?truck ?loc)" << std::endl;
	myfile << "\t\t(in-ot ?obj ?truck)" << std::endl;
	
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t(not (in-ot ?obj ?truck))" << std::endl;
	myfile << "\t\t(at-ol ?obj ?loc)" << std::endl;
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * Unload a truck at an airport.
	 */
	myfile << ";; Unload the truck at an airport." << std::endl;
	myfile << "(:action unload-truck-ap" << std::endl;
	myfile << "\t:parameters (?obj - obj ?truck - truck ?loc - airport ?city - city)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(in-city-t ?truck ?city)" << std::endl;
	myfile << "\t\t(in-city-a ?loc ?city)" << std::endl;
	myfile << "\t\t(at-ta ?truck ?loc)" << std::endl;
	myfile << "\t\t(in-ot ?obj ?truck)" << std::endl;
	
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t(not (in-ot ?obj ?truck))" << std::endl;
	myfile << "\t\t(at-oa ?obj ?loc)" << std::endl;
	
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * Unload an airplane.
	 */
	myfile << ";; Unload an airplane." << std::endl;
	myfile << "(:action unload-airplane" << std::endl;
	myfile << "\t:parameters (?obj - obj ?airplane - airplane ?loc - airport)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(at-aa ?airplane ?loc)" << std::endl;
	myfile << "\t\t(in-oa ?obj ?airplane)" << std::endl;
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t(not (in-oa ?obj ?airplane))" << std::endl;
	myfile << "\t\t(at-oa ?obj ?loc)" << std::endl;
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * Drive truck to an airport.
	 */
	myfile << ";; Drive a truck to an airport." << std::endl;
	myfile << "(:action drive-truck-loc-ap" << std::endl;
	myfile << "\t:parameters (?truck - truck ?loc1 - location ?loc2 - airport ?city - city)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(in-city-l ?loc1 ?city)" << std::endl;
	myfile << "\t\t(in-city-a ?loc2 ?city)" << std::endl;
	myfile << "\t\t(in-city-t ?truck ?city)" << std::endl;
	myfile << "\t\t(at-tl ?truck ?loc1)" << std::endl;
	
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t(not (at-tl ?truck ?loc1))" << std::endl;
	myfile << "\t\t(at-ta ?truck ?loc2)" << std::endl;
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * Drive truck to a location.
	 */
	myfile << ";; Drive a truck to a location." << std::endl;
	myfile << "(:action drive-truck-ap-loc" << std::endl;
	myfile << "\t:parameters (?truck - truck ?loc1 - airport ?loc2 - location ?city - city)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(in-city-a ?loc1 ?city)" << std::endl;
	myfile << "\t\t(in-city-l ?loc2 ?city)" << std::endl;
	myfile << "\t\t(in-city-t ?truck ?city)" << std::endl;
	myfile << "\t\t(at-ta ?truck ?loc1)" << std::endl;
	
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t(not (at-ta ?truck ?loc1))" << std::endl;
	myfile << "\t\t(at-tl ?truck ?loc2)" << std::endl;
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * Fly an airplane.
	 */
	myfile << ";; Fly an airplane." << std::endl;
	myfile << "(:action fly-airplane" << std::endl;
	myfile << "\t:parameters (?airplane - airplane ?loc1 ?loc2 - airport)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(at-aa ?airplane ?loc1)" << std::endl;
	
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t(not (at-aa ?airplane ?loc1))" << std::endl;
	myfile << "\t\t(at-aa ?airplane ?loc2)" << std::endl;
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	myfile << ")" << std::endl;
	myfile.close();
}

/**
 * PRP-PO generator.
 */


void generateProblem(const std::string& file_name, KnowledgeBase& current_knowledge_base,  const std::vector<const KnowledgeBase*>& knowledge_base, const std::vector<City*>& cities, const std::vector<Truck*>& trucks, const Airplane& airplane, const std::vector<Package*>& packages, bool factorise)
{
	std::ofstream myfile;
	myfile.open(file_name.c_str());
	myfile << "(define (problem logistics-problem)" << std::endl;
	myfile << "(:domain logistics)" << std::endl;
	myfile << "(:objects" << std::endl;
	
	if (factorise)
	{
		myfile << "\tl0 - LEVEL" << std::endl;
		myfile << "\tl1 - LEVEL" << std::endl;
	}
	else
	{
		for (unsigned int i = 0; i < packages.size() + 1; ++i)
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
	else 
	{
		for (unsigned int i = 1; i < packages.size() + 1; ++i)
		{
			myfile << "\t(next l" << (i - 1) << " l" << i << ")" << std::endl;
		}
	}
	
	// Construct the city.
	for (std::vector<City*>::const_iterator ci = cities.begin(); ci != cities.end(); ++ci)
	{
		const City* city = *ci;
		for (std::vector<Airport*>::const_iterator ci = city->airports_.begin(); ci != city->airports_.end(); ++ci)
		{
			Airport* airport = *ci;
			myfile << "\t(in-city-a " << airport->name_ << " " << city->name_ << ")" << std::endl;
		}
		
		for (std::vector<Location*>::const_iterator ci = city->locations_.begin(); ci != city->locations_.end(); ++ci)
		{
			Location* location = *ci;
			myfile << "\t(in-city-l " << location->name_ << " " << city->name_ << ")" << std::endl;
		}
	}
	
	// Put the trucks and airplanes in their starting locations.
	for (std::vector<Truck*>::const_iterator ci = trucks.begin(); ci != trucks.end(); ++ci)
	{
		Truck* truck = *ci;
		myfile << "\t(in-city-t " << truck->name_ << " " << truck->loc_->city_->name_ << ")" << std::endl;
	}
	
	// Now generate the states.
	std::cout << "Encoding current knowledge base (" << current_knowledge_base.name_ << ")..." << std::endl;
	if (factorise)
	{
		myfile << "\t(current_kb " << current_knowledge_base.name_ << ")" << std::endl;
	}
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_base.begin(); ci != knowledge_base.end(); ++ci)
	{
		const KnowledgeBase* kb = *ci;
		std::cout << "Process knowledge base: " << kb->name_ << "(" << kb->states_.size() << ")" << std::endl;
		
		for (std::vector<const State*>::const_iterator ci = kb->states_.begin(); ci != kb->states_.end(); ++ci)
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
			}
			
			if (!factorise || kb == &current_knowledge_base)
			{
				for (std::vector<Truck*>::const_iterator ci = trucks.begin(); ci != trucks.end(); ++ci)
				{
					Truck* truck = *ci;
					myfile << "\t(at-tl " << truck->name_ << " " << truck->loc_->name_ << " " << state->state_name_<< ")" << std::endl;
				}
				myfile << "\t(at-aa " << airplane.name_ << " " << airplane.airport_->name_ << " " << state->state_name_ << ")" << std::endl;
			}
			
			// Position the packages.
			for (std::map<Package*, Location*>::const_iterator ci = state->packages_.begin(); ci != state->packages_.end(); ++ci)
			{
				Package* package = (*ci).first;
				Location* location= (*ci).second;
				myfile << "\t(at-ol " << package->name_ << " " << location->name_ << " " << state->state_name_ << ")" << std::endl;
				if (factorise)
				{
					myfile << "\t(in " << package->name_ << " " << state->kb_->name_ << ")" << std::endl;
				}
			}
		}
		
		for (std::vector<const KnowledgeBase*>::const_iterator ci = kb->children_.begin(); ci != kb->children_.end(); ++ci)
		{
			myfile << "\t(parent " << kb->name_ << " " << (*ci)->name_ << ")" << std::endl;
		}
	}
	
	std::cout << "Encoding goals..." << std::endl;
	myfile << ")" << std::endl;
	myfile << "(:goal (and" << std::endl;
	if (factorise)
	{
		for (std::vector<Package*>::const_iterator ci = packages.begin(); ci != packages.end(); ++ci)
		{
			Package* package = *ci;
			for (std::vector<const State*>::const_iterator ci = current_knowledge_base.states_.begin(); ci != current_knowledge_base.states_.end(); ++ci)
			{
				const State* state = *ci;
				myfile << "\t(at-oa " << package->name_ << " " << package->destination_->name_ << " " << state->state_name_ << ")" << std::endl;
			}
		}
	}
	else
	{
		for (std::vector<const State*>::const_iterator ci = current_knowledge_base.states_.begin(); ci != current_knowledge_base.states_.end(); ++ci)
		{
			const State* state = *ci;
			for (std::map<Package*, Location*>::const_iterator ci = state->packages_.begin(); ci != state->packages_.end(); ++ci)
			{
				Package* package = (*ci).first;
				myfile << "\t(at-oa " << package->name_ << " " << package->destination_->name_ << " " << state->state_name_ << ")" << std::endl;
			}
		}
	}
	myfile << ")" << std::endl;
	myfile << ")" << std::endl;
	myfile << ")" << std::endl;
	myfile.close();
}

void generateDomain(const std::string& file_name, const KnowledgeBase& current_knowledge_base, const std::vector<const KnowledgeBase*>& knowledge_bases, const std::vector<City*>& cities, const std::vector<Truck*>& trucks, const Airplane& airplane, const std::vector<Package*>& packages, bool factorise)
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
	myfile << "(define (domain logistics)" << std::endl;
	myfile << "(:requirements :typing :conditional-effects :negative-preconditions :disjunctive-preconditions)" << std::endl;
	myfile << std::endl;
	myfile << "(:types" << std::endl;
	myfile << "\tobj truck location airplane city airport" << std::endl;
	myfile << "\tlevel" << std::endl;
	myfile << "\tstate" << std::endl;
	if (factorise)
	{
		myfile << "\tknowledgebase" << std::endl;
	}
	myfile << ")" << std::endl;
	myfile << std::endl;
	myfile << "(:predicates" << std::endl;
	// (at-tl ?obj - TRUCK  ?loc - LOCATION)
	myfile << "\t(at-tl ?obj - truck ?loc - location ?s - state)" << std::endl;
	myfile << "\t(Rat-tl ?obj - truck ?loc - location ?s - state)" << std::endl;
	
	// (at-ta ?obj - TRUCK  ?loc - AIRPORT)
	myfile << "\t(at-ta ?obj - truck ?loc - airport ?s - state)" << std::endl;
	myfile << "\t(Rat-ta ?obj - truck ?loc - airport ?s - state)" << std::endl;
	
	// (at-aa ?obj - AIRPLANE ?loc - AIRPORT)
	myfile << "\t(at-aa ?obj - airplane ?loc - airport ?s - state)" << std::endl;
	myfile << "\t(Rat-aa ?obj - airplane ?loc - airport ?s - state)" << std::endl;
	
	// (at-ol ?obj - OBJ ?loc - LOCATION)
	myfile << "\t(at-ol ?obj - obj ?loc - location ?s - state)" << std::endl;
	myfile << "\t(Rat-ol ?obj - obj ?loc - location ?s - state)" << std::endl;
	
	// (at-oa ?obj - OBJ ?loc - AIRPORT)
	myfile << "\t(at-oa ?obj - obj ?loc - airport ?s - state)" << std::endl;
	myfile << "\t(Rat-oa ?obj - obj ?loc - airport ?s - state)" << std::endl;
	
	// (in-ot ?obj1 - OBJ ?obj2 - TRUCK)
	myfile << "\t(in-ot ?obj1 - obj ?obj2 - truck ?s - state)" << std::endl;
	myfile << "\t(Rin-ot ?obj1 - obj ?obj2 - truck ?s - state)" << std::endl;
	
	// (in-oa ?obj1 - OBJ ?obj2 - AIRPLANE)
	myfile << "\t(in-oa ?obj1 - obj ?obj2 - airplane ?s - state)" << std::endl;
	myfile << "\t(Rin-oa ?obj1 - obj ?obj2 - airplane ?s - state)" << std::endl;
	
	//(in-city-l ?loc - LOCATION ?city - CITY)
	myfile << "\t(in-city-l ?loc - location ?city - city)" << std::endl;
	
	//(in-city-a ?loc - AIRPORT ?city - CITY)
	myfile << "\t(in-city-a ?loc - airport ?city - city)" << std::endl;
	
	//(in-city-t ?loc - TRUCK ?city - CITY))
	myfile << "\t(in-city-t ?loc - truck ?city - city)" << std::endl;
	
	if (factorise)
	{
		myfile << "\t(in ?obj - obj ?kb - knowledgebase)" << std::endl;
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
	
	for (std::vector<Package*>::const_iterator ci = packages.begin(); ci != packages.end(); ++ci)
	{
		myfile << "\t" << (*ci)->name_ << " - obj" << std::endl;
	}
	
	for (std::vector<Truck*>::const_iterator ci = trucks.begin(); ci != trucks.end(); ++ci)
	{
		myfile << "\t" << (*ci)->name_ << " - truck" << std::endl;
	}
	
	myfile << "\t" << airplane.name_ << " - airplane" << std::endl;
	
	for (std::vector<City*>::const_iterator ci = cities.begin(); ci != cities.end(); ++ci)
	{
		City* city = *ci;
		myfile << "\t" << city->name_ << " - city" << std::endl;
		for (std::vector<Location*>::const_iterator ci = city->locations_.begin(); ci != city->locations_.end(); ++ci)
		{
			Location* location = *ci;
			myfile << "\t" << location->name_ << " -location" << std::endl;
		}
		
		for (std::vector<Airport*>::const_iterator ci = city->airports_.begin(); ci != city->airports_.end(); ++ci)
		{
			Airport* airport = *ci;
			myfile << "\t" << airport->name_ << " -airport" << std::endl;
		}
	}

	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * Sense a package at a location with a truck.
	 */
	myfile << ";; Sense if a package is at the same location as the truck." << std::endl;
	myfile << "(:action sense-package-loc-t" << std::endl;
	if (factorise)
	{
		myfile << "\t:parameters (?obj - obj ?loc - location ?truck - truck ?l ?l2 - level ?kb - knowledgebase)" << std::endl;
	}
	else
	{
		myfile << "\t:parameters (?obj - obj ?loc - location ?truck - truck ?l ?l2 - level)" << std::endl;
	}
	
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	myfile << "\t\t(next ?l ?l2)" << std::endl;
	myfile << "\t\t(lev ?l)" << std::endl;
	myfile << std::endl;
	
	if (factorise)
	{
		myfile << "\t\t(current_kb ?kb)" << std::endl;
		myfile << "\t\t(not (current_kb basis_kb))" << std::endl;
	}

	myfile << "\t\t;; For every state ?s" << std::endl;
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(Rat-tl ?truck ?loc " << (*ci)->state_name_ << ")" << std::endl;
		myfile << std::endl;
	}
	
	myfile << "\t\t;; This action is only applicable if there are world states where the outcome can be different." << std::endl;
	if (factorise)
	{
		myfile << "\t\t(exists (?s - state) (and (m ?s) (at-ol ?obj ?loc ?s) (part-of ?s ?kb)))" << std::endl;
		myfile << "\t\t(exists (?s - state) (and (m ?s) (not (at-ol ?obj ?loc ?s)) (part-of ?s ?kb)))" << std::endl;;
	}
	else
	{
		myfile << "\t\t(exists (?s - state) (and (m ?s) (at-ol ?obj ?loc ?s)))" << std::endl;
		myfile << "\t\t(exists (?s - state) (and (m ?s) (not (at-ol ?obj ?loc ?s))))" << std::endl;;
	}

	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t(not (lev ?l))" << std::endl;
	myfile << "\t\t(lev ?l2)" << std::endl;
	myfile << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(when (and (m " << (*ci)->state_name_ << ") (not (at-ol ?obj ?loc " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t(and (stack " << (*ci)->state_name_ << " ?l) (not (m " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	myfile << "\t\t(resolve-axioms)" << std::endl;
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * Sense a package at an airport with a truck.
	 */
	myfile << ";; Sense if a package is at the same airport as the truck." << std::endl;
	myfile << "(:action sense-package-ap-t" << std::endl;
	if (factorise)
	{
		myfile << "\t:parameters (?obj - obj ?loc - airport ?truck - truck ?l ?l2 - level ?kb - knowledgebase)" << std::endl;
	}
	else
	{
		myfile << "\t:parameters (?obj - obj ?loc - airport ?truck - truck ?l ?l2 - level)" << std::endl;
	}

	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	myfile << "\t\t(next ?l ?l2)" << std::endl;
	myfile << "\t\t(lev ?l)" << std::endl;
	myfile << std::endl;
	if (factorise)
	{
		myfile << "\t\t(current_kb ?kb)" << std::endl;
		myfile << "\t\t(not (current_kb basis_kb))" << std::endl;
	}

	myfile << "\t\t;; For every state ?s" << std::endl;
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(Rat-ta ?truck ?loc " << (*ci)->state_name_ << ")" << std::endl;
		myfile << std::endl;
	}
	
	myfile << "\t\t;; This action is only applicable if there are world states where the outcome can be different." << std::endl;
	if (factorise)
	{
		myfile << "\t\t(exists (?s - state) (and (m ?s) (at-oa ?obj ?loc ?s) (part-of ?s ?kb)))" << std::endl;
		myfile << "\t\t(exists (?s - state) (and (m ?s) (not (at-oa ?obj ?loc ?s)) (part-of ?s ?kb)))" << std::endl;;
	}
	else
	{
		myfile << "\t\t(exists (?s - state) (and (m ?s) (at-oa ?obj ?loc ?s)))" << std::endl;
		myfile << "\t\t(exists (?s - state) (and (m ?s) (not (at-oa ?obj ?loc ?s))))" << std::endl;;
	}

	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t(not (lev ?l))" << std::endl;
	myfile << "\t\t(lev ?l2)" << std::endl;
	myfile << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(when (and (m " << (*ci)->state_name_ << ") (not (at-oa ?obj ?loc " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t(and (stack " << (*ci)->state_name_ << " ?l) (not (m " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	myfile << "\t\t(resolve-axioms)" << std::endl;
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * Sense a package at an airport with a plane.
	 */
	myfile << ";; Sense if a package is at the same airport as the plane." << std::endl;
	myfile << "(:action sense-package-ap-a" << std::endl;
	if (factorise)
	{
		myfile << "\t:parameters (?obj - obj ?loc - airport ?airplane - airplane ?l ?l2 - level ?kb - knowledgebase)" << std::endl;
	}
	else
	{
		myfile << "\t:parameters (?obj - obj ?loc - airport ?airplane - airplane ?l ?l2 - level)" << std::endl;
	}

	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	myfile << "\t\t(next ?l ?l2)" << std::endl;
	myfile << "\t\t(lev ?l)" << std::endl;
	myfile << std::endl;
	if (factorise)
	{
		myfile << "\t\t(current_kb ?kb)" << std::endl;
		myfile << "\t\t(not (current_kb basis_kb))" << std::endl;
	}

	myfile << "\t\t;; For every state ?s" << std::endl;
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(Rat-aa ?airplane ?loc " << (*ci)->state_name_ << ")" << std::endl;
		myfile << std::endl;
	}
	
	myfile << "\t\t;; This action is only applicable if there are world states where the outcome can be different." << std::endl;
	if (factorise)
	{
		myfile << "\t\t(exists (?s - state) (and (m ?s) (at-oa ?obj ?loc ?s) (part-of ?s ?kb)))" << std::endl;
		myfile << "\t\t(exists (?s - state) (and (m ?s) (not (at-oa ?obj ?loc ?s)) (part-of ?s ?kb)))" << std::endl;;
	}
	else
	{
		myfile << "\t\t(exists (?s - state) (and (m ?s) (at-oa ?obj ?loc ?s)))" << std::endl;
		myfile << "\t\t(exists (?s - state) (and (m ?s) (not (at-oa ?obj ?loc ?s))))" << std::endl;;
	}

	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t(not (lev ?l))" << std::endl;
	myfile << "\t\t(lev ?l2)" << std::endl;
	myfile << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(when (and (m " << (*ci)->state_name_ << ") (not (at-oa ?obj ?loc " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t\t(and (stack " << (*ci)->state_name_ << " ?l) (not (m " << (*ci)->state_name_ << ")))" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	myfile << "\t\t(resolve-axioms)" << std::endl;
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * Load the truck at a location.
	 */
	myfile << ";; Load the truck." << std::endl;
	myfile << "(:action load-truck-loc" << std::endl;
	myfile << "\t:parameters (?obj - obj ?truck - truck ?loc - location ?city - city)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	myfile << "\t\t(in-city-t ?truck ?city)" << std::endl;
	myfile << "\t\t(in-city-l ?loc ?city)" << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_.begin(); ci != knowledge_base->states_.end(); ++ci)
		{
			myfile << "\t\t(Rat-tl ?truck ?loc " << (*ci)->state_name_ << ")" << std::endl;
			myfile << "\t\t(Rat-ol ?obj ?loc " << (*ci)->state_name_ << ")" << std::endl;
		}
	}
	
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(when (m " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t(and" << std::endl;
		myfile << "\t\t\t\t(not (at-ol ?obj ?loc " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(in-ot ?obj ?truck " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t\t(not (Rat-ol ?obj ?loc " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(Rin-ot ?obj ?truck " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t)" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * Load the truck at the airport.
	 */
	myfile << ";; Load the truck." << std::endl;
	myfile << "(:action load-truck-ap" << std::endl;
	myfile << "\t:parameters (?obj - obj ?truck - truck ?loc - airport ?city - city)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	myfile << "\t\t(in-city-t ?truck ?city)" << std::endl;
	myfile << "\t\t(in-city-a ?loc ?city)" << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_.begin(); ci != knowledge_base->states_.end(); ++ci)
		{
			myfile << "\t\t(Rat-ta ?truck ?loc " << (*ci)->state_name_ << ")" << std::endl;
			myfile << "\t\t(Rat-oa ?obj ?loc " << (*ci)->state_name_ << ")" << std::endl;
		}
	}
	
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(when (m " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t(and" << std::endl;
		myfile << "\t\t\t\t(not (at-oa ?obj ?loc " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(in-ot ?obj ?truck " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t\t(not (Rat-oa ?obj ?loc " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(Rin-ot ?obj ?truck " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t)" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * Load the airplane at the airport.
	 */
	myfile << ";; Load the airplane." << std::endl;
	myfile << "(:action load-airplane" << std::endl;
	myfile << "\t:parameters (?obj - obj ?airplane - airplane ?loc - airport ?city - city)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_.begin(); ci != knowledge_base->states_.end(); ++ci)
		{
			myfile << "\t\t(Rat-aa ?airplane ?loc " << (*ci)->state_name_ << ")" << std::endl;
			myfile << "\t\t(Rat-oa ?obj ?loc " << (*ci)->state_name_ << ")" << std::endl;
		}
	}
	
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(when (m " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t(and" << std::endl;
		myfile << "\t\t\t\t(not (at-oa ?obj ?loc " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(in-oa ?obj ?airplane " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t\t(not (Rat-oa ?obj ?loc " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(Rin-oa ?obj ?airplane " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t)" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * Unload a truck at a location.
	 */
	myfile << ";; Unload the truck." << std::endl;
	myfile << "(:action unload-truck-loc" << std::endl;
	myfile << "\t:parameters (?obj - obj ?truck - truck ?loc - location ?city - city)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	myfile << "\t\t(in-city-t ?truck ?city)" << std::endl;
	myfile << "\t\t(in-city-l ?loc ?city)" << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_.begin(); ci != knowledge_base->states_.end(); ++ci)
		{
			myfile << "\t\t(Rat-tl ?truck ?loc " << (*ci)->state_name_ << ")" << std::endl;
			myfile << "\t\t(Rin-ot ?obj ?truck " << (*ci)->state_name_ << ")" << std::endl;
		}
	}
	
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(when (m " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t(and" << std::endl;
		myfile << "\t\t\t\t(not (in-ot ?obj ?truck " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(not (Rin-ot ?obj ?truck " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(at-ol ?obj ?loc " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t\t(Rat-ol ?obj ?loc " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t)" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * Unload a truck at an airport.
	 */
	myfile << ";; Unload the truck at an airport." << std::endl;
	myfile << "(:action unload-truck-ap" << std::endl;
	myfile << "\t:parameters (?obj - obj ?truck - truck ?loc - airport ?city - city)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	myfile << "\t\t(in-city-t ?truck ?city)" << std::endl;
	myfile << "\t\t(in-city-a ?loc ?city)" << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_.begin(); ci != knowledge_base->states_.end(); ++ci)
		{
			myfile << "\t\t(Rat-ta ?truck ?loc " << (*ci)->state_name_ << ")" << std::endl;
			myfile << "\t\t(Rin-ot ?obj ?truck " << (*ci)->state_name_ << ")" << std::endl;
		}
	}
	
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(when (m " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t(and" << std::endl;
		myfile << "\t\t\t\t(not (in-ot ?obj ?truck " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(not (Rin-ot ?obj ?truck " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(at-oa ?obj ?loc " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t\t(Rat-oa ?obj ?loc " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t)" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * Unload an airplane.
	 */
	myfile << ";; Unload an airplane." << std::endl;
	myfile << "(:action unload-airplane" << std::endl;
	myfile << "\t:parameters (?obj - obj ?airplane - airplane ?loc - airport)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_.begin(); ci != knowledge_base->states_.end(); ++ci)
		{
			myfile << "\t\t(Rat-aa ?airplane ?loc " << (*ci)->state_name_ << ")" << std::endl;
			myfile << "\t\t(Rin-oa ?obj ?airplane " << (*ci)->state_name_ << ")" << std::endl;
		}
	}
	
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(when (m " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t(and" << std::endl;
		myfile << "\t\t\t\t(not (in-oa ?obj ?airplane " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(not (Rin-oa ?obj ?airplane " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(at-oa ?obj ?loc " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t\t(Rat-oa ?obj ?loc " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t)" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * Drive truck to an airport.
	 */
	myfile << ";; Drive a truck to an airport." << std::endl;
	myfile << "(:action drive-truck-loc-ap" << std::endl;
	myfile << "\t:parameters (?truck - truck ?loc1 - location ?loc2 - airport ?city - city)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	myfile << "\t\t(in-city-l ?loc1 ?city)" << std::endl;
	myfile << "\t\t(in-city-a ?loc2 ?city)" << std::endl;
	myfile << "\t\t(in-city-t ?truck ?city)" << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_.begin(); ci != knowledge_base->states_.end(); ++ci)
		{
			myfile << "\t\t(Rat-tl ?truck ?loc1 " << (*ci)->state_name_ << ")" << std::endl;
		}
	}
	
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(when (m " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t(and" << std::endl;
		myfile << "\t\t\t\t(not (at-tl ?truck ?loc1 " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(not (Rat-tl ?truck ?loc1 " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(at-ta ?truck ?loc2 " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t\t(Rat-ta ?truck ?loc2 " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t)" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * Drive truck to a location.
	 */
	myfile << ";; Drive a truck to a location." << std::endl;
	myfile << "(:action drive-truck-ap-loc" << std::endl;
	myfile << "\t:parameters (?truck - truck ?loc1 - airport ?loc2 - location ?city - city)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	myfile << "\t\t(in-city-a ?loc1 ?city)" << std::endl;
	myfile << "\t\t(in-city-l ?loc2 ?city)" << std::endl;
	myfile << "\t\t(in-city-t ?truck ?city)" << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_.begin(); ci != knowledge_base->states_.end(); ++ci)
		{
			myfile << "\t\t(Rat-ta ?truck ?loc1 " << (*ci)->state_name_ << ")" << std::endl;
		}
	}
	
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(when (m " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t(and" << std::endl;
		myfile << "\t\t\t\t(not (at-ta ?truck ?loc1 " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(not (Rat-ta ?truck ?loc1 " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(at-tl ?truck ?loc2 " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t\t(Rat-tl ?truck ?loc2 " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t)" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	
	myfile << "\t)" << std::endl;
	myfile << ")" << std::endl;
	myfile << std::endl;
	
	/**
	 * Fly an airplane.
	 */
	myfile << ";; Fly an airplane." << std::endl;
	myfile << "(:action fly-airplane" << std::endl;
	myfile << "\t:parameters (?airplane - airplane ?loc1 ?loc2 - airport)" << std::endl;
	myfile << "\t:precondition (and" << std::endl;
	myfile << "\t\t(not (resolve-axioms))" << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	
	for (std::vector<const KnowledgeBase*>::const_iterator ci = knowledge_bases.begin(); ci != knowledge_bases.end(); ++ci)
	{
		const KnowledgeBase* knowledge_base = *ci;
		for (std::vector<const State*>::const_iterator ci = knowledge_base->states_.begin(); ci != knowledge_base->states_.end(); ++ci)
		{
			myfile << "\t\t(Rat-aa ?airplane ?loc1 " << (*ci)->state_name_ << ")" << std::endl;
		}
	}
	
	myfile << "\t)" << std::endl;
	myfile << "\t:effect (and" << std::endl;
	myfile << "\t\t;; For every state ?s" << std::endl;
	
	for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
	{
		myfile << "\t\t(when (m " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t(and" << std::endl;
		myfile << "\t\t\t\t(not (at-aa ?airplane ?loc1 " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(not (Rat-aa ?airplane ?loc1 " << (*ci)->state_name_ << "))" << std::endl;
		myfile << "\t\t\t\t(at-aa ?airplane ?loc2 " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t\t(Rat-aa ?airplane ?loc2 " << (*ci)->state_name_ << ")" << std::endl;
		myfile << "\t\t\t)" << std::endl;
		myfile << "\t\t)" << std::endl;
	}
	
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

	/**
	 * Raminificate.
	 */
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
		
		// The locations of the trucks, airplanes, and packages.
		for (std::vector<City*>::const_iterator ci = cities.begin(); ci != cities.end(); ++ci)
		{
			City* city = *ci;
			for (std::vector<Location*>::const_iterator ci = city->locations_.begin(); ci != city->locations_.end(); ++ci)
			{
				Location* location = *ci;
				for (std::vector<Truck*>::const_iterator ci = trucks.begin(); ci != trucks.end(); ++ci)
				{
					Truck* truck = *ci;
					myfile << "\t\t(when (or (at-tl " << truck->name_ << " " << location->name_ << " " << state->state_name_ << ") (not (m " << state->state_name_ << ")))" << std::endl;
					myfile << "\t\t\t(Rat-tl " << truck->name_ << " " << location->name_ << " " << state->state_name_ << ")" << std::endl;
					myfile << "\t\t)" << std::endl;
					
					myfile << "\t\t(when (and (not (at-tl " << truck->name_ << " " << location->name_ << " " << state->state_name_ << ")) (m " << state->state_name_ << "))" << std::endl;
					myfile << "\t\t\t(not (Rat-tl " << truck->name_ << " " << location->name_ << " " << state->state_name_ << "))" << std::endl;
					myfile << "\t\t)" << std::endl;
				}
				
				for (std::vector<Package*>::const_iterator ci = packages.begin(); ci != packages.end(); ++ci)
				{
					Package* package = *ci;
					myfile << "\t\t(when (or (at-ol " << package->name_ << " " << location->name_ << " " << state->state_name_ << ") (not (m " << state->state_name_ << ")))" << std::endl;
					myfile << "\t\t\t(Rat-ol " << package->name_ << " " << location->name_ << " " << state->state_name_ << ")" << std::endl;
					myfile << "\t\t)" << std::endl;
					
					myfile << "\t\t(when (and (not (at-ol " << package->name_ << " " << location->name_ << " " << state->state_name_ << ")) (m " << state->state_name_ << "))" << std::endl;
					myfile << "\t\t\t(not (Rat-ol " << package->name_ << " " << location->name_ << " " << state->state_name_ << "))" << std::endl;
					myfile << "\t\t)" << std::endl;
				}
			}
			
			for (std::vector<Airport*>::const_iterator ci = city->airports_.begin(); ci != city->airports_.end(); ++ci)
			{
				Airport* airport = *ci;
				for (std::vector<Truck*>::const_iterator ci = trucks.begin(); ci != trucks.end(); ++ci)
				{
					Truck* truck = *ci;
					myfile << "\t\t(when (or (at-ta " << truck->name_ << " " << airport->name_ << " " << state->state_name_ << ") (not (m " << state->state_name_ << ")))" << std::endl;
					myfile << "\t\t\t(Rat-ta " << truck->name_ << " " << airport->name_ << " " << state->state_name_ << ")" << std::endl;
					myfile << "\t\t)" << std::endl;
					
					myfile << "\t\t(when (and (not (at-ta " << truck->name_ << " " << airport->name_ << " " << state->state_name_ << ")) (m " << state->state_name_ << "))" << std::endl;
					myfile << "\t\t\t(not (Rat-ta " << truck->name_ << " " << airport->name_ << " " << state->state_name_ << "))" << std::endl;
					myfile << "\t\t)" << std::endl;
				}
				
				for (std::vector<Package*>::const_iterator ci = packages.begin(); ci != packages.end(); ++ci)
				{
					Package* package = *ci;
					myfile << "\t\t(when (or (at-oa " << package->name_ << " " << airport->name_ << " " << state->state_name_ << ") (not (m " << state->state_name_ << ")))" << std::endl;
					myfile << "\t\t\t(Rat-oa " << package->name_ << " " << airport->name_ << " " << state->state_name_ << ")" << std::endl;
					myfile << "\t\t)" << std::endl;
					
					myfile << "\t\t(when (and (not (at-oa " << package->name_ << " " << airport->name_ << " " << state->state_name_ << ")) (m " << state->state_name_ << "))" << std::endl;
					myfile << "\t\t\t(not (Rat-oa " << package->name_ << " " << airport->name_ << " " << state->state_name_ << "))" << std::endl;
					myfile << "\t\t)" << std::endl;
				}
				
				myfile << "\t\t(when (or (at-aa " << airplane.name_ << " " << airport->name_ << " " << state->state_name_ << ") (not (m " << state->state_name_ << ")))" << std::endl;
				myfile << "\t\t\t(Rat-aa " << airplane.name_ << " " << airport->name_ << " " << state->state_name_ << ")" << std::endl;
				myfile << "\t\t)" << std::endl;
				
				myfile << "\t\t(when (and (not (at-aa " << airplane.name_ << " " << airport->name_ << " " << state->state_name_ << ")) (m " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t\t(not (Rat-aa " << airplane.name_ << " " << airport->name_ << " " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t)" << std::endl;
			}
		}
		
		// Location of packages in trucks / airplanes.
		for (std::vector<Package*>::const_iterator ci = packages.begin(); ci != packages.end(); ++ci)
		{
			Package* package = *ci;
			for (std::vector<Truck*>::const_iterator ci = trucks.begin(); ci != trucks.end(); ++ci)
			{
				Truck* truck = *ci;
				myfile << "\t\t(when (or (in-ot " << package->name_ << " " << truck->name_ << " " << state->state_name_ << ") (not (m " << state->state_name_ << ")))" << std::endl;
				myfile << "\t\t\t(Rin-ot " << package->name_ << " " << truck->name_ << " " << state->state_name_ << ")" << std::endl;
				myfile << "\t\t)" << std::endl;
				
				myfile << "\t\t(when (and (not (in-ot " << package->name_ << " " << truck->name_ << " " << state->state_name_ << ")) (m " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t\t(not (Rin-ot " << package->name_ << " " << truck->name_ << " " << state->state_name_ << "))" << std::endl;
				myfile << "\t\t)" << std::endl;
			}
			
			myfile << "\t\t(when (or (in-oa " << package->name_ << " " << airplane.name_ << " " << state->state_name_ << ") (not (m " << state->state_name_ << ")))" << std::endl;
			myfile << "\t\t\t(Rin-oa " << package->name_ << " " << airplane.name_ << " " << state->state_name_ << ")" << std::endl;
			myfile << "\t\t)" << std::endl;
			
			myfile << "\t\t(when (and (not (in-oa " << package->name_ << " " << airplane.name_ << " " << state->state_name_ << ")) (m " << state->state_name_ << "))" << std::endl;
			myfile << "\t\t\t(not (Rin-oa " << package->name_ << " " << airplane.name_ << " " << state->state_name_ << "))" << std::endl;
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
				for (std::vector<City*>::const_iterator ci = cities.begin(); ci != cities.end(); ++ci)
				{
					City* city = *ci;
					for (std::vector<Location*>::const_iterator ci = city->locations_.begin(); ci != city->locations_.end(); ++ci)
					{
						Location* location = *ci;
						for (std::vector<Truck*>::const_iterator ci = trucks.begin(); ci != trucks.end(); ++ci)
						{
							const Truck* truck = *ci;
							// Copy the states of where we are.
							myfile << "\t\t(when (and (part-of " << state->state_name_ << " ?old_kb) (at-tl " << truck->name_ << " " << location->name_ << " " << state->state_name_ << ") (part-of " << state2->state_name_ << " ?new_kb))" << std::endl;
							myfile << "\t\t\t(and " << std::endl;
							myfile << "\t\t\t\t(not (Rat-tl " << truck->name_ << " " << location->name_ << " " << state->state_name_ << "))" << std::endl;
							myfile << "\t\t\t\t(not (at-tl " << truck->name_ << " " << location->name_ << " " << state->state_name_ << "))" << std::endl;
							myfile << "\t\t\t\t(Rat-tl " << truck->name_ << " " << location->name_ << " " << state2->state_name_ << ")" << std::endl;
							myfile << "\t\t\t\t(at-tl " << truck->name_ << " " << location->name_ << " " << state2->state_name_ << ")" << std::endl;
							myfile << "\t\t\t)" << std::endl;
							myfile << "\t\t)" << std::endl;
						}
						
						for (std::vector<Package*>::const_iterator ci = packages.begin(); ci != packages.end(); ++ci)
						{
							const Package* package = *ci;
							// Copy the states of where we are.
							myfile << "\t\t(when (and (part-of " << state->state_name_ << " ?old_kb) (at-ol " << package->name_ << " " << location->name_ << " " << state->state_name_ << ") (part-of " << state2->state_name_ << " ?new_kb))" << std::endl;
							myfile << "\t\t\t(and " << std::endl;
							myfile << "\t\t\t\t(not (Rat-ol " << package->name_ << " " << location->name_ << " " << state->state_name_ << "))" << std::endl;
							myfile << "\t\t\t\t(not (at-ol " << package->name_ << " " << location->name_ << " " << state->state_name_ << "))" << std::endl;
							myfile << "\t\t\t\t(Rat-ol " << package->name_ << " " << location->name_ << " " << state2->state_name_ << ")" << std::endl;
							myfile << "\t\t\t\t(at-ol " << package->name_ << " " << location->name_ << " " << state2->state_name_ << ")" << std::endl;
							myfile << "\t\t\t\t(in " << package->name_ << " " << state2->kb_->name_ << ")" << std::endl;
							myfile << "\t\t\t)" << std::endl;
							myfile << "\t\t)" << std::endl;
						}
					}
					
					for (std::vector<Airport*>::const_iterator ci = city->airports_.begin(); ci != city->airports_.end(); ++ci)
					{
						Airport* location = *ci;
						for (std::vector<Truck*>::const_iterator ci = trucks.begin(); ci != trucks.end(); ++ci)
						{
							const Truck* truck = *ci;
							// Copy the states of where we are.
							myfile << "\t\t(when (and (part-of " << state->state_name_ << " ?old_kb) (at-ta " << truck->name_ << " " << location->name_ << " " << state->state_name_ << ") (part-of " << state2->state_name_ << " ?new_kb))" << std::endl;
							myfile << "\t\t\t(and " << std::endl;
							myfile << "\t\t\t\t(not (Rat-ta " << truck->name_ << " " << location->name_ << " " << state->state_name_ << "))" << std::endl;
							myfile << "\t\t\t\t(not (at-ta " << truck->name_ << " " << location->name_ << " " << state->state_name_ << "))" << std::endl;
							myfile << "\t\t\t\t(Rat-ta " << truck->name_ << " " << location->name_ << " " << state2->state_name_ << ")" << std::endl;
							myfile << "\t\t\t\t(at-ta " << truck->name_ << " " << location->name_ << " " << state2->state_name_ << ")" << std::endl;
							myfile << "\t\t\t)" << std::endl;
							myfile << "\t\t)" << std::endl;
						}
						
						for (std::vector<Package*>::const_iterator ci = packages.begin(); ci != packages.end(); ++ci)
						{
							const Package* package = *ci;
							// Copy the states of where we are.
							myfile << "\t\t(when (and (part-of " << state->state_name_ << " ?old_kb) (at-oa " << package->name_ << " " << location->name_ << " " << state->state_name_ << ") (part-of " << state2->state_name_ << " ?new_kb))" << std::endl;
							myfile << "\t\t\t(and " << std::endl;
							myfile << "\t\t\t\t(not (Rat-oa " << package->name_ << " " << location->name_ << " " << state->state_name_ << "))" << std::endl;
							myfile << "\t\t\t\t(not (at-oa " << package->name_ << " " << location->name_ << " " << state->state_name_ << "))" << std::endl;
							myfile << "\t\t\t\t(Rat-oa " << package->name_ << " " << location->name_ << " " << state2->state_name_ << ")" << std::endl;
							myfile << "\t\t\t\t(at-oa " << package->name_ << " " << location->name_ << " " << state2->state_name_ << ")" << std::endl;
							myfile << "\t\t\t\t(in " << package->name_ << " " << state2->kb_->name_ << ")" << std::endl;
							myfile << "\t\t\t)" << std::endl;
							myfile << "\t\t)" << std::endl;
						}
						
						myfile << "\t\t(when (and (part-of " << state->state_name_ << " ?old_kb) (at-aa " << airplane.name_ << " " << location->name_ << " " << state->state_name_ << ") (part-of " << state2->state_name_ << " ?new_kb))" << std::endl;
						myfile << "\t\t\t(and " << std::endl;
						myfile << "\t\t\t\t(not (Rat-aa " << airplane.name_ << " " << location->name_ << " " << state->state_name_ << "))" << std::endl;
						myfile << "\t\t\t\t(not (at-aa " << airplane.name_ << " " << location->name_ << " " << state->state_name_ << "))" << std::endl;
						myfile << "\t\t\t\t(Rat-aa " << airplane.name_ << " " << location->name_ << " " << state2->state_name_ << ")" << std::endl;
						myfile << "\t\t\t\t(at-aa " << airplane.name_ << " " << location->name_ << " " << state2->state_name_ << ")" << std::endl;
						myfile << "\t\t\t)" << std::endl;
						myfile << "\t\t)" << std::endl;
					}
				}
				
				for (std::vector<Package*>::const_iterator ci = packages.begin(); ci != packages.end(); ++ci)
				{
					const Package* package = *ci;
					for (std::vector<Truck*>::const_iterator ci = trucks.begin(); ci != trucks.end(); ++ci)
					{
						const Truck* truck = *ci;
						myfile << "\t\t(when (and (part-of " << state->state_name_ << " ?old_kb) (in-ot " << package->name_ << " " << truck->name_ << " " << state->state_name_ << ") (part-of " << state2->state_name_ << " ?new_kb))" << std::endl;
						myfile << "\t\t\t(and " << std::endl;
						myfile << "\t\t\t\t(not (Rin-ot " << package->name_ << " " << truck->name_ << " " << state->state_name_ << "))" << std::endl;
						myfile << "\t\t\t\t(not (in-ot " << package->name_ << " " << truck->name_ << " " << state->state_name_ << "))" << std::endl;
						myfile << "\t\t\t\t(Rin-ot " << package->name_ << " " << truck->name_ << " " << state2->state_name_ << ")" << std::endl;
						myfile << "\t\t\t\t(in-ot " << package->name_ << " " << truck->name_ << " " << state2->state_name_ << ")" << std::endl;
						myfile << "\t\t\t\t(in " << package->name_ << " " << state2->kb_->name_ << ")" << std::endl;
						myfile << "\t\t\t)" << std::endl;
						myfile << "\t\t)" << std::endl;
					}
					
					myfile << "\t\t(when (and (part-of " << state->state_name_ << " ?old_kb) (in-oa " << package->name_ << " " << airplane.name_ << " " << state->state_name_ << ") (part-of " << state2->state_name_ << " ?new_kb))" << std::endl;
					myfile << "\t\t\t(and " << std::endl;
					myfile << "\t\t\t\t(not (Rin-oa " << package->name_ << " " << airplane.name_ << " " << state->state_name_ << "))" << std::endl;
					myfile << "\t\t\t\t(not (in-oa " << package->name_ << " " << airplane.name_ << " " << state->state_name_ << "))" << std::endl;
					myfile << "\t\t\t\t(Rin-oa " << package->name_ << " " << airplane.name_ << " " << state2->state_name_ << ")" << std::endl;
					myfile << "\t\t\t\t(in-oa " << package->name_ << " " << airplane.name_ << " " << state2->state_name_ << ")" << std::endl;
					myfile << "\t\t\t\t(in " << package->name_ << " " << state2->kb_->name_ << ")" << std::endl;
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
			const State* state = *ci;
			myfile << "\t\t(or " << std::endl;
			myfile << "\t\t\t(not (part-of " << state->state_name_ << " ?old_kb))" << std::endl;
			myfile << "\t\t\t(not (exists (?l - level ) (stack " << state->state_name_ << " ?l)))" << std::endl;
			myfile << "\t\t)" << std::endl;
		}
		
		// Force trucks to be at the same location.
		for (std::vector<Truck*>::const_iterator ci = trucks.begin(); ci != trucks.end(); ++ci)
		{
			Truck* truck = *ci;
			myfile << "\t\t(or" << std::endl;
			
			// Force the planner to have each truck, airplane, and package at the same location.
			for (std::vector<City*>::const_iterator ci = cities.begin(); ci != cities.end(); ++ci)
			{
				City* city = *ci;
				for (std::vector<Location*>::const_iterator ci = city->locations_.begin(); ci != city->locations_.end(); ++ci)
				{
					Location* location = *ci;
					myfile << "\t\t\t(and" << std::endl;
					for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
					{
						const State* state = *ci;
						myfile << "\t\t\t\t(or " << std::endl;
						myfile << "\t\t\t\t\t(not (part-of " << state->state_name_ << " ?old_kb))" << std::endl;
						myfile << "\t\t\t\t\t(at-tl " << truck->name_ << " " << location->name_ << " " << state->state_name_ << ")" << std::endl;
						myfile << "\t\t\t\t)" << std::endl;
					}
					myfile << "\t\t\t)"<< std::endl;
				}
				
				for (std::vector<Airport*>::const_iterator ci = city->airports_.begin(); ci != city->airports_.end(); ++ci)
				{
					Airport* location = *ci;
					myfile << "\t\t\t(and" << std::endl;
					for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
					{
						const State* state = *ci;
						myfile << "\t\t\t\t(or " << std::endl;
						myfile << "\t\t\t\t\t(not (part-of " << state->state_name_ << " ?old_kb))" << std::endl;
						myfile << "\t\t\t\t\t(at-ta " << truck->name_ << " " << location->name_ << " " << state->state_name_ << ")" << std::endl;
						myfile << "\t\t\t\t)" << std::endl;
					}
					myfile << "\t\t\t)" << std::endl;
				}
			}
			myfile << "\t\t)" << std::endl;
		}
		
		// Force the airplane to be at the same location.
		myfile << "\t\t(or" << std::endl;
		for (std::vector<City*>::const_iterator ci = cities.begin(); ci != cities.end(); ++ci)
		{
			City* city = *ci;
			
			for (std::vector<Airport*>::const_iterator ci = city->airports_.begin(); ci != city->airports_.end(); ++ci)
			{
				Airport* location = *ci;
				myfile << "\t\t\t(and" << std::endl;
				for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
				{
					const State* state = *ci;
					myfile << "\t\t\t\t(or " << std::endl;
					myfile << "\t\t\t\t\t(not (part-of " << state->state_name_ << " ?old_kb))" << std::endl;
					myfile << "\t\t\t\t\t(at-aa " << airplane.name_ << " " << location->name_ << " " << state->state_name_ << ")" << std::endl;
					myfile << "\t\t\t\t)" << std::endl;
				}
				myfile << "\t\t\t)" << std::endl;
			}
		}
		myfile << "\t\t)" << std::endl;
		
		// Force the package to be at the same location or in the same airplane / truck.
		for (std::vector<Package*>::const_iterator ci = packages.begin(); ci != packages.end(); ++ci)
		{
			Package* package = *ci;
			myfile << "\t\t(or" << std::endl;
			myfile << "\t\t\t(not (in " << package->name_ << " ?old_kb))" << std::endl;
			for (std::vector<City*>::const_iterator ci = cities.begin(); ci != cities.end(); ++ci)
			{
				City* city = *ci;
				/*
				for (std::vector<Location*>::const_iterator ci = city->locations_.begin(); ci != city->locations_.end(); ++ci)
				{
					Location* location = *ci;
					myfile << "\t\t\t(and" << std::endl;
					
					for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
					{
						const State* state = *ci;
						myfile << "\t\t\t\t(or " << std::endl;
						myfile << "\t\t\t\t\t(not (part-of " << state->state_name_ << " ?old_kb))" << std::endl;
						myfile << "\t\t\t\t\t(at-ol " << package->name_ << " " << location->name_ << " " << state->state_name_ << ")" << std::endl;
						myfile << "\t\t\t\t)" << std::endl;
					}
					myfile << "\t\t\t)" << std::endl;
				}
				*/
				for (std::vector<Airport*>::const_iterator ci = city->airports_.begin(); ci != city->airports_.end(); ++ci)
				{
					Airport* location = *ci;
					myfile << "\t\t\t(and" << std::endl;
					
					for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
					{
						const State* state = *ci;
						myfile << "\t\t\t\t(or " << std::endl;
						myfile << "\t\t\t\t\t(not (part-of " << state->state_name_ << " ?old_kb))" << std::endl;
						myfile << "\t\t\t\t\t(at-oa " << package->name_ << " " << location->name_ << " " << state->state_name_ << ")" << std::endl;
						myfile << "\t\t\t\t)" << std::endl;
					}
					myfile << "\t\t\t)" << std::endl;
				}
			}
			/*
			for (std::vector<Truck*>::const_iterator ci = trucks.begin(); ci != trucks.end(); ++ci)
			{
				Truck* truck = *ci;
				myfile << "\t\t\t(and" << std::endl;
					
				for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
				{
					const State* state = *ci;
					myfile << "\t\t\t\t(or " << std::endl;
					myfile << "\t\t\t\t\t(not (part-of " << state->state_name_ << " ?old_kb))" << std::endl;
					myfile << "\t\t\t\t\t(in-ot " << package->name_ << " " << truck->name_ << " " << state->state_name_ << ")" << std::endl;
					myfile << "\t\t\t\t)" << std::endl;
				}
				myfile << "\t\t\t)" << std::endl;
			}
			
			myfile << "\t\t\t(and" << std::endl;
			for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
			{
				const State* state = *ci;
				myfile << "\t\t\t\t(or " << std::endl;
				myfile << "\t\t\t\t\t(not (part-of " << state->state_name_ << " ?old_kb))" << std::endl;
				myfile << "\t\t\t\t\t(in-oa " << package->name_ << " " << airplane.name_ << " " << state->state_name_ << ")" << std::endl;
				myfile << "\t\t\t\t)" << std::endl;
			}
			myfile << "\t\t\t)" << std::endl;
			*/
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
			
		for (std::vector<City*>::const_iterator ci = cities.begin(); ci != cities.end(); ++ci)
		{
			const City* city = *ci;
			for (std::vector<Location*>::const_iterator ci = city->locations_.begin(); ci != city->locations_.end(); ++ci)
			{
				Location* location = *ci;
				for (std::vector<Truck*>::const_iterator ci = trucks.begin(); ci != trucks.end(); ++ci)
				{
					const Truck* truck = *ci;
					
					// at-tl
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
							myfile << "\t\t\t\t\t\t(at-tl " << truck->name_ << " " << location->name_ << " " << state2->state_name_ << ")" << std::endl;
							myfile << "\t\t\t\t\t)" << std::endl;
						}
						myfile << "\t\t\t\t\t(part-of " << state->state_name_ << " ?new_kb)" << std::endl;
						myfile << "\t\t\t)" << std::endl;

						myfile << "\t\t\t;; Conditional effects" << std::endl;
						myfile << "\t\t\t(and " << std::endl;
						for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
						{
							const State* state2 = *ci;
							myfile << "\t\t\t\t(not (at-tl " << truck->name_ << " " << location->name_ << " " << state2->state_name_ << "))" << std::endl;
						}
						myfile << "\t\t\t\t(at-tl " << truck->name_ << " " << location->name_ << " " << state->state_name_ << ")" << std::endl;
						myfile << "\t\t\t)" << std::endl;
						myfile << "\t\t)" << std::endl;
					}
				}
				
				// at-ol
				for (std::vector<Package*>::const_iterator ci = packages.begin(); ci != packages.end(); ++ci)
				{
					Package* package = *ci;
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
							myfile << "\t\t\t\t\t\t(at-ol " << package->name_ << " " << location->name_ << " " << state2->state_name_ << ")" << std::endl;
							myfile << "\t\t\t\t\t)" << std::endl;
						}
						myfile << "\t\t\t\t\t(part-of " << state->state_name_ << " ?new_kb)" << std::endl;
						myfile << "\t\t\t)" << std::endl;

						myfile << "\t\t\t;; Conditional effects" << std::endl;
						myfile << "\t\t\t(and " << std::endl;
						for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
						{
							const State* state2 = *ci;
							myfile << "\t\t\t\t(not (at-ol " << package->name_ << " " << location->name_ << " " << state2->state_name_ << "))" << std::endl;
							myfile << "\t\t\t\t(not (in " << package->name_ << " " << state2->kb_->name_ << "))" << std::endl;
						}
						myfile << "\t\t\t\t(at-ol " << package->name_ << " " << location->name_ << " " << state->state_name_ << ")" << std::endl;
						myfile << "\t\t\t\t(in " << package->name_ << " " << state->kb_->name_ << ")" << std::endl;
						myfile << "\t\t\t)" << std::endl;
						myfile << "\t\t)" << std::endl;
					}
				}
			}
			
			for (std::vector<Airport*>::const_iterator ci = city->airports_.begin(); ci != city->airports_.end(); ++ci)
			{
				Airport* location= *ci;
				for (std::vector<Truck*>::const_iterator ci = trucks.begin(); ci != trucks.end(); ++ci)
				{
					const Truck* truck = *ci;
					
					// at-ta
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
							myfile << "\t\t\t\t\t\t(at-ta " << truck->name_ << " " << location->name_ << " " << state2->state_name_ << ")" << std::endl;
							myfile << "\t\t\t\t\t)" << std::endl;
						}
						myfile << "\t\t\t\t\t(part-of " << state->state_name_ << " ?new_kb)" << std::endl;
						myfile << "\t\t\t)" << std::endl;

						myfile << "\t\t\t;; Conditional effects" << std::endl;
						myfile << "\t\t\t(and " << std::endl;
						for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
						{
							const State* state2 = *ci;
							myfile << "\t\t\t\t(not (at-ta " << truck->name_ << " " << location->name_ << " " << state2->state_name_ << "))" << std::endl;
						}
						myfile << "\t\t\t\t(at-ta " << truck->name_ << " " << location->name_ << " " << state->state_name_ << ")" << std::endl;
						myfile << "\t\t\t)" << std::endl;
						myfile << "\t\t)" << std::endl;
					}
				}
				
				// at-oa
				for (std::vector<Package*>::const_iterator ci = packages.begin(); ci != packages.end(); ++ci)
				{
					Package* package = *ci;
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
							myfile << "\t\t\t\t\t\t(at-oa " << package->name_ << " " << location->name_ << " " << state2->state_name_ << ")" << std::endl;
							myfile << "\t\t\t\t\t)" << std::endl;
						}
						myfile << "\t\t\t\t\t(part-of " << state->state_name_ << " ?new_kb)" << std::endl;
						myfile << "\t\t\t)" << std::endl;

						myfile << "\t\t\t;; Conditional effects" << std::endl;
						myfile << "\t\t\t(and " << std::endl;
						for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
						{
							const State* state2 = *ci;
							myfile << "\t\t\t\t(not (at-oa " << package->name_ << " " << location->name_ << " " << state2->state_name_ << "))" << std::endl;
							myfile << "\t\t\t\t(not (in " << package->name_ << " " << state2->kb_->name_ << "))" << std::endl;
						}
						myfile << "\t\t\t\t(at-oa " << package->name_ << " " << location->name_ << " " << state->state_name_ << ")" << std::endl;
						myfile << "\t\t\t\t(in " << package->name_ << " " << state->kb_->name_ << ")" << std::endl;
						myfile << "\t\t\t)" << std::endl;
						myfile << "\t\t)" << std::endl;
					}
				}
				
				// at-aa
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
						myfile << "\t\t\t\t\t\t(at-aa " << airplane.name_ << " " << location->name_ << " " << state2->state_name_ << ")" << std::endl;
						myfile << "\t\t\t\t\t)" << std::endl;
					}
					myfile << "\t\t\t\t\t(part-of " << state->state_name_ << " ?new_kb)" << std::endl;
					myfile << "\t\t\t)" << std::endl;

					myfile << "\t\t\t;; Conditional effects" << std::endl;
					myfile << "\t\t\t(and " << std::endl;
					for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
					{
						const State* state2 = *ci;
						myfile << "\t\t\t\t(not (at-aa " << airplane.name_ << " " << location->name_ << " " << state2->state_name_ << "))" << std::endl;
					}
					myfile << "\t\t\t\t(at-aa " << airplane.name_ << " " << location->name_ << " " << state->state_name_ << ")" << std::endl;
					myfile << "\t\t\t)" << std::endl;
					myfile << "\t\t)" << std::endl;
				}
			}
		}
		
		for (std::vector<Package*>::const_iterator ci = packages.begin(); ci != packages.end(); ++ci)
		{
			Package* package = *ci;
			// in-ot
			for (std::vector<Truck*>::const_iterator ci = trucks.begin(); ci != trucks.end(); ++ci)
			{
				Truck* truck = *ci;
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
						myfile << "\t\t\t\t\t\t(in-ot " << package->name_ << " " << truck->name_ << " " << state2->state_name_ << ")" << std::endl;
						myfile << "\t\t\t\t\t)" << std::endl;
					}
					myfile << "\t\t\t\t\t(part-of " << state->state_name_ << " ?new_kb)" << std::endl;
					myfile << "\t\t\t)" << std::endl;

					myfile << "\t\t\t;; Conditional effects" << std::endl;
					myfile << "\t\t\t(and " << std::endl;
					for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
					{
						const State* state2 = *ci;
						myfile << "\t\t\t\t(not (in-ot " << package->name_ << " " << truck->name_ << " " << state2->state_name_ << "))" << std::endl;
						myfile << "\t\t\t\t(not (in " << package->name_ << " " << state2->kb_->name_ << "))" << std::endl;
					}
					myfile << "\t\t\t\t(in-ot " << package->name_ << " " << truck->name_ << " " << state->state_name_ << ")" << std::endl;
					myfile << "\t\t\t\t(in " << package->name_ << " " << state->kb_->name_ << ")" << std::endl;
					myfile << "\t\t\t)" << std::endl;
					myfile << "\t\t)" << std::endl;
				}
			}
			
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
					myfile << "\t\t\t\t\t\t(in-oa " << package->name_ << " " << airplane.name_ << " " << state2->state_name_ << ")" << std::endl;
					myfile << "\t\t\t\t\t)" << std::endl;
				}
				myfile << "\t\t\t\t\t(part-of " << state->state_name_ << " ?new_kb)" << std::endl;
				myfile << "\t\t\t)" << std::endl;

				myfile << "\t\t\t;; Conditional effects" << std::endl;
				myfile << "\t\t\t(and " << std::endl;
				for (std::vector<const State*>::const_iterator ci = states.begin(); ci != states.end(); ++ci)
				{
					const State* state2 = *ci;
					myfile << "\t\t\t\t(not (in-oa " << package->name_ << " " << airplane.name_ << " " << state2->state_name_ << "))" << std::endl;
					myfile << "\t\t\t\t(not (in " << package->name_ << " " << state2->kb_->name_ << "))" << std::endl;
				}
				myfile << "\t\t\t\t(in-oa " << package->name_ << " " << airplane.name_ << " " << state->state_name_ << ")" << std::endl;
				myfile << "\t\t\t\t(in " << package->name_ << " " << state->kb_->name_ << ")" << std::endl;
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
	if (argc < 6)
	{
		std::cout << "Usage: <number cities> <number locations per city> <number of airports per city> <number of trucks per city> <number of packages per city> {-f}" << std::endl;
		return -1;
	}
	
	unsigned int nr_cities = ::atoi(argv[1]);
	unsigned int nr_locations_per_city = ::atoi(argv[2]);
	unsigned int nr_airports_per_city = ::atoi(argv[3]);
	unsigned int nr_trucks_per_city = ::atoi(argv[4]);
	unsigned int nr_packages_per_city = ::atoi(argv[5]);
	
	enum MODE { ORIGINAL, FACTORISED, PRP};
	MODE mode = ORIGINAL;
	
	if (argc == 6)
	{
		mode = ORIGINAL;
		std::cout << "ORIGINAL" << std::endl;
	}
	else if (std::string(argv[6]) == "-f")
	{
		mode = FACTORISED;
		std::cout << "FACTORISED" << std::endl;
	}
	else if (std::string(argv[6]) == "-p")
	{
		mode = PRP;
		std::cout << "PRP" << std::endl;
	}
	else
	{
		std::cerr << "Unknown option " << argv[6] << std::endl;
		exit(-1);
	}
	
	// Create the cities.
	std::vector<City*> cities;
	std::vector<Truck*> trucks;
	std::vector<Package*> packages;
	std::stringstream ss;
	for (unsigned int city_nr = 0; city_nr < nr_cities; ++city_nr)
	{
		ss.str(std::string());
		ss << "city_" << city_nr;
		City* c = new City(ss.str());
		cities.push_back(c);
		
		std::cout << "Creating city " << ss.str() << std::endl;
		
		for (unsigned int location_nr = 0; location_nr < nr_locations_per_city; ++location_nr)
		{
			ss.str(std::string());
			ss << "loc_" << location_nr << "_city_" << city_nr;
			Location* loc = new Location(ss.str(), *c);
			c->locations_.push_back(loc);
		}
		std::cout << "..." << c->locations_.size() << " Locations created!" << std::endl;
		
		for (unsigned int airport_nr = 0; airport_nr < nr_airports_per_city; ++airport_nr)
		{
			ss.str(std::string());
			ss << "airport_" << airport_nr << "_city_" << city_nr;
			Airport* airport = new Airport(ss.str(), *c);
			c->airports_.push_back(airport);
		}
		std::cout << "..." << c->airports_.size() << " Airports created!" << std::endl;
		
		for (unsigned int truck_nr = 0; truck_nr < nr_trucks_per_city; ++truck_nr)
		{
			ss.str(std::string());
			ss << "truck_" << truck_nr << "_city_" << city_nr;
			
			StoragePlace* truck_location = NULL;
			int location = rand() % (nr_locations_per_city);
			truck_location = c->locations_[location];
			Truck* truck = new Truck(ss.str(), *truck_location);
			trucks.push_back(truck);
		}
		std::cout << "..." << trucks.size() << " Trucks created!" << std::endl;
	}
	
	for (unsigned int city_nr = 0; city_nr < nr_cities; ++city_nr)
	{	
		for (unsigned int package_nr = 0; package_nr < nr_packages_per_city; ++package_nr)
		{
			ss.str(std::string());
			ss << "package_" << cities[city_nr]->name_ << "_" << package_nr;
			Package* package = new Package(ss.str(), *cities[city_nr], *cities[(city_nr + 1) % nr_cities]->airports_[0]);
			packages.push_back(package);
		}
	}
	std::cout << "..." << packages.size() << " Packages created!" << std::endl;
	
	// Place the airplane somewhere.
	Airplane* airplane = NULL;
	if (nr_airports_per_city > 0 && nr_cities > 0)
	{
		airplane = new Airplane("airplane0", *cities[0]->airports_[0]);
	}
	
	std::cout << "Creating all possible states..." << std::endl;
	if (mode == FACTORISED)
	{
		std::map<Package*, Location*> no_packages;
		State basic_state("basic", no_packages);
		
		std::vector<const KnowledgeBase*> knowledge_bases;
		KnowledgeBase basis_kb("basis_kb");
		basis_kb.addState(basic_state);
		knowledge_bases.push_back(&basis_kb);
		
		// Create a new knowledge base for each package.
		for (std::vector<Package*>::const_iterator ci = packages.begin(); ci != packages.end(); ++ci)
		{
			Package* package = *ci;
			ss.str(std::string());
			ss << "kb_" << package->name_;
			
			KnowledgeBase* kb = new KnowledgeBase(ss.str());
			basis_kb.addChild(*kb);
			knowledge_bases.push_back(kb);
			
			for (std::vector<Location*>::const_iterator ci = package->starting_city_->locations_.begin(); ci != package->starting_city_->locations_.end(); ++ci)
			{
				Location* location = *ci;
				ss.str(std::string());
				ss << "s_" << package->name_ << "_" <<location->name_;
				
				std::map<Package*, Location*> package_location_mapping;
				package_location_mapping[package] = location;
				State* state = new State(ss.str(), package_location_mapping);
				kb->addState(*state);
			}
		}
		
		std::cout << "Generate domain..." << std::endl;
		generateDomain("test_domain.pddl", basis_kb, knowledge_bases, cities, trucks, *airplane, packages, true);
		std::cout << "Generate problem..." << std::endl;
		generateProblem("test_problem.pddl", basis_kb, knowledge_bases, cities, trucks, *airplane, packages, true);
	}
	else if (mode == ORIGINAL)
	{
		// Enumerate all the possible states.
		std::vector<int> package_locations(nr_cities * nr_packages_per_city, 0);
		
		bool done = false;
		unsigned int state_id = 0;
		std::stringstream ss;
		
		KnowledgeBase basis_kb("basis_kb");
		std::vector<const KnowledgeBase*> knowledge_bases;
		knowledge_bases.push_back(&basis_kb);
		
		while (!done)
		{
			std::map<Package*, Location*> package_location_mapping;
			for (unsigned int package_id = 0; package_id < packages.size(); ++package_id)
			{
				Package* package = packages[package_id];
				package_location_mapping[package] = package->starting_city_->locations_[package_locations[package_id]];
			}
			
			ss.str(std::string());
			ss << "s" << state_id;
			State* state = new State(ss.str(), package_location_mapping);
			basis_kb.addState(*state);
			++state_id;
			
			done = true;
			for (unsigned int i = 0; i < package_locations.size(); ++i)
			{
				if (package_locations[i] + 1 != nr_locations_per_city)
				{
					package_locations[i] = package_locations[i] + 1;
					done = false;
					break;
				}
				package_locations[i] = 0;
			}
		}
		
		std::cout << "Created " << basis_kb.states_.size() << " states." << std::endl;
		
		
		
		std::cout << "Generate domain..." << std::endl;
		generateDomain("test_domain.pddl", basis_kb, knowledge_bases, cities, trucks, *airplane, packages, false);
		std::cout << "Generate problem..." << std::endl;
		generateProblem("test_problem.pddl", basis_kb, knowledge_bases, cities, trucks, *airplane, packages, false);
	}
	else
	{
		std::cout << "Generate domain..." << std::endl;
		generateDomain("test_domain.pddl", cities, trucks, *airplane, packages);
		std::cout << "Generate problem..." << std::endl;
		generateProblem("test_problem.pddl", cities, trucks, *airplane, packages);
	}

	
	return 0;
}
