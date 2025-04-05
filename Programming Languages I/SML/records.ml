type student = {
    name : string;
    year : int;
}

let rgb = {
    name = "Ntinaras";
    year = 2003;
}

let name_with_year s =
    match s with
    | {name; year} -> name ^ " '" ^ string_of_int (year mod 100)