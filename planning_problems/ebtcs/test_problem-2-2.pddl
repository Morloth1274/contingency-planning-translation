(define (problem Keys-0)
(:domain find_key)
(:objects
	l0 - LEVEL
	l1 - LEVEL
)

(:init
	(resolve-axioms)
	(lev l0)
	(next l0 l1)
	(current_kb basis_kb)
	(part-of basic basis_kb)
	(m basic)
	(part-of basic basis_kb)
	(parent basis_kb kb_bomb_0)
	(parent basis_kb kb_bomb_1)
	(part-of kb_bomb_0_package_0 kb_bomb_0)
	(in package_0 bomb_0 kb_bomb_0_package_0)
	(part-of kb_bomb_0_package_0_package_1 kb_bomb_0)
	(in package_1 bomb_0 kb_bomb_0_package_0_package_1)
	(part-of kb_bomb_1_package_0 kb_bomb_1)
	(in package_0 bomb_1 kb_bomb_1_package_0)
	(part-of kb_bomb_1_package_0_package_1 kb_bomb_1)
	(in package_1 bomb_1 kb_bomb_1_package_0_package_1)
)
(:goal (and
	(defused bomb_0 basic)
	(defused bomb_1 basic)
)
)
)
