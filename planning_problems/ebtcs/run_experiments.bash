#!/bin/bash

main=`pwd`
poprp="/home/bram/projects/factorised_contingent_planning/po-prp/planner-for-relevant-policies/src/"
clg="/home/bram/planners/clg/CLG_cluster/"

for bombs in {1..5}
do
	for packages in {2..5}
	do
		if [ $bombs -eq 1 -o $packages -eq 1 ];
		then
		# Build the domain and problem file for the non-factorised domain.
		echo "Create a domain with ${bombs} bombs and ${packages} packages!\n"
#		./build/ebtcs ${bombs} ${packages} > /dev/null
		
		# Execute the planner, we give it 30 minutes per problem and 2BG of memory
#		echo " -- ORIGNAL...\n"
#		perl constraint.perl -t 1800 -m 2097152 ~/planners/original/FF-X/ff -o test_domain.pddl -f test_problem.pddl > results/original_${bombs}_${packages}.plan

		# Build the domain and problem file for the factorised domain.
#		./build/ebtcs ${bombs} ${packages} -f > /dev/null
		
		# Execute the planner, we give it 30 minutes per problem and 2BG of memory
#		echo " -- FACTORISED...\n"
#		perl constraint.perl -t 1800 -m 2097152 ~/planners/original/FF-X/ff -o test_domain.pddl -f test_problem.pddl > results/factorised_${bombs}_${packages}.plan

		echo "PRP-PO...\n"
		./build/ebtcs ${bombs} ${packages} -p > /dev/null

		# Execute the planner, we give it 30 minutes per problem and 2GB of memory
		cd $poprp; perl $main/constraint.perl -t 1800 -m 2097152 ./poprp $main/test_domain.pddl $main/test_problem.pddl > $main/results/poprp_${bombs}_${packages}.plan; cd $main

		echo "CLG...\n"
		cd $clg; perl $main/constraint.perl -t 1800 -m 2097152 ./run-clg.sh -1 $main/test_domain.pddl $main/test_problem.pddl | grep "total time\|number of actions" > $main/results/clg_${bombs}_${packages}.plan; cd $main
		fi
	done
done

