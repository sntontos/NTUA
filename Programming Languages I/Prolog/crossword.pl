/* solve_crossword(+FilePath): main entry, solves puzzle and prints horizontal words */
crossword(FilePath) :-
    parse_input_file(FilePath, Rows, Cols, Blacks, Words),
    build_empty_grid(Rows, Cols, Grid),
    apply_blocked_cells(Grid, Blacks, GridWithBlacks),
    detect_word_slots(GridWithBlacks, Rows, Cols, RawSlots),
    sort_slots_by_candidates(Words, RawSlots, Slots),
    (   solve_by_backtracking(GridWithBlacks, Slots, Words, SolvedGrid) ->
        print_solution_words(SolvedGrid, Rows, Cols)
    ;   write('IMPOSSIBLE'), nl
    ).

/* parse_input_file(+FilePath, -Rows, -Cols, -Blacks, -Words): read file header and lists */
parse_input_file(FilePath, Rows, Cols, Blacks, Words) :-
    open(FilePath, read, Stream),
    read_line_to_codes(Stream, FirstLine),
    codes_to_atoms(FirstLine, FirstTokens),
    FirstTokens = [RowsAtom, ColsAtom, NumBlacksAtom, NumWordsAtom],
    atom_number(RowsAtom, Rows),
    atom_number(ColsAtom, Cols),
    atom_number(NumBlacksAtom, NumBlacks),
    atom_number(NumWordsAtom, NumWords),
    read_blacks(Stream, NumBlacks, Blacks),
    read_word_list(Stream, NumWords, Words),
    close(Stream).

codes_to_atoms([], []).
codes_to_atoms(Codes, Atoms) :-
    skip_spaces(Codes, NoSpaces),
    split_on_space(NoSpaces, [], TokenCodes),
    maplist(atom_codes, Atoms, TokenCodes).

skip_spaces([32|Rest], NoSpaces) :- !, skip_spaces(Rest, NoSpaces).
skip_spaces(Codes, Codes).

split_on_space([], Current, [Current]) :- Current \= [].
split_on_space([], [], []).
split_on_space([32|Rest], Current, [Current|Tokens]) :-
    Current \= [], !,
    skip_spaces(Rest, NoSpaces),
    split_on_space(NoSpaces, [], Tokens).
split_on_space([C|Rest], Current, Tokens) :-
    append(Current, [C], NewCurrent),
    split_on_space(Rest, NewCurrent, Tokens).

/* read_blacks(+Stream, +Count, -BlackCoords): read black cell coords */
read_blacks(_, 0, []) :- !.
read_blacks(Stream, N, [(R, C)|Rest]) :-
    N > 0,
    read_line_to_codes(Stream, Line),
    codes_to_atoms(Line, [R1Atom, C1Atom]),
    atom_number(R1Atom, R1), atom_number(C1Atom, C1),
    R is R1 - 1, C is C1 - 1,
    N1 is N - 1,
    read_blacks(Stream, N1, Rest).

/* read_word_list(+Stream, +Count, -Words): read puzzle words */
read_word_list(_, 0, []) :- !.
read_word_list(Stream, N, [Word|Rest]) :-
    N > 0,
    read_line_to_codes(Stream, Line),
    atom_codes(Word, Line),
    N1 is N - 1,
    read_word_list(Stream, N1, Rest).

/* build_empty_grid(+Rows, +Cols, -Grid): create grid of empty cells */
build_empty_grid(Rows, Cols, Grid) :-
    length(Grid, Rows),
    maplist(create_empty_row(Cols), Grid).

create_empty_row(Cols, Row) :-
    length(Row, Cols),
    maplist(=(empty), Row).

/* apply_blocked_cells(+Grid, +BlackCoords, -NewGrid): mark black cells */
apply_blocked_cells(Grid, [], Grid).
apply_blocked_cells(Grid, [(R, C)|Rest], FinalGrid) :-
    set_grid_cell(Grid, R, C, black, GridWithBlack),
    apply_blocked_cells(GridWithBlack, Rest, FinalGrid).

set_grid_cell(Grid, R, C, Value, NewGrid) :-
    nth0(R, Grid, Row),
    set_nth0(C, Row, Value, NewRow),
    set_nth0(R, Grid, NewRow, NewGrid).

set_nth0(0, [_|T], Value, [Value|T]) :- !.
set_nth0(N, [H|T], Value, [H|NewT]) :-
    N > 0,
    N1 is N - 1,
    set_nth0(N1, T, Value, NewT).

/* detect_word_slots(+Grid, +Rows, +Cols, -Slots): find slots length>=2 */
detect_word_slots(Grid, Rows, Cols, AllSlots) :-
    R1 is Rows - 1,
    C1 is Cols - 1,
    findall(slot(R, C, L, h),
            (between(0, R1, R), horizontal_slot(Grid, R, C, L, Cols)),
            HSlots),
    findall(slot(R, C, L, v),
            (between(0, C1, C), vertical_slot(Grid, R, C, L, Rows)),
            VSlots),
    append(HSlots, VSlots, AllSlots).

horizontal_slot(Grid, R, StartC, Len, Cols) :-
    between(0, Cols, StartC),
    StartC < Cols,
    nth0(R, Grid, Row),
    nth0(StartC, Row, Cell),
    Cell \= black,
    count_h_length(Grid, R, StartC, Cols, 0, Len),
    Len >= 2,
    ( StartC =:= 0 ->
        true
    ; StartC1 is StartC - 1,
      nth0(StartC1, Row, PrevCell),
      PrevCell = black
    ).

