type cell = char option
type grid = cell array array

(* Deep copy the grid *)
let copy_grid g = Array.map Array.copy g

(* Initialize an a×b grid, marking black cells as '#' *)
let make_grid a b black_coords =
  Array.init a (fun i ->
    Array.init b (fun j ->
      if List.mem (i, j) black_coords then Some '#' else None
    )
  )

(* Slot representation *)
type dir = H | V
type slot = { row:int; col:int; len:int; dir:dir }

(* Find all slots of length >= 2 *)
let find_slots grid =
  let a = Array.length grid and b = Array.length grid.(0) in
  let slots = ref [] in
  (* Horizontal slots *)
  for i = 0 to a - 1 do
    let j = ref 0 in
    while !j < b do
      if grid.(i).(!j) <> Some '#' then begin
        let start = !j in
        while !j < b && grid.(i).(!j) <> Some '#' do incr j done;
        let l = !j - start in
        if l >= 2 then slots := {row=i; col=start; len=l; dir=H} :: !slots
      end else incr j
    done
  done;
  (* Vertical slots *)
  for j = 0 to b - 1 do
    let i = ref 0 in
    while !i < a do
      if grid.(!i).(j) <> Some '#' then begin
        let start = !i in
        while !i < a && grid.(!i).(j) <> Some '#' do incr i done;
        let l = !i - start in
        if l >= 2 then slots := {row=start; col=j; len=l; dir=V} :: !slots
      end else incr i
    done
  done;
  List.rev !slots

(* Check and place a word in a slot *)
let can_place_word grid word slot =
  if String.length word <> slot.len then false else
  let rec check k =
    if k = slot.len then true else
    let i = if slot.dir = H then slot.row else slot.row + k in
    let j = if slot.dir = H then slot.col + k else slot.col in
    match grid.(i).(j) with
    | Some '#' -> false
    | Some c   -> c = word.[k] && check (k+1)
    | None     -> check (k+1)
  in check 0

let place_word grid word slot =
  for k = 0 to slot.len - 1 do
    let i = if slot.dir = H then slot.row else slot.row + k in
    let j = if slot.dir = H then slot.col + k else slot.col in
    grid.(i).(j) <- Some word.[k]
  done

(* Backtracking solver *)
let rec solve grid slots words =
  match words with
  | [] -> Some grid
  | w::ws ->
    List.fold_left (fun acc slot ->
      match acc with
      | Some _ -> acc
      | None ->
        if can_place_word grid w slot then
          let g2 = copy_grid grid in
          place_word g2 w slot;
          solve g2 (List.filter (fun s -> s <> slot) slots) ws
        else None
    ) None slots

let () =
  let fn = Sys.argv.(1) in
  let ic = open_in fn in

  (* Parse first line: a b c d *)
  let first_line =
    try input_line ic
    with End_of_file ->
      prerr_endline "Error: input file is empty"; exit 1
  in
  let nums = first_line
    |> String.split_on_char ' '
    |> List.filter (fun s -> String.trim s <> "")
    |> List.map int_of_string
  in
  let a,b,c,d =
    match nums with
    | [a; b; c; d] -> a, b, c, d
    | _ ->
        prerr_endline "Error: first line must contain exactly four integers"; exit 1
  in

  if a > 21 || b > 21 then begin
    prerr_endline (Printf.sprintf "Error: dimensions %dx%d exceed maximum 21x21" a b);
    exit 1
  end;

  (* Read black-cell coords *)
  let rec read_n n acc =
    if n = 0 then List.rev acc else
    let line =
      try input_line ic
      with End_of_file ->
        prerr_endline "Error: unexpected end of file while reading list"; exit 1
    in
    read_n (n-1) (line :: acc)
  in
  let blacks =
    read_n c []
    |> List.map (fun l ->
         match String.split_on_char ' ' l |> List.map int_of_string with
         | [x; y] -> (x-1, y-1)
         | _ -> prerr_endline "Error: bad black-cell format"; exit 1)
  in
  (* Validate black-cell coordinates within bounds *)
  List.iter (fun (x,y) ->
    if x < 0 || x >= a || y < 0 || y >= b then begin
      prerr_endline (Printf.sprintf "Error: black cell (%d,%d) out of bounds" (x+1) (y+1));
      exit 1
    end
  ) blacks;

  (* Read words *)
  let words = read_n d [] in
  close_in ic;

  let grid  = make_grid a b blacks in
  let slots = find_slots grid in

  match solve grid slots words with
  | Some sol ->
      (* For each row, collect horizontal runs and print them joined with spaces *)
      let rows = Array.length sol and cols = Array.length sol.(0) in
      for i = 0 to rows - 1 do
        let rec split j curr acc =
          if j = cols then
            let acc = if curr = [] then acc
                      else (String.concat "" (List.rev curr)) :: acc
            in List.rev acc
          else
            match sol.(i).(j) with
            | Some c when c <> '#' ->
                split (j+1) (Char.escaped c :: curr) acc
            | _ ->
                let acc, curr =
                  if curr = [] then acc, []
                  else (String.concat "" (List.rev curr)) :: acc, []
                in
                split (j+1) curr acc
        in
        let segs = split 0 [] [] in
        let good = List.filter (fun s -> String.length s >= 2) segs in
        if good = [] then print_newline ()
        else print_endline (String.concat " " good)
      done
  | None ->
      prerr_endline "No solution"; exit 1