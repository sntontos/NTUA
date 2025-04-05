let read_lines filename =
  let ic = open_in filename in
  try
    let line = input_line ic in
    close_in ic;
    String.split_on_char ' ' line |> List.map String.trim |> List.filter (fun s -> s <> "") |> List.map int_of_string
  with End_of_file ->
    close_in ic;
    []

let rec max_left heights =
    let rec aux max_so_far = function
      | [] -> []
      | h :: t -> let new_max = max max_so_far h in new_max :: aux new_max t
    in aux 0 heights

(* Υπολογίζει τον σωρευτικό μέγιστο από τα δεξιά προς τα αριστερά *)
let max_right heights =
  List.rev (max_left (List.rev heights))

(* Υπολογίζει τον συνολικό όγκο του νερού που μπορεί να συγκρατηθεί *)
let calculate_trapped_water heights =
  let left_max = max_left heights in
  let right_max = max_right heights in
  let rec aux acc = function
    | ([], _, _) -> acc
    | (_, [], _) -> acc
    | (_, _, []) -> acc
    | (h :: hs, lm :: lms, rm :: rms) ->
      let trapped = max 0 (min lm rm - h) in
      aux (acc + trapped) (hs, lms, rms)
  in aux 0 (heights, left_max, right_max)


let () =
  let filename = Sys.argv.(1) in
  let lines = read_lines filename in
  let heights = lines in
  let trapped_water = calculate_trapped_water heights in
  Printf.printf "Total trapped water: %d\n" trapped_water




