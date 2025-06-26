#!/usr/bin/env python3
from dataclasses import dataclass
from typing import List, Optional, Dict
import sys

Cell = Optional[str]        
Grid = List[List[Cell]]


@dataclass(frozen=True)
class Slot:
    """One horizontal or vertical stretch of empty/letter squares."""
    r: int
    c: int
    length: int
    dir: str            # "H" or "V"


def make_grid(r: int, c: int, blacks: list[tuple[int, int]]) -> Grid:
    g = [[None] * c for _ in range(r)]
    for x, y in blacks:
        g[x][y] = "#"
    return g


def copy_grid(g: Grid) -> Grid:
    return [row[:] for row in g]


def find_slots(g: Grid) -> List[Slot]:
    slots: List[Slot] = []
    R, C = len(g), len(g[0])

    # horizontal
    for r in range(R):
        c = 0
        while c < C:
            if g[r][c] == "#":
                c += 1
                continue
            start = c
            while c < C and g[r][c] != "#":
                c += 1
            if c - start >= 2:
                slots.append(Slot(r, start, c - start, "H"))

    # vertical
    for c in range(C):
        r = 0
        while r < R:
            if g[r][c] == "#":
                r += 1
                continue
            start = r
            while r < R and g[r][c] != "#":
                r += 1
            if r - start >= 2:
                slots.append(Slot(start, c, r - start, "V"))

    return slots


def fits(g: Grid, word: str, s: Slot) -> bool:
    if len(word) != s.length:
        return False
    for k, ch in enumerate(word):
        r = s.r + (k if s.dir == "V" else 0)
        c = s.c + (0 if s.dir == "V" else k)
        cell = g[r][c]
        if cell == "#" or (cell and cell != ch):
            return False
    return True


def place(g: Grid, word: str, s: Slot) -> None:
    for k, ch in enumerate(word):
        r = s.r + (k if s.dir == "V" else 0)
        c = s.c + (0 if s.dir == "V" else k)
        g[r][c] = ch


def solve(g: Grid, candidates: Dict[Slot, List[str]]) -> Optional[Grid]:
    """
    Backtracking with MRV heuristic ordering and forward checking.
    """
    # Base case: no slots left
    if not candidates:
        return g

    # MRV: pick the slot with fewest candidates
    slot = min(candidates, key=lambda s: len(candidates[s]))

    for word in candidates[slot]:
        # Place the word in-place
        place(g, word, slot)

        # Forward-check: build domains for remaining slots
        new_candidates: Dict[Slot, List[str]] = {}
        for s, ws in candidates.items():
            if s is slot:
                continue
            # remove the placed word and any that no longer fit
            filtered = [w for w in ws if w != word and fits(g, w, s)]
            if not filtered:
                break
            new_candidates[s] = filtered
        else:
            # Recurse if all slots still have candidates
            result = solve(g, new_candidates)
            if result:
                return result

        # Undo placement
        for k in range(slot.length):
            r = slot.r + (k if slot.dir == "V" else 0)
            c = slot.c + (0 if slot.dir == "V" else k)
            g[r][c] = None

    return None


def print_rows(g: Grid) -> None:
    for row in g:
        run: List[str] = []
        out: List[str] = []
        for cell in row + ["#"]:  # sentinel makes cleanup simpler
            if cell and cell != "#":
                run.append(cell)
            else:
                if len(run) >= 2:
                    out.append("".join(run))
                run.clear()
        print(" ".join(out))


def read_lines(fh, n: int, err: str) -> List[str]:
    lines = [fh.readline().strip() for _ in range(n)]
    if any(l == "" and i < n - 1 for i, l in enumerate(lines)):
        sys.exit(err)
    return lines


def parse(path: str):
    with open(path, encoding="utf-8") as fh:
        header = fh.readline()
        if not header:
            sys.exit("empty input")
        try:
            R, C, B, W = map(int, header.split())
        except ValueError:
            sys.exit("bad header")
        if R > 21 or C > 21:
            sys.exit("grid too big (21×21 max)")

        black_lines = read_lines(fh, B, "bad black list")
        blacks: list[tuple[int, int]] = []
        for line in black_lines:
            try:
                x, y = map(int, line.split())
                x -= 1; y -= 1
            except ValueError:
                sys.exit("bad black cell")
            if not (0 <= x < R and 0 <= y < C):
                sys.exit("black cell out of bounds")
            blacks.append((x, y))

        words = read_lines(fh, W, "bad word list")
    return R, C, blacks, words


def main() -> None:
    if len(sys.argv) != 2:
        sys.exit(f"usage: {sys.argv[0]} INPUT")

    R, C, blacks, words = parse(sys.argv[1])
    grid = make_grid(R, C, blacks)
    slots = find_slots(grid)
    candidates: Dict[Slot, List[str]] = {
        s: [w for w in words if len(w) == s.length]
        for s in slots
    }
    solution = solve(grid, candidates)

    if not solution:
        print("IMPOSSIBLE")
        sys.exit(0)
    print_rows(solution)


if __name__ == "__main__":
    main()
