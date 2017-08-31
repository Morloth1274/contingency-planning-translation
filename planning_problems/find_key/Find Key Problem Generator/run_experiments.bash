#!/bin/bash

#bram@ubuntu:~/projects/factorised_contingent_planning/planning_problems/logistics/Logistics$ ./build/logistics 2 2 1 1 1 
#Creating a logistics problem with 2 cities, with 2 locations, 1 airports, 1 trucks, and 1 packages per city.

for nr_keys in {4..5}
do
	for nr_locations in {1..5}
	do
		# Build the domain and problem file for the non-factorised domain.
		#echo "Create a domain with $nr_keys keys, each having $nr_locations locations!\n"
		#./build/generator ${nr_keys} ${nr_locations} > /dev/null
		
		# Execute the planner, we give it 30 minutes per problem and 2BG of memory
		#echo "Plan for this logistics problem, non factorised...\n"
		#perl constraint.perl -t 1800 -m 2097152 ~/planners/original/FF-X/ff -o test_domain.pddl -f test_problem.pddl > results/original_${nr_keys}_${nr_locations}.plan

		# Build the domain and problem file for the factorised domain.
		echo "Create a factorised domain with $nr_keys keys, each having $nr_locations locations!\n"
		./build/generator ${nr_keys} ${nr_locations} -f > /dev/null
		
		# Execute the planner, we give it 30 minutes per problem and 2BG of memory
		echo "Plan for this grid, factorised...\n"
		perl constraint.perl -t 1800 -m 2097152 ~/planners/original/FF-X/ff -o test_domain.pddl -f test_problem.pddl > results/factorised_${nr_keys}_${nr_locations}.plan
	done
done

