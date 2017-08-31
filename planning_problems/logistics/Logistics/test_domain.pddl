(define (domain logistics)
(:requirements :typing)

(:types
	obj truck location airplane city airport
)

(:predicates
	(at-tl ?obj - truck ?loc - location)
	(at-ta ?obj - truck ?loc - airport)
	(at-aa ?obj - airplane ?loc - airport)
	(at-ol ?obj - obj ?loc - location)
	(at-oa ?obj - obj ?loc - airport)
	(in-ot ?obj1 - obj ?obj2 - truck)
	(in-oa ?obj1 - obj ?obj2 - airplane)
	(in-city-l ?loc - location ?city - city)
	(in-city-a ?loc - airport ?city - city)
	(in-city-t ?loc - truck ?city - city)
)

;; Sense if a package is at the same location as the truck.
(:action sense-package-loc-t
	:parameters (?obj - obj ?loc - location ?truck - truck)
	:precondition (and
		(at-tl ?truck ?loc)
	)
	:observe (at-ol ?obj ?loc)
)

;; Sense if a package is at the same airport as the truck.
(:action sense-package-ap-t
	:parameters (?obj - obj ?loc - airport ?truck - truck)
	:precondition (and
		(at-ta ?truck ?loc)
	)
	:observe (at-oa ?obj ?loc)
)

;; Sense if a package is at the same airport as the plane.
(:action sense-package-ap-a
	:parameters (?obj - obj ?loc - airport ?airplane - airplane)
	:precondition (and
		(at-aa ?airplane ?loc)
	)
	:observe (at-oa ?obj ?loc)
)

;; Load the truck.
(:action load-truck-loc
	:parameters (?obj - obj ?truck - truck ?loc - location ?city - city)
	:precondition (and
		(in-city-t ?truck ?city)
		(in-city-l ?loc ?city)
		(at-tl ?truck ?loc)
		(at-ol ?obj ?loc)
	)
	:effect (and
				(not (at-ol ?obj ?loc))
				(in-ot ?obj ?truck)
	)
)

;; Load the truck.
(:action load-truck-ap
	:parameters (?obj - obj ?truck - truck ?loc - airport ?city - city)
	:precondition (and
		(in-city-t ?truck ?city)
		(in-city-a ?loc ?city)
		(at-ta ?truck ?loc)
		(at-oa ?obj ?loc)
	)
	:effect (and
				(not (at-oa ?obj ?loc))
				(in-ot ?obj ?truck)
	)
)

;; Load the airplane.
(:action load-airplane
	:parameters (?obj - obj ?airplane - airplane ?loc - airport ?city - city)
	:precondition (and
		(at-aa ?airplane ?loc)
		(at-oa ?obj ?loc)
	)
	:effect (and
		(not (at-oa ?obj ?loc))
		(in-oa ?obj ?airplane)
	)
)

;; Unload the truck.
(:action unload-truck-loc
	:parameters (?obj - obj ?truck - truck ?loc - location ?city - city)
	:precondition (and
		(in-city-t ?truck ?city)
		(in-city-l ?loc ?city)
		(at-tl ?truck ?loc)
		(in-ot ?obj ?truck)
	)
	:effect (and
		(not (in-ot ?obj ?truck))
		(at-ol ?obj ?loc)
	)
)

;; Unload the truck at an airport.
(:action unload-truck-ap
	:parameters (?obj - obj ?truck - truck ?loc - airport ?city - city)
	:precondition (and
		(in-city-t ?truck ?city)
		(in-city-a ?loc ?city)
		(at-ta ?truck ?loc)
		(in-ot ?obj ?truck)
	)
	:effect (and
		(not (in-ot ?obj ?truck))
		(at-oa ?obj ?loc)
	)
)

;; Unload an airplane.
(:action unload-airplane
	:parameters (?obj - obj ?airplane - airplane ?loc - airport)
	:precondition (and
		(at-aa ?airplane ?loc)
		(in-oa ?obj ?airplane)
	)
	:effect (and
		(not (in-oa ?obj ?airplane))
		(at-oa ?obj ?loc)
	)
)

;; Drive a truck to an airport.
(:action drive-truck-loc-ap
	:parameters (?truck - truck ?loc1 - location ?loc2 - airport ?city - city)
	:precondition (and
		(in-city-l ?loc1 ?city)
		(in-city-a ?loc2 ?city)
		(in-city-t ?truck ?city)
		(at-tl ?truck ?loc1)
	)
	:effect (and
		(not (at-tl ?truck ?loc1))
		(at-ta ?truck ?loc2)
	)
)

;; Drive a truck to a location.
(:action drive-truck-ap-loc
	:parameters (?truck - truck ?loc1 - airport ?loc2 - location ?city - city)
	:precondition (and
		(in-city-a ?loc1 ?city)
		(in-city-l ?loc2 ?city)
		(in-city-t ?truck ?city)
		(at-ta ?truck ?loc1)
	)
	:effect (and
		(not (at-ta ?truck ?loc1))
		(at-tl ?truck ?loc2)
	)
)

;; Fly an airplane.
(:action fly-airplane
	:parameters (?airplane - airplane ?loc1 ?loc2 - airport)
	:precondition (and
		(at-aa ?airplane ?loc1)
	)
	:effect (and
		(not (at-aa ?airplane ?loc1))
		(at-aa ?airplane ?loc2)
	)
)

)
