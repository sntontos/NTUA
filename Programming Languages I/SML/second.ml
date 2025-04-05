let filename = Sys.argv.(1)

let m_list = ref []
let k_list = ref []
let capacity = ref 0

(* Διαβάζει τις γραμμές από το αρχείο και τις αποθηκεύει σε λίστες *)

let rec sum_selective lst1 lst2 m k =
    match lst1, lst2 with
    | _, _ when m = 0 && k = 0 -> 0
    | h1 :: t1, h2 :: t2 ->
        if h1 >= h2 && m > 0 then
          h1 + sum_selective t1 lst2 (m - 1) k
        else if k > 0 then
          h2 + sum_selective lst1 t2 m (k - 1)
        else
          0
    | h1 :: t1, [] when m > 0 -> h1 + sum_selective t1 [] (m - 1) k
    | [], h2 :: t2 when k > 0 -> h2 + sum_selective [] t2 m (k - 1)
    | _ -> 0


let () =
  (* Open the input file *)
  let ic = open_in filename in
  try
    (* Read lines from the file and process them *)
    while true do
      let line = input_line ic in
      match String.split_on_char ' ' line with
      | m :: k :: [] ->
        (* Parse integers and add them to the respective lists *)
        let m = int_of_string m in
        let k = int_of_string k in
        m_list := m :: !m_list;
        k_list := k :: !k_list
      | _ -> failwith "Invalid input format"
    done
  with End_of_file ->
    (* Close the input channel when done reading *)
    close_in ic;

    (* Reverse the lists to maintain the original order *)
    m_list := List.rev !m_list;
    k_list := List.rev !k_list;

    (* Extract the first elements as m and k *)
    let m = List.hd !m_list and k = List.hd !k_list in

    (* Sort the remaining elements in descending order *)
    m_list := List.sort (fun a b -> compare b a) (List.tl !m_list);
    k_list := List.sort (fun a b -> compare b a) (List.tl !k_list);

    (* Assign the sorted lists *)
    let lst1, lst2 = !m_list, !k_list in

    (* Print the parsed and sorted data *)
    (* Printf.printf "m: %d, k: %d\nList1: [%s]\nList2: [%s]\n"
      m k
      (String.concat "; " (List.map string_of_int lst1))
      (String.concat "; " (List.map string_of_int lst2)); *)

    (* Compute and print the selective sum *)
    Printf.printf "Sum: %d\n" (sum_selective lst1 lst2 m k)