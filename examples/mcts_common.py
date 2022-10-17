#!/usr/bin/env python3

#  (C) Copyright Wieger Wesselink 2022. Distributed under the GPL-3.0
#  Software License, (See accompanying file license.txt or copy at
#  https://www.gnu.org/licenses/gpl-3.0.txt)

# Alternative MCTS implementation that does not use a parent attribute in the nodes.
# Instead, a path from the root to a leaf is explicitly constructed inside the MCTS algorithm.

from enum import Enum
from draughts1 import *

Move = int  # moves are stored as integers


def init_scan(bb_size = 0):
    Scan.set("variant", "normal")
    Scan.set("book", "false")
    Scan.set("book-ply", "4")
    Scan.set("book-margin", "4")
    Scan.set("ponder", "false")
    Scan.set("threads", "1")
    Scan.set("tt-size", "24")
    Scan.set("bb-size", f"{bb_size}")
    Scan.update()
    Scan.init()


class GlobalSettings(object):
    verbose = False
    debug = False


class GameResult(Enum):
    WIN = 1,
    DRAW = 0,
    LOSS = -1,
    UNKNOWN = -2


def find_move(u: Pos, v: Pos) -> Move:
    moves = generate_moves(u)
    for m in moves:
        if u.succ(m) == v:
            return m
    return move_none()


def print_move_between_positions(src: Pos, dest: Pos) -> str:
    m = find_move(src, dest)
    return print_move(m, src) if m else f'jump({print_position(dest, False, True)})'


def normalize(x: float) -> float:
    if x > 0:
        return 1
    elif x < 0:
        return 0
    else:
        return 0.5
