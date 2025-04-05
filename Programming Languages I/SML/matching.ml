let x =
    match not true with
    | true -> "nope"
    | false -> "yep"

let y =
    match 42 with
    | a -> a

let matchstring =
    match "foo" with
    | "bar" -> 0
    | _ -> 1

let a x=
    match x with
    | [] -> "empty"
    | _ -> "not empty"

let a =
    match ["taylor" ; "swift"] with
    | [] -> ["empty"] (*without [] to get h*)
    | h :: t -> t
(*h means head of the list while t means tail*)

let empty lst =
    match lst with
    | [] -> true
    | h::t -> false

let rec sum lst =
    match lst with
    | [] -> 0
    | h :: t -> h + sum t

let rec length lst =
    match lst with
    | [] -> 0
    | h :: t -> 1 + length t

let rec append lst1 lst2 =
    match lst1 with
    | [] -> lst2
    | h :: t -> h :: (append t lst2)