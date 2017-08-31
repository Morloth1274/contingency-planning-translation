(define (domain doors)
(:requirements :typing :equality)

(:types
	cell
)

(:predicates
	(at ?c - cell)
	(opened ?c - cell)
	(adj ?c ?c2 - cell)
)

;; Action to move to a different cell.
(:action navigate
	:parameters (?from ?to - cell)
	:precondition (and
		(adj ?from ?to)
		(at ?from)
		(opened ?to)
	)
	:effect (and
		(not (at ?from))
		(at ?to)
	)
)

;; Sense if a cell is open.
(:action sense
	:parameters (?c ?c2 - cell)
	:precondition (and
		(adj ?c2 ?c)

		(at ?c2)
	)
	:observe (opened ?c)
)

)
