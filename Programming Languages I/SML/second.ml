let filename = Sys.argv.(1)
let m_list = ref []
let k_list = ref []

let rec nth_element lst n =
  match lst, n with
  | [], _ -> failwith "Index out of bounds"
  | x::_, 0 -> x
  | _::xs, n when n > 0 -> nth_element xs (n - 1)
  | _ -> failwith "Negative index" 

let rec diff lst1 lst2 =
  match lst1, lst2 with
  | [], [] -> []
  | x::xs, y::ys -> (x - y) :: diff xs ys
  | _ -> failwith "Lists must have the same length"

let rec zip_with_index lst idx =
  match lst with
  | [] -> []
  | x::xs -> (x, idx) :: zip_with_index xs (idx + 1)

let rec sum (tuple_lst, m, k) lst1 lst2 =
  match tuple_lst, m, k with
  | _, 0, 0 -> 0
  | [], _, _ -> 0
  | (x, idx)::xs, 0, k -> nth_element lst2 idx + sum (xs, 0, k - 1) lst1 lst2  
  | (x, idx)::xs, m, k -> nth_element lst1 idx + sum (xs, m - 1 , k) lst1 lst2
(* Διαβάζει τις γραμμές από το αρχείο και τις αποθηκεύει σε λίστες *)

let () =
  (* Open the input file *)
  let ic =
    try open_in filename
    with Sys_error msg ->
      Printf.eprintf "Error opening file: %s\n" msg;
      exit 1
  in
  try
    (* Read the lines and split into m_list and k_list *)
    while true do
      let line = input_line ic in
      match String.split_on_char ' ' line with
      | [m_val; k_val] ->
          m_list := int_of_string m_val :: !m_list;
          k_list := int_of_string k_val :: !k_list
      | _ -> failwith "Invalid input format in data lines"
    done
  with End_of_file ->
    (* Close the input channel when done reading *)
    close_in ic;

    (* Reverse the lists to maintain the original order *)
    m_list := List.rev !m_list;
    k_list := List.rev !k_list;
   
    (* Save the head values of m_list and k_list *)
    let m = List.hd !m_list in
    let k = List.hd !k_list in

    (* Remove the heads from m_list and k_list *)
    m_list := List.tl !m_list;
    k_list := List.tl !k_list;

    (* Extract the first elements as m and k *)
    let diff = diff (!m_list) (!k_list) in
    let sorted_zipped = List.sort (fun (a, _) (b, _) -> compare a b) (zip_with_index diff 0) in
    let reversed_sorted_zipped = List.rev sorted_zipped in

    let result = sum (reversed_sorted_zipped, m, k) !m_list !k_list in
    Printf.printf "Result: %d\n" result;