vertical_slot(Grid, StartR, C, Len, Rows) :-
    between(0, Rows, StartR),
    StartR < Rows,
    nth0(StartR, Grid, Row),
    nth0(C, Row, Cell),
    Cell \= black,
    count_v_length(Grid, StartR, C, Rows, 0, Len),
    Len >= 2,
    ( StartR =:= 0 ->
        true
    ; StartR1 is StartR - 1,
      nth0(StartR1, Grid, PrevRow),
      nth0(C, PrevRow, PrevCell),
      PrevCell = black
    ).

count_h_length(Grid, R, C, Cols, Acc, Len) :-
    (   C >= Cols ->
        Len = Acc
    ;   nth0(R, Grid, Row),
        nth0(C, Row, Cell),
        (   Cell = black ->
            Len = Acc
        ;   Acc1 is Acc + 1,
            C1 is C + 1,
            count_h_length(Grid, R, C1, Cols, Acc1, Len)
        )
    ).

count_v_length(Grid, R, C, Rows, Acc, Len) :-
    (   R >= Rows ->
        Len = Acc
    ;   nth0(R, Grid, Row),
        nth0(C, Row, Cell),
        (   Cell = black ->
            Len = Acc
        ;   Acc1 is Acc + 1,
            R1 is R + 1,
            count_v_length(Grid, R1, C, Rows, Acc1, Len)
        )
    ).

/* solve_by_backtracking(+Grid, +Slots, +Words, -SolGrid): try word placements */
solve_by_backtracking(Grid, [], [], Grid).
solve_by_backtracking(Grid, [Slot|RestSlots], Words, FinalGrid) :-
    select(Word, Words, RestWords),
    fit_word(Grid, Word, Slot, NewGrid),
    solve_by_backtracking(NewGrid, RestSlots, RestWords, FinalGrid).

fit_word(Grid, Word, slot(R, C, Len, Dir), NewGrid) :-
    atom_codes(Word, Codes),
    length(Codes, Len),
    place_word_at(Grid, Codes, R, C, Dir, 0, NewGrid).

/* place_word_at(+Grid, +Codes, +Row, +Col, +Dir, +Offset, -NewGrid): place word recursively */
place_word_at(Grid, [], _R, _C, _Dir, _Offset, Grid).
place_word_at(Grid, [Code|RestCodes], R, C, Dir, Offset, FinalGrid) :-
    ( Dir = h ->
        R1 = R,
        C1 is C + Offset
    ;
        R1 is R + Offset,
        C1 is C
    ),
    nth0(R1, Grid, Row),
    nth0(C1, Row, Cell),
    atom_codes(CharAtom, [Code]),
    (   Cell = empty ->
        set_grid_cell(Grid, R1, C1, CharAtom, NewGrid)
    ;   Cell = CharAtom ->
        NewGrid = Grid
    ;   fail
    ),
    Offset1 is Offset + 1,
    place_word_at(NewGrid, RestCodes, R, C, Dir, Offset1, FinalGrid).

/* print_solution_words(+Grid, +Rows, +Cols): output horizontal words per row */
print_solution_words(Grid, Rows, _) :-
    print_all_rows(Grid, 0, Rows).

print_all_rows(_, Rows, Rows).
print_all_rows(Grid, R, Rows) :-
    R < Rows,
    nth0(R, Grid, Row),
    extract_words_from_row(Row, Words),
    print_words(Words),
    nl,
    R1 is R + 1,
    print_all_rows(Grid, R1, Rows).

extract_words_from_row(Row, ValidWords) :-
    collect_row_words(Row, [], "", AllWords),
    include(valid_word, AllWords, ValidWords).

collect_row_words([], Acc, Current, Words) :-
    (   Current = "" ->
        reverse(Acc, Words)
    ;   reverse([Current|Acc], Words)
    ).
collect_row_words([Cell|Rest], Acc, Current, Words) :-
    (   Cell = empty ; Cell = black ->
        (   Current = "" ->
            collect_row_words(Rest, Acc, "", Words)
        ;   collect_row_words(Rest, [Current|Acc], "", Words)
        )
    ;   atom_concat(Current, Cell, NewCurrent),
        collect_row_words(Rest, Acc, NewCurrent, Words)
    ).

valid_word(Word) :-
    atom_length(Word, Len),
    Len >= 2.

print_words([]).
print_words([Word]) :-
    write(Word).
print_words([Word|Rest]) :-
    Rest \= [],
    write(Word), write(' '),
    print_words(Rest).


:- use_module(library(lists)).


sort_slots_by_candidates(Words, Slots, SortedSlots) :-
    findall(Count-Slot,
        ( member(Slot, Slots),
          Slot = slot(_,_,Len,_),
          include({Len}/[W]>>(st ring_length(W,L), L=Len), Words, Cand),
          length(Cand, Count)
        ),
        PairList),
    keysort(PairList, SortedPairs),
    pairs_values(SortedPairs, SortedSlots).

% pairs_values(+Pairs, -Values)
pairs_values(Pairs, Values) :-
    findall(V, member(_-V, Pairs), Values).