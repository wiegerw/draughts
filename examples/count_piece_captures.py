#!/usr/bin/env python3

#  (C) Copyright Wieger Wesselink 2023. Distributed under the GPL-3.0
#  Software License, (See accompanying file license.txt or copy at
#  https://www.gnu.org/licenses/gpl-3.0.txt)

# Program to count the number of possible piece captures of a given depth

import itertools
from typing import List, Tuple


class GlobalSettings:
    verbose = False


def rc2f(row, col) -> int:
    return 1 + (9 - row) * 5 + (col // 2)


def print_capture(row: int, col: int, capture: Tuple[Tuple[int, int]]) -> None:
    fields = [rc2f(row, col)]
    for (dr, dc) in capture:
        row += dr
        col += dc
        fields.append(rc2f(row, col))
    print('x'.join(map(str, fields)))


def count_piece_captures(depth: int) -> int:
    def is_valid_square(row, col):
        return (0 <= row < 10) and (0 <= col < 10) and (row + col) % 2 == 0

    def has_duplicate_edges(nodes: List[Tuple[int, int]]) -> bool:
        edges = set([])
        for i in range(len(nodes) - 1):
            edges.add((nodes[i], nodes[i+1]))
        return len(edges) != len(nodes) - 1

    def is_valid_capture(row: int, col: int, capture: Tuple[Tuple[int, int]]) -> bool:
        if not is_valid_square(row, col):
            return False
        nodes = [(row, col)]
        for (dr, dc) in capture:
            row += dr
            col += dc
            nodes.append((row, col))
            if not is_valid_square(row, col):
                return False
        if has_duplicate_edges(nodes):
            return False
        return True

    count = 0

    capture_directions = [(-2, -2), (-2, 2), (2, -2), (2, 2)]

    for row in range(9):
        for col in range(10):
            if not is_valid_square(row, col):
                continue
            for capture in itertools.product(capture_directions, repeat=depth):
                if is_valid_capture(row, col, capture):
                    count += 1
                    if GlobalSettings.verbose:
                        print_capture(row, col, capture)
    return count


if __name__ == '__main__':
    for depth in range(1, 8):
        print(f'depth = {depth}  #captures = {count_piece_captures(depth)}')
