(*Function to read the n numbers from a file .txt*)
let read_peaks filename = 
  let file = open_in filename in
  try
    let line = input_line file in
    close_in file;
    List.map int_of_string (String.split_on_char ' ' line)
  with e ->
    close_in_noerr file;
    raise e;;


let numbers = read_peaks "readme.txt";;

