:- use_module(library(apply)).
:- use_module(library(lists)).
:- use_module(library(pairs)).
:- use_module(library(random)).

hire(Filename, MaxProfit) :-
    read_contracts(Filename, M, K, Profits),
    Profits \= [],
    compute_max_profit(M, K, Profits, MaxProfit).

read_contracts(File, M, K, Profits) :-
    catch(open(File, read, Stream), E,
        ( print_message(error, E), fail )),
    read_string(Stream, _, Str),
    close(Stream),
    split_string(Str, " \t\n", " \t\n", Tokens),
    maplist(number_string, Numbers, Tokens),
    Numbers = [M, K | Rest],
    pairs_of_list(Rest, Profits).

pairs_of_list([], []).
pairs_of_list([A,B|T], [(A,B)|Rest]) :-
    pairs_of_list(T, Rest).

% random_quickselect(+Pairs, +N, -Pivot)
% Choose the N-th largest difference in average linear time using a random pivot.
random_quickselect([D-_|_], 1, D) :- !.
random_quickselect(Pairs, N, Pivot) :-
    random_member(Pivot-_, Pairs),
    partition3(Pairs, Pivot, Greater, Equal, Less),
    length(Greater, GL), length(Equal, EL),
    (  N =< GL
    -> random_quickselect(Greater, N, Pivot)
    ;  N =< GL + EL
    -> true
    ;  N2 is N - GL - EL,
       random_quickselect(Less, N2, Pivot)
    ).

% partition3(+Pairs, +Pivot, -Greater, -Equal, -Less)
% Partition Diff-(A,B) pairs relative to Pivot.
partition3([], _, [], [], []).
partition3([D-AB|T], P, Gr, Eq, Le) :-
    ( D > P -> Gr = [D-AB|Gr1], partition3(T, P, Gr1, Eq, Le)
    ; D =:= P -> Eq = [D-AB|Eq1], partition3(T, P, Gr, Eq1, Le)
    ; Le = [D-AB|Le1], partition3(T, P, Gr, Eq, Le1)
    ).

% sort_diff_desc(+Keyed, -ValuesDesc)
% Fully sort Diff-(A,B) descending by diff and return only (A,B) values.
sort_diff_desc(Keyed, DescValues) :-
    keysort(Keyed, Asc),
    reverse(Asc, DescPairs),
    pairs_values(DescPairs, DescValues).

% compute_max_profit(+M, +K, +Profits, -Total)
% If M+K covers all workers, use full sort; otherwise use linear selection.
compute_max_profit(M, K, Profits, Total) :-
    map_list_to_pairs(diff_val, Profits, Keyed),
    length(Profits, Len),
    N is M + K,
    (  N < Len
    -> random_quickselect(Keyed, N, Pivot),
       partition3(Keyed, Pivot, Greater, Equal, _),
       length(Greater, GL),
       NeedEq is N - GL,
       take(NeedEq, Equal, EqualSel),
       append(Greater, EqualSel, TopN),
       sort_diff_desc(TopN, Values)
    ;  sort_diff_desc(Keyed, Values)
    ),
    % sum A for first M and B for next K
    take(M, Values, First),
    sum_list_a(First, SumA),
    skip_take(M, Values, Rest),
    take(K, Rest, Next),
    sum_list_b(Next, SumB),
    Total is SumA + SumB.

%% diff_val(+Pair, -Diff) is det.
diff_val((A,B), Diff) :- Diff is A - B.

%% take(+N, +List, -Prefix) is det.
take(N, L, P) :- length(P, N), append(P, _, L).

%% skip_take(+N, +List, -Rest) is det.
skip_take(N, L, R) :- length(P, N), append(P, R, L).

%% sum_list_a(+Pairs, -Sum) is det.
sum_list_a(Pairs, Sum) :- findall(A, member((A,_), Pairs), As), sum_list(As, Sum).

%% sum_list_b(+Pairs, -Sum) is det.
sum_list_b(Pairs, Sum) :- findall(B, member((_,B), Pairs), Bs), sum_list(Bs, Sum).
