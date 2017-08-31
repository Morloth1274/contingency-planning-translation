#!/bin/bash

main=`pwd`
poprp="/home/bram/projects/factorised_contingent_planning/po-prp/planner-for-relevant-policies/src/"
clg="/home/bram/planners/clg/CLG_cluster/"

echo $main
echo $poprp

for x in {15..15}
do
	for y in {1..9..2}
	do
		# Build the domain and problem file for the non-factorised domain.
#		echo "Create a domain with a [$x, $y] grid!\n"
#		./build/sliding_doors ${x} ${y} > /dev/null
		
		# Execute the planner, we give it 30 minutes per problem and 2GB of memory
#		echo "Plan for this grid, non factorised...\n"
#		perl constraint.perl -t 1800 -m 2097152 ~/planners/original/FF-X/ff -o test_domain.pddl -f test_problem.pddl > results/original_${x}_${y}.plan

		# Build the domain and problem file for the factorised domain.
#		echo "Create a factorised domain with a [$x, $y] grid!\n"
#		./build/sliding_doors ${x} ${y} -f > /dev/null
		
		# Execute the planner, we give it 30 minutes per problem and 2GB of memory
#		echo "Plan for this grid, factorised...\n"
#		perl constraint.perl -t 1800 -m 2097152 ~/planners/original/FF-X/ff -o test_domain.pddl -f test_problem.pddl > results/factorised_${x}_${y}.plan

		# Build the domain and problem file using CLG translation for PO-PRP
		echo "Create a factorised domain with a [$x, $y] grid!\n"
		./build/sliding_doors ${x} ${y} -p > /dev/null

		# Execute the planner, we give it 30 minutes per problem and 2GB of memory
		echo "POPRP\n"
		cd $poprp; perl $main/constraint.perl -t 1800 -m 2097152 ./poprp $main/test_domain.pddl $main/test_problem.pddl > $main/results/poprp_${x}_${y}.plan; cd $main

		echo "CLG...\n"
		cd $clg; perl $main/constraint.perl -t 1800 -m 2097152 ./run-clg.sh -1 $main/test_domain.pddl $main/test_problem.pddl | grep "total time\|number of actions" > $main/results/clg_${x}_${y}.plan; cd $main
	done
done

