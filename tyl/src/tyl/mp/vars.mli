open Ast

val freeVarse : expr -> Id.Set.t
val freeVarsc : prop -> Id.Set.t
val freeVarsp : prog -> Id.Set.t

val isClosede : expr -> bool
val isClosedc : prop -> bool
val isClosedp : prog -> bool

val subee : expr -> Id.t -> expr -> expr
val subec : expr -> Id.t -> prop -> prop
val subec' : expr list -> Id.t list -> prop -> prop

val alphaConverte : Id.t -> expr -> Id.Set.t -> Id.t * expr
val alphaConvertc : Id.t -> prop -> Id.Set.t -> Id.t * prop
