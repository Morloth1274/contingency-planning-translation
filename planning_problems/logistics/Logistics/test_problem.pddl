(define (problem logistics-problem)
(:domain logistics)
(:objects
	package_city_0_0 - obj
	package_city_0_1 - obj
	package_city_0_2 - obj
	package_city_1_0 - obj
	package_city_1_1 - obj
	package_city_1_2 - obj
	truck_0_city_0 - truck
	truck_0_city_1 - truck
	airplane0 - airplane
	city_0 - city
	loc_0_city_0 -location
	loc_1_city_0 -location
	loc_2_city_0 -location
	airport_0_city_0 -airport
	city_1 - city
	loc_0_city_1 -location
	loc_1_city_1 -location
	loc_2_city_1 -location
	airport_0_city_1 -airport
)

(:init
	(in-city-a airport_0_city_0 city_0)
	(in-city-l loc_0_city_0 city_0)
	(in-city-l loc_1_city_0 city_0)
	(in-city-l loc_2_city_0 city_0)
	(in-city-a airport_0_city_1 city_1)
	(in-city-l loc_0_city_1 city_1)
	(in-city-l loc_1_city_1 city_1)
	(in-city-l loc_2_city_1 city_1)
	(in-city-t truck_0_city_0 city_0)
	(at-tl truck_0_city_0 loc_1_city_0)
	(in-city-t truck_0_city_1 city_1)
	(at-tl truck_0_city_1 loc_1_city_1)
	(at-aa airplane0 airport_0_city_0)
	(oneof 
	(at-ol package_city_0_0 loc_0_city_0)
	(at-ol package_city_0_0 loc_1_city_0)
	(at-ol package_city_0_0 loc_2_city_0)
	(at-ol package_city_0_0 loc_0_city_1)
	(at-ol package_city_0_0 loc_1_city_1)
	(at-ol package_city_0_0 loc_2_city_1)
	)
	(oneof 
	(at-ol package_city_0_1 loc_0_city_0)
	(at-ol package_city_0_1 loc_1_city_0)
	(at-ol package_city_0_1 loc_2_city_0)
	(at-ol package_city_0_1 loc_0_city_1)
	(at-ol package_city_0_1 loc_1_city_1)
	(at-ol package_city_0_1 loc_2_city_1)
	)
	(oneof 
	(at-ol package_city_0_2 loc_0_city_0)
	(at-ol package_city_0_2 loc_1_city_0)
	(at-ol package_city_0_2 loc_2_city_0)
	(at-ol package_city_0_2 loc_0_city_1)
	(at-ol package_city_0_2 loc_1_city_1)
	(at-ol package_city_0_2 loc_2_city_1)
	)
	(oneof 
	(at-ol package_city_1_0 loc_0_city_0)
	(at-ol package_city_1_0 loc_1_city_0)
	(at-ol package_city_1_0 loc_2_city_0)
	(at-ol package_city_1_0 loc_0_city_1)
	(at-ol package_city_1_0 loc_1_city_1)
	(at-ol package_city_1_0 loc_2_city_1)
	)
	(oneof 
	(at-ol package_city_1_1 loc_0_city_0)
	(at-ol package_city_1_1 loc_1_city_0)
	(at-ol package_city_1_1 loc_2_city_0)
	(at-ol package_city_1_1 loc_0_city_1)
	(at-ol package_city_1_1 loc_1_city_1)
	(at-ol package_city_1_1 loc_2_city_1)
	)
	(oneof 
	(at-ol package_city_1_2 loc_0_city_0)
	(at-ol package_city_1_2 loc_1_city_0)
	(at-ol package_city_1_2 loc_2_city_0)
	(at-ol package_city_1_2 loc_0_city_1)
	(at-ol package_city_1_2 loc_1_city_1)
	(at-ol package_city_1_2 loc_2_city_1)
	)
)
(:goal (and
	(at-oa package_city_0_0 airport_0_city_1)
	(at-oa package_city_0_1 airport_0_city_1)
	(at-oa package_city_0_2 airport_0_city_1)
	(at-oa package_city_1_0 airport_0_city_0)
	(at-oa package_city_1_1 airport_0_city_0)
	(at-oa package_city_1_2 airport_0_city_0)
)
)
)
