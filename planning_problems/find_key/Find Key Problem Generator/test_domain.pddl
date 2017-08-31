(define (domain find_key)
(:requirements :typing :conditional-effects :negative-preconditions :disjunctive-preconditions)

(:types
	waypoint
	robot key - locatable
	level
	state
	knowledgebase
)

(:predicates
	(at ?l - locatable ?wp - waypoint ?s - state)
	(Rat ?l - locatable ?wp - waypoint ?s - state)
	(has ?k - key ?s - state)
	(Rhas ?k - key ?s - state)

	;; Bookkeeping predicates.
	(next ?l ?l2 - level)
	(lev ?l - LEVEL)
	(m ?s - STATE)
	(stack ?s - STATE ?l - LEVEL)
	(resolve-axioms)
)

(:constants
	; All the waypoints.
	wp0_for_key0 - waypoint
	wp1_for_key0 - waypoint
	wp2_for_key0 - waypoint
	wp3_for_key0 - waypoint
	wp4_for_key0 - waypoint
	start_wp - waypoint
	; The keys.
	key0 - key
	; The robot.
	robot - robot
	; All the states.
	s0 - state
	s1 - state
	s2 - state
	s3 - state
	s4 - state
)

;; Action to move the Robot.
(:action navigate
	:parameters (?r - robot ?from ?to - waypoint)
	:precondition (and
		(not (resolve-axioms))
		;; For every state ?s
		(Rat ?r ?from s0)
		(Rat ?r ?from s1)
		(Rat ?r ?from s2)
		(Rat ?r ?from s3)
		(Rat ?r ?from s4)
	)
	:effect (and
		;; For every state ?s
		(when (m s0)
			(and
				(not (at ?r ?from s0))
				(at ?r ?to s0)
				(not (Rat ?r ?from s0))
				(Rat ?r ?to s0)
			)
		)
		(when (m s1)
			(and
				(not (at ?r ?from s1))
				(at ?r ?to s1)
				(not (Rat ?r ?from s1))
				(Rat ?r ?to s1)
			)
		)
		(when (m s2)
			(and
				(not (at ?r ?from s2))
				(at ?r ?to s2)
				(not (Rat ?r ?from s2))
				(Rat ?r ?to s2)
			)
		)
		(when (m s3)
			(and
				(not (at ?r ?from s3))
				(at ?r ?to s3)
				(not (Rat ?r ?from s3))
				(Rat ?r ?to s3)
			)
		)
		(when (m s4)
			(and
				(not (at ?r ?from s4))
				(at ?r ?to s4)
				(not (Rat ?r ?from s4))
				(Rat ?r ?to s4)
			)
		)
	)
)

;; Pick up a key.
(:action pickup
	:parameters (?k - key ?r - robot ?wp - waypoint)
	:precondition (and
		(not (resolve-axioms))
		;; For every state ?s
		(Rat ?r ?wp s0)
		(Rat ?k ?wp s0)
		;; For every state ?s
		(Rat ?r ?wp s1)
		(Rat ?k ?wp s1)
		;; For every state ?s
		(Rat ?r ?wp s2)
		(Rat ?k ?wp s2)
		;; For every state ?s
		(Rat ?r ?wp s3)
		(Rat ?k ?wp s3)
		;; For every state ?s
		(Rat ?r ?wp s4)
		(Rat ?k ?wp s4)
	)
	:effect (and
		;; For every state s0
		(when (m s0)
			(and
				(not (at ?k ?wp s0))
				(has ?k s0)

				;; This optimises the search as it does not require a raminificate action to discover these facts.
				(not (Rat ?k ?wp s0))
				(Rhas ?k s0)
			)
		)
		;; For every state s1
		(when (m s1)
			(and
				(not (at ?k ?wp s1))
				(has ?k s1)

				;; This optimises the search as it does not require a raminificate action to discover these facts.
				(not (Rat ?k ?wp s1))
				(Rhas ?k s1)
			)
		)
		;; For every state s2
		(when (m s2)
			(and
				(not (at ?k ?wp s2))
				(has ?k s2)

				;; This optimises the search as it does not require a raminificate action to discover these facts.
				(not (Rat ?k ?wp s2))
				(Rhas ?k s2)
			)
		)
		;; For every state s3
		(when (m s3)
			(and
				(not (at ?k ?wp s3))
				(has ?k s3)

				;; This optimises the search as it does not require a raminificate action to discover these facts.
				(not (Rat ?k ?wp s3))
				(Rhas ?k s3)
			)
		)
		;; For every state s4
		(when (m s4)
			(and
				(not (at ?k ?wp s4))
				(has ?k s4)

				;; This optimises the search as it does not require a raminificate action to discover these facts.
				(not (Rat ?k ?wp s4))
				(Rhas ?k s4)
			)
		)
	)
)

;; Sense for the key.
(:action sense
	:parameters (?k - key ?r - robot ?wp - waypoint ?l ?l2 - level)
	:precondition (and
		(not (resolve-axioms))
		(next ?l ?l2)
		(lev ?l)

		;; For every state ?s
		(Rat ?r ?wp s0)

		;; For every state ?s
		(Rat ?r ?wp s1)

		;; For every state ?s
		(Rat ?r ?wp s2)

		;; For every state ?s
		(Rat ?r ?wp s3)

		;; For every state ?s
		(Rat ?r ?wp s4)

		;; This action is only applicable if there are world states where the outcome can be different.
		(exists (?s - state) (and (m ?s) (Rat ?k ?wp ?s)))
		(exists (?s - state) (and (m ?s) (not (Rat ?k ?wp ?s))))
	)
	:effect (and
		(not (lev ?l))
		(lev ?l2)

		;; For every state ?s
		(when (and (m s0) (not (Rat ?k ?wp s0)))
			(and (stack s0 ?l) (not (m s0)))
		)
		(when (and (m s1) (not (Rat ?k ?wp s1)))
			(and (stack s1 ?l) (not (m s1)))
		)
		(when (and (m s2) (not (Rat ?k ?wp s2)))
			(and (stack s2 ?l) (not (m s2)))
		)
		(when (and (m s3) (not (Rat ?k ?wp s3)))
			(and (stack s3 ?l) (not (m s3)))
		)
		(when (and (m s4) (not (Rat ?k ?wp s4)))
			(and (stack s4 ?l) (not (m s4)))
		)
		(resolve-axioms)
	)
)

;; Exit the current branch.
(:action pop
	:parameters (?l ?l2 - level)
	:precondition (and
		(lev ?l)
		(next ?l2 ?l)
		(not (resolve-axioms))
	)
	:effect (and 
		(not (lev ?l))
		(lev ?l2)
		(resolve-axioms)
		(when (m s0) 
			(not (m s0))
		)
		(when (stack s0 ?l2)
			(and 
				(m s0)
				(not (stack s0 ?l2))
			)
		)
		(when (m s1) 
			(not (m s1))
		)
		(when (stack s1 ?l2)
			(and 
				(m s1)
				(not (stack s1 ?l2))
			)
		)
		(when (m s2) 
			(not (m s2))
		)
		(when (stack s2 ?l2)
			(and 
				(m s2)
				(not (stack s2 ?l2))
			)
		)
		(when (m s3) 
			(not (m s3))
		)
		(when (stack s3 ?l2)
			(and 
				(m s3)
				(not (stack s3 ?l2))
			)
		)
		(when (m s4) 
			(not (m s4))
		)
		(when (stack s4 ?l2)
			(and 
				(m s4)
				(not (stack s4 ?l2))
			)
		)
	)
)

;; Resolve the axioms manually.
(:action raminificate
	:parameters ()
	:precondition (resolve-axioms)
	:effect (and 
		(not (resolve-axioms))
		;; For every state ?s
		(when (or (at robot wp0_for_key0 s0) (not (m s0)))
			(Rat robot wp0_for_key0 s0)
		)
		(when (and (not (at robot wp0_for_key0 s0)) (m s0))
			(not (Rat robot wp0_for_key0 s0))
		)
		(when (or (at robot wp1_for_key0 s0) (not (m s0)))
			(Rat robot wp1_for_key0 s0)
		)
		(when (and (not (at robot wp1_for_key0 s0)) (m s0))
			(not (Rat robot wp1_for_key0 s0))
		)
		(when (or (at robot wp2_for_key0 s0) (not (m s0)))
			(Rat robot wp2_for_key0 s0)
		)
		(when (and (not (at robot wp2_for_key0 s0)) (m s0))
			(not (Rat robot wp2_for_key0 s0))
		)
		(when (or (at robot wp3_for_key0 s0) (not (m s0)))
			(Rat robot wp3_for_key0 s0)
		)
		(when (and (not (at robot wp3_for_key0 s0)) (m s0))
			(not (Rat robot wp3_for_key0 s0))
		)
		(when (or (at robot wp4_for_key0 s0) (not (m s0)))
			(Rat robot wp4_for_key0 s0)
		)
		(when (and (not (at robot wp4_for_key0 s0)) (m s0))
			(not (Rat robot wp4_for_key0 s0))
		)
		(when (or (at robot start_wp s0) (not (m s0)))
			(Rat robot start_wp s0)
		)
		(when (and (not (at robot start_wp s0)) (m s0))
			(not (Rat robot start_wp s0))
		)
		(when (or (has key0 s0) (not (m s0)))
			(Rhas key0 s0)
		)
		(when (and (not (has key0 s0)) (m s0))
			(not (Rhas key0 s0))
		)
		(when (or (at key0 wp0_for_key0 s0) (not (m s0)))
			(Rat key0 wp0_for_key0 s0)
		)
		(when (and (not (at key0 wp0_for_key0 s0)) (m s0))
			(not (Rat key0 wp0_for_key0 s0))
		)
		(when (or (at key0 wp1_for_key0 s0) (not (m s0)))
			(Rat key0 wp1_for_key0 s0)
		)
		(when (and (not (at key0 wp1_for_key0 s0)) (m s0))
			(not (Rat key0 wp1_for_key0 s0))
		)
		(when (or (at key0 wp2_for_key0 s0) (not (m s0)))
			(Rat key0 wp2_for_key0 s0)
		)
		(when (and (not (at key0 wp2_for_key0 s0)) (m s0))
			(not (Rat key0 wp2_for_key0 s0))
		)
		(when (or (at key0 wp3_for_key0 s0) (not (m s0)))
			(Rat key0 wp3_for_key0 s0)
		)
		(when (and (not (at key0 wp3_for_key0 s0)) (m s0))
			(not (Rat key0 wp3_for_key0 s0))
		)
		(when (or (at key0 wp4_for_key0 s0) (not (m s0)))
			(Rat key0 wp4_for_key0 s0)
		)
		(when (and (not (at key0 wp4_for_key0 s0)) (m s0))
			(not (Rat key0 wp4_for_key0 s0))
		)
		(when (or (at key0 start_wp s0) (not (m s0)))
			(Rat key0 start_wp s0)
		)
		(when (and (not (at key0 start_wp s0)) (m s0))
			(not (Rat key0 start_wp s0))
		)
		(when (or (at robot wp0_for_key0 s1) (not (m s1)))
			(Rat robot wp0_for_key0 s1)
		)
		(when (and (not (at robot wp0_for_key0 s1)) (m s1))
			(not (Rat robot wp0_for_key0 s1))
		)
		(when (or (at robot wp1_for_key0 s1) (not (m s1)))
			(Rat robot wp1_for_key0 s1)
		)
		(when (and (not (at robot wp1_for_key0 s1)) (m s1))
			(not (Rat robot wp1_for_key0 s1))
		)
		(when (or (at robot wp2_for_key0 s1) (not (m s1)))
			(Rat robot wp2_for_key0 s1)
		)
		(when (and (not (at robot wp2_for_key0 s1)) (m s1))
			(not (Rat robot wp2_for_key0 s1))
		)
		(when (or (at robot wp3_for_key0 s1) (not (m s1)))
			(Rat robot wp3_for_key0 s1)
		)
		(when (and (not (at robot wp3_for_key0 s1)) (m s1))
			(not (Rat robot wp3_for_key0 s1))
		)
		(when (or (at robot wp4_for_key0 s1) (not (m s1)))
			(Rat robot wp4_for_key0 s1)
		)
		(when (and (not (at robot wp4_for_key0 s1)) (m s1))
			(not (Rat robot wp4_for_key0 s1))
		)
		(when (or (at robot start_wp s1) (not (m s1)))
			(Rat robot start_wp s1)
		)
		(when (and (not (at robot start_wp s1)) (m s1))
			(not (Rat robot start_wp s1))
		)
		(when (or (has key0 s1) (not (m s1)))
			(Rhas key0 s1)
		)
		(when (and (not (has key0 s1)) (m s1))
			(not (Rhas key0 s1))
		)
		(when (or (at key0 wp0_for_key0 s1) (not (m s1)))
			(Rat key0 wp0_for_key0 s1)
		)
		(when (and (not (at key0 wp0_for_key0 s1)) (m s1))
			(not (Rat key0 wp0_for_key0 s1))
		)
		(when (or (at key0 wp1_for_key0 s1) (not (m s1)))
			(Rat key0 wp1_for_key0 s1)
		)
		(when (and (not (at key0 wp1_for_key0 s1)) (m s1))
			(not (Rat key0 wp1_for_key0 s1))
		)
		(when (or (at key0 wp2_for_key0 s1) (not (m s1)))
			(Rat key0 wp2_for_key0 s1)
		)
		(when (and (not (at key0 wp2_for_key0 s1)) (m s1))
			(not (Rat key0 wp2_for_key0 s1))
		)
		(when (or (at key0 wp3_for_key0 s1) (not (m s1)))
			(Rat key0 wp3_for_key0 s1)
		)
		(when (and (not (at key0 wp3_for_key0 s1)) (m s1))
			(not (Rat key0 wp3_for_key0 s1))
		)
		(when (or (at key0 wp4_for_key0 s1) (not (m s1)))
			(Rat key0 wp4_for_key0 s1)
		)
		(when (and (not (at key0 wp4_for_key0 s1)) (m s1))
			(not (Rat key0 wp4_for_key0 s1))
		)
		(when (or (at key0 start_wp s1) (not (m s1)))
			(Rat key0 start_wp s1)
		)
		(when (and (not (at key0 start_wp s1)) (m s1))
			(not (Rat key0 start_wp s1))
		)
		(when (or (at robot wp0_for_key0 s2) (not (m s2)))
			(Rat robot wp0_for_key0 s2)
		)
		(when (and (not (at robot wp0_for_key0 s2)) (m s2))
			(not (Rat robot wp0_for_key0 s2))
		)
		(when (or (at robot wp1_for_key0 s2) (not (m s2)))
			(Rat robot wp1_for_key0 s2)
		)
		(when (and (not (at robot wp1_for_key0 s2)) (m s2))
			(not (Rat robot wp1_for_key0 s2))
		)
		(when (or (at robot wp2_for_key0 s2) (not (m s2)))
			(Rat robot wp2_for_key0 s2)
		)
		(when (and (not (at robot wp2_for_key0 s2)) (m s2))
			(not (Rat robot wp2_for_key0 s2))
		)
		(when (or (at robot wp3_for_key0 s2) (not (m s2)))
			(Rat robot wp3_for_key0 s2)
		)
		(when (and (not (at robot wp3_for_key0 s2)) (m s2))
			(not (Rat robot wp3_for_key0 s2))
		)
		(when (or (at robot wp4_for_key0 s2) (not (m s2)))
			(Rat robot wp4_for_key0 s2)
		)
		(when (and (not (at robot wp4_for_key0 s2)) (m s2))
			(not (Rat robot wp4_for_key0 s2))
		)
		(when (or (at robot start_wp s2) (not (m s2)))
			(Rat robot start_wp s2)
		)
		(when (and (not (at robot start_wp s2)) (m s2))
			(not (Rat robot start_wp s2))
		)
		(when (or (has key0 s2) (not (m s2)))
			(Rhas key0 s2)
		)
		(when (and (not (has key0 s2)) (m s2))
			(not (Rhas key0 s2))
		)
		(when (or (at key0 wp0_for_key0 s2) (not (m s2)))
			(Rat key0 wp0_for_key0 s2)
		)
		(when (and (not (at key0 wp0_for_key0 s2)) (m s2))
			(not (Rat key0 wp0_for_key0 s2))
		)
		(when (or (at key0 wp1_for_key0 s2) (not (m s2)))
			(Rat key0 wp1_for_key0 s2)
		)
		(when (and (not (at key0 wp1_for_key0 s2)) (m s2))
			(not (Rat key0 wp1_for_key0 s2))
		)
		(when (or (at key0 wp2_for_key0 s2) (not (m s2)))
			(Rat key0 wp2_for_key0 s2)
		)
		(when (and (not (at key0 wp2_for_key0 s2)) (m s2))
			(not (Rat key0 wp2_for_key0 s2))
		)
		(when (or (at key0 wp3_for_key0 s2) (not (m s2)))
			(Rat key0 wp3_for_key0 s2)
		)
		(when (and (not (at key0 wp3_for_key0 s2)) (m s2))
			(not (Rat key0 wp3_for_key0 s2))
		)
		(when (or (at key0 wp4_for_key0 s2) (not (m s2)))
			(Rat key0 wp4_for_key0 s2)
		)
		(when (and (not (at key0 wp4_for_key0 s2)) (m s2))
			(not (Rat key0 wp4_for_key0 s2))
		)
		(when (or (at key0 start_wp s2) (not (m s2)))
			(Rat key0 start_wp s2)
		)
		(when (and (not (at key0 start_wp s2)) (m s2))
			(not (Rat key0 start_wp s2))
		)
		(when (or (at robot wp0_for_key0 s3) (not (m s3)))
			(Rat robot wp0_for_key0 s3)
		)
		(when (and (not (at robot wp0_for_key0 s3)) (m s3))
			(not (Rat robot wp0_for_key0 s3))
		)
		(when (or (at robot wp1_for_key0 s3) (not (m s3)))
			(Rat robot wp1_for_key0 s3)
		)
		(when (and (not (at robot wp1_for_key0 s3)) (m s3))
			(not (Rat robot wp1_for_key0 s3))
		)
		(when (or (at robot wp2_for_key0 s3) (not (m s3)))
			(Rat robot wp2_for_key0 s3)
		)
		(when (and (not (at robot wp2_for_key0 s3)) (m s3))
			(not (Rat robot wp2_for_key0 s3))
		)
		(when (or (at robot wp3_for_key0 s3) (not (m s3)))
			(Rat robot wp3_for_key0 s3)
		)
		(when (and (not (at robot wp3_for_key0 s3)) (m s3))
			(not (Rat robot wp3_for_key0 s3))
		)
		(when (or (at robot wp4_for_key0 s3) (not (m s3)))
			(Rat robot wp4_for_key0 s3)
		)
		(when (and (not (at robot wp4_for_key0 s3)) (m s3))
			(not (Rat robot wp4_for_key0 s3))
		)
		(when (or (at robot start_wp s3) (not (m s3)))
			(Rat robot start_wp s3)
		)
		(when (and (not (at robot start_wp s3)) (m s3))
			(not (Rat robot start_wp s3))
		)
		(when (or (has key0 s3) (not (m s3)))
			(Rhas key0 s3)
		)
		(when (and (not (has key0 s3)) (m s3))
			(not (Rhas key0 s3))
		)
		(when (or (at key0 wp0_for_key0 s3) (not (m s3)))
			(Rat key0 wp0_for_key0 s3)
		)
		(when (and (not (at key0 wp0_for_key0 s3)) (m s3))
			(not (Rat key0 wp0_for_key0 s3))
		)
		(when (or (at key0 wp1_for_key0 s3) (not (m s3)))
			(Rat key0 wp1_for_key0 s3)
		)
		(when (and (not (at key0 wp1_for_key0 s3)) (m s3))
			(not (Rat key0 wp1_for_key0 s3))
		)
		(when (or (at key0 wp2_for_key0 s3) (not (m s3)))
			(Rat key0 wp2_for_key0 s3)
		)
		(when (and (not (at key0 wp2_for_key0 s3)) (m s3))
			(not (Rat key0 wp2_for_key0 s3))
		)
		(when (or (at key0 wp3_for_key0 s3) (not (m s3)))
			(Rat key0 wp3_for_key0 s3)
		)
		(when (and (not (at key0 wp3_for_key0 s3)) (m s3))
			(not (Rat key0 wp3_for_key0 s3))
		)
		(when (or (at key0 wp4_for_key0 s3) (not (m s3)))
			(Rat key0 wp4_for_key0 s3)
		)
		(when (and (not (at key0 wp4_for_key0 s3)) (m s3))
			(not (Rat key0 wp4_for_key0 s3))
		)
		(when (or (at key0 start_wp s3) (not (m s3)))
			(Rat key0 start_wp s3)
		)
		(when (and (not (at key0 start_wp s3)) (m s3))
			(not (Rat key0 start_wp s3))
		)
		(when (or (at robot wp0_for_key0 s4) (not (m s4)))
			(Rat robot wp0_for_key0 s4)
		)
		(when (and (not (at robot wp0_for_key0 s4)) (m s4))
			(not (Rat robot wp0_for_key0 s4))
		)
		(when (or (at robot wp1_for_key0 s4) (not (m s4)))
			(Rat robot wp1_for_key0 s4)
		)
		(when (and (not (at robot wp1_for_key0 s4)) (m s4))
			(not (Rat robot wp1_for_key0 s4))
		)
		(when (or (at robot wp2_for_key0 s4) (not (m s4)))
			(Rat robot wp2_for_key0 s4)
		)
		(when (and (not (at robot wp2_for_key0 s4)) (m s4))
			(not (Rat robot wp2_for_key0 s4))
		)
		(when (or (at robot wp3_for_key0 s4) (not (m s4)))
			(Rat robot wp3_for_key0 s4)
		)
		(when (and (not (at robot wp3_for_key0 s4)) (m s4))
			(not (Rat robot wp3_for_key0 s4))
		)
		(when (or (at robot wp4_for_key0 s4) (not (m s4)))
			(Rat robot wp4_for_key0 s4)
		)
		(when (and (not (at robot wp4_for_key0 s4)) (m s4))
			(not (Rat robot wp4_for_key0 s4))
		)
		(when (or (at robot start_wp s4) (not (m s4)))
			(Rat robot start_wp s4)
		)
		(when (and (not (at robot start_wp s4)) (m s4))
			(not (Rat robot start_wp s4))
		)
		(when (or (has key0 s4) (not (m s4)))
			(Rhas key0 s4)
		)
		(when (and (not (has key0 s4)) (m s4))
			(not (Rhas key0 s4))
		)
		(when (or (at key0 wp0_for_key0 s4) (not (m s4)))
			(Rat key0 wp0_for_key0 s4)
		)
		(when (and (not (at key0 wp0_for_key0 s4)) (m s4))
			(not (Rat key0 wp0_for_key0 s4))
		)
		(when (or (at key0 wp1_for_key0 s4) (not (m s4)))
			(Rat key0 wp1_for_key0 s4)
		)
		(when (and (not (at key0 wp1_for_key0 s4)) (m s4))
			(not (Rat key0 wp1_for_key0 s4))
		)
		(when (or (at key0 wp2_for_key0 s4) (not (m s4)))
			(Rat key0 wp2_for_key0 s4)
		)
		(when (and (not (at key0 wp2_for_key0 s4)) (m s4))
			(not (Rat key0 wp2_for_key0 s4))
		)
		(when (or (at key0 wp3_for_key0 s4) (not (m s4)))
			(Rat key0 wp3_for_key0 s4)
		)
		(when (and (not (at key0 wp3_for_key0 s4)) (m s4))
			(not (Rat key0 wp3_for_key0 s4))
		)
		(when (or (at key0 wp4_for_key0 s4) (not (m s4)))
			(Rat key0 wp4_for_key0 s4)
		)
		(when (and (not (at key0 wp4_for_key0 s4)) (m s4))
			(not (Rat key0 wp4_for_key0 s4))
		)
		(when (or (at key0 start_wp s4) (not (m s4)))
			(Rat key0 start_wp s4)
		)
		(when (and (not (at key0 start_wp s4)) (m s4))
			(not (Rat key0 start_wp s4))
		)
	)
)
)
