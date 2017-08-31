#!/bin/bash

#bram@ubuntu:~/projects/factorised_contingent_planning/planning_problems/logistics/Logistics$ ./build/logistics 2 2 1 1 1 
#Creating a logistics problem with 2 cities, with 2 locations, 1 airports, 1 trucks, and 1 packages per city.
main=`pwd`
clg="/home/bram/planners/clg/CLG_cluster/"
poprp="/home/bram/projects/factorised_contingent_planning/po-prp/planner-for-relevant-policies/src/"

for nr_cities in {1..2}
do
	for nr_locations in {1..3}
	do
		for nr_packages in {1..3}
		do
			
			# Build the domain and problem file for the non-factorised domain.
			echo "Create a domain with $nr_cities cities, each having $nr_locations locations and $nr_packages packages per city to be delivered!\n"
			#./build/logistics ${nr_cities} ${nr_locations} 1 1 ${nr_packages} > /dev/null
		
			# Execute the planner, we give it 30 minutes per problem and 2BG of memory
			#echo "Plan for this logistics problem, non factorised...\n"
			#perl constraint.perl -t 1800 -m 2097152 ~/planners/original/FF-X/ff -o test_domain.pddl -f test_problem.pddl > results/original_${nr_cities}_${nr_locations}_${nr_packages}.plan

			# Build the domain and problem file for the factorised domain.
#			echo "Create a factorised domain with $nr_cities cities, each having $nr_locations locations and $nr_packages packages per city to be delivered!\n"
#			./build/logistics ${nr_cities} ${nr_locations} 1 1 ${nr_packages} -f > /dev/null
		
			# Execute the planner, we give it 30 minutes per problem and 2BG of memory
#			echo "Plan for this grid, factorised...\n"
#			perl constraint.perl -t 1800 -m 2097152 ~/planners/original/FF-X/ff -o test_domain.pddl -f test_problem.pddl > results/factorised_${nr_cities}_${nr_locations}_${nr_packages}.plan

			./build/logistics ${nr_cities} ${nr_locations} 1 1 ${nr_packages} -p > /dev/null

			# Execute the planner, we give it 30 minutes per problem and 2GB of memory
			echo "PRP-PO...\n"
			cd $poprp; perl $main/constraint.perl -t 1800 -m 2097152 ./poprp $main/test_domain.pddl $main/test_problem.pddl > $main/results/poprp_${nr_cities}_${nr_locations}_${nr_packages}.plan; cd $main

			echo "CLG...\n"
			cd $clg; perl $main/constraint.perl -t 1800 -m 2097152 ./run-clg.sh -1 $main/test_domain.pddl $main/test_problem.pddl | grep "action\|total time\|number of actions" > $main/results/clg_${nr_cities}_${nr_locations}_${nr_packages}.plan; cd $main
		done
	done
done